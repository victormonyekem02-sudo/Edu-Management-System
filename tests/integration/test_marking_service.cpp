// IT-MRK: MarkingService - UC-02 "Mark submission", penalties and audit (FR-14, FR-15).
#include <gtest/gtest.h>

#include <limits>

#include "TestSchool.h"
#include "lms/common/Errors.h"

using namespace lms;
using namespace lms::test;

namespace {

class MarkingTest : public ::testing::Test {
protected:
    TestSchool school;
    MarkingService& svc() { return school.sys->marking(); }
    int submitAt(std::int64_t offsetFromDeadline, const User* who = nullptr) {
        school.setTimeRelativeToDeadline(offsetFromDeadline);
        return school.sys->submissions()
            .submit(who ? *who : *school.student, school.assignment, "work")
            .submissionId;
    }
    Submission load(int id) { return *school.sys->repositories().submissions.findById(id); }
};

TEST_F(MarkingTest, OnTimeWorkKeepsRawMarkAndIsAudited) {
    const int id = submitAt(-kSecondsPerDay);
    const PenaltyResult r = svc().mark(*school.lecturer, id, 20, "Well argued");
    EXPECT_DOUBLE_EQ(r.finalMark, 20);
    const Submission s = load(id);
    EXPECT_EQ(s.status, SubmissionStatus::Marked);
    EXPECT_DOUBLE_EQ(*s.rawMark, 20);
    EXPECT_DOUBLE_EQ(*s.finalMark, 20);
    EXPECT_EQ(s.feedback, "Well argued");
    EXPECT_EQ(*s.markedBy, school.lecturer->id());

    const auto audit = school.sys->repositories().audit.listForSubmission(id);
    ASSERT_EQ(audit.size(), 1u);
    EXPECT_EQ(audit[0].action, AuditAction::Mark);
    EXPECT_EQ(audit[0].changedBy, school.lecturer->id());
    EXPECT_FALSE(audit[0].oldMark.has_value());
    EXPECT_DOUBLE_EQ(*audit[0].newMark, 20);
    EXPECT_NE(audit[0].reason.find("Initial marking"), std::string::npos);
}

struct LateCase {
    std::int64_t offset;
    double expectedFinal;
    int expectedDays;
};

class LateMarking : public MarkingTest, public ::testing::WithParamInterface<LateCase> {};

TEST_P(LateMarking, PenaltyAppliedFromSubmissionTime) {
    const LateCase& c = GetParam();
    const int id = submitAt(c.offset);
    school.clock->advance(10 * kSecondsPerDay);  // marking later must not add lateness
    const PenaltyResult r = svc().mark(*school.lecturer, id, 20, "Feedback");
    EXPECT_DOUBLE_EQ(r.finalMark, c.expectedFinal);
    EXPECT_EQ(load(id).daysLate, c.expectedDays);
}

INSTANTIATE_TEST_SUITE_P(IT_MRK, LateMarking,
                         ::testing::Values(LateCase{0, 20, 0},
                                           LateCase{1, 17.5, 1},
                                           LateCase{kSecondsPerDay, 17.5, 1},
                                           LateCase{kSecondsPerDay + 1, 15, 2},
                                           LateCase{3 * kSecondsPerDay, 12.5, 3},
                                           LateCase{3 * kSecondsPerDay + 1, 0, 4}));

TEST_F(MarkingTest, VeryLateWorkScoresZeroButKeepsFeedback) {
    const int id = submitAt(5 * kSecondsPerDay);
    svc().mark(*school.lecturer, id, 25, "Excellent, but far too late");
    const Submission s = load(id);
    EXPECT_DOUBLE_EQ(*s.finalMark, 0);
    EXPECT_DOUBLE_EQ(*s.rawMark, 25);
    EXPECT_EQ(s.feedback, "Excellent, but far too late");
}

TEST_F(MarkingTest, ExtensionGrantedAfterLateSubmissionRemovesPenalty) {
    const int id = submitAt(2 * kSecondsPerDay);
    school.sys->assignments().grantExtension(*school.lecturer, school.assignment,
                                             school.student->id(),
                                             school.deadline + 3 * kSecondsPerDay, "Hospital");
    EXPECT_DOUBLE_EQ(svc().mark(*school.lecturer, id, 20, "ok").finalMark, 20);
}

TEST_F(MarkingTest, ValidatesMarkRangeAndFeedback) {
    const int id = submitAt(-10);
    EXPECT_THROW(svc().mark(*school.lecturer, id, -0.01, "x"), ValidationError);
    EXPECT_THROW(svc().mark(*school.lecturer, id, 25.01, "x"), ValidationError);
    EXPECT_THROW(svc().mark(*school.lecturer, id, std::numeric_limits<double>::quiet_NaN(), "x"),
                 ValidationError);
    EXPECT_THROW(svc().mark(*school.lecturer, id, 10, "   "), ValidationError);
    EXPECT_THROW(svc().mark(*school.lecturer, id, 10, std::string(5001, 'f')), ValidationError);
    EXPECT_EQ(load(id).status, SubmissionStatus::Submitted);  // nothing changed
    EXPECT_NO_THROW(svc().mark(*school.lecturer, id, 0, "Nothing correct"));
    EXPECT_NO_THROW(svc().mark(*school.lecturer, id, 25, "All correct", "Re-checked answers"));
}

TEST_F(MarkingTest, RemarkRequiresReasonAndIsAudited) {
    const int id = submitAt(-10);
    svc().mark(*school.lecturer, id, 15, "First look");
    EXPECT_THROW(svc().mark(*school.lecturer, id, 18, "Second look"), ValidationError);
    svc().mark(*school.lecturer, id, 18, "Second look", "Missed question 4 on first pass");
    const auto audit = school.sys->repositories().audit.listForSubmission(id);
    ASSERT_EQ(audit.size(), 2u);
    EXPECT_EQ(audit[1].action, AuditAction::Remark);
    EXPECT_DOUBLE_EQ(*audit[1].oldMark, 15);
    EXPECT_DOUBLE_EQ(*audit[1].newMark, 18);
    EXPECT_NE(audit[1].reason.find("Missed question 4"), std::string::npos);
}

TEST_F(MarkingTest, OnlyTheCoursesLecturerMarks) {
    const int id = submitAt(-10);
    EXPECT_THROW(svc().mark(*school.otherLecturer, id, 10, "x"), AuthorizationError);
    EXPECT_THROW(svc().mark(*school.hod, id, 10, "x"), AuthorizationError);
    EXPECT_THROW(svc().mark(*school.student, id, 25, "x"), AuthorizationError);
    EXPECT_THROW(svc().mark(*school.lecturer, 999, 10, "x"), NotFoundError);
}

TEST_F(MarkingTest, LecturerCannotChangeModeratedMark) {
    const int id = submitAt(-10);
    svc().mark(*school.lecturer, id, 15, "ok");
    school.sys->moderation().approve(*school.hod, id);
    EXPECT_THROW(svc().mark(*school.lecturer, id, 25, "better", "changed my mind"),
                 BusinessRuleError);
}

TEST_F(MarkingTest, ListsSubmissionsWithStudentNames) {
    submitAt(-10);
    submitAt(5, school.student2.get());
    const auto items = svc().listForAssignment(*school.lecturer, school.assignment);
    ASSERT_EQ(items.size(), 2u);
    EXPECT_EQ(items[0].studentName, "Student One");
    EXPECT_EQ(items[1].submission.status, SubmissionStatus::Late);
    EXPECT_THROW(svc().listForAssignment(*school.otherLecturer, school.assignment),
                 AuthorizationError);
}

TEST(IT_MRK, PenaltyStrategyIsInjectable) {
    TestSchool lenient(std::make_unique<NoLatePenaltyPolicy>());
    lenient.setTimeRelativeToDeadline(10 * kSecondsPerDay);
    const int id =
        lenient.sys->submissions().submit(*lenient.student, lenient.assignment, "w").submissionId;
    EXPECT_DOUBLE_EQ(lenient.sys->marking().mark(*lenient.lecturer, id, 20, "ok").finalMark, 20);
    EXPECT_EQ(lenient.sys->marking().policy().description(), "No late penalty");
}

}  // namespace
