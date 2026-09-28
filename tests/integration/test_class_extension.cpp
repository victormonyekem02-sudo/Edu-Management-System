// IT-CLX: class-wide extensions - change request CR1, release v1.1 (FR-22, BR-03).
#include <gtest/gtest.h>

#include "TestSchool.h"
#include "lms/common/Errors.h"

using namespace lms;
using namespace lms::test;

namespace {

class ClassExtensionTest : public ::testing::Test {
protected:
    TestSchool school;
    AssignmentService& svc() { return school.sys->assignments(); }
    Assignment assignment() { return *school.sys->directory().findAssignment(school.assignment); }
    void grantClass(std::int64_t offset, const std::string& reason = "Exam week clash") {
        svc().grantClassExtension(*school.lecturer, school.assignment, school.deadline + offset,
                                  reason);
    }
    void grantIndividual(const User& student, std::int64_t offset) {
        svc().grantExtension(*school.lecturer, school.assignment, student.id(),
                             school.deadline + offset, "Medical certificate");
    }
};

TEST_F(ClassExtensionTest, AppliesToEveryEnrolledStudent) {
    grantClass(2 * kSecondsPerDay);
    for (const User* s : {school.student.get(), school.student2.get()}) {
        const DeadlineInfo info = svc().deadlineFor(assignment(), s->id());
        EXPECT_EQ(info.deadline, school.deadline + 2 * kSecondsPerDay);
        EXPECT_EQ(info.kind, ExtensionKind::Class);
        const auto views = svc().listForStudent(*s);
        ASSERT_EQ(views.size(), 1u);
        EXPECT_EQ(views[0].extensionKind, ExtensionKind::Class);
        EXPECT_TRUE(views[0].extended);
        EXPECT_EQ(views[0].assignment.deadline, school.deadline);  // original still available
    }
}

TEST_F(ClassExtensionTest, LaterClassExtensionOverridesEarlierIndividualOne) {
    grantIndividual(*school.student, kSecondsPerDay);
    grantClass(2 * kSecondsPerDay);
    const DeadlineInfo info = svc().deadlineFor(assignment(), school.student->id());
    EXPECT_EQ(info.deadline, school.deadline + 2 * kSecondsPerDay);
    EXPECT_EQ(info.kind, ExtensionKind::Class);
}

TEST_F(ClassExtensionTest, LaterIndividualExtensionOverridesClassOne) {
    grantClass(2 * kSecondsPerDay);
    grantIndividual(*school.student, 3 * kSecondsPerDay);
    EXPECT_EQ(svc().deadlineFor(assignment(), school.student->id()).kind,
              ExtensionKind::Individual);
    EXPECT_EQ(svc().effectiveDeadline(assignment(), school.student->id()),
              school.deadline + 3 * kSecondsPerDay);
    EXPECT_EQ(svc().deadlineFor(assignment(), school.student2->id()).kind, ExtensionKind::Class);
}

TEST_F(ClassExtensionTest, WithoutExtensionsKindIsNone) {
    EXPECT_EQ(svc().deadlineFor(assignment(), school.student->id()).kind, ExtensionKind::None);
    EXPECT_EQ(extensionKindName(ExtensionKind::None), "-");
    EXPECT_EQ(extensionKindName(ExtensionKind::Class), "class");
    EXPECT_EQ(extensionKindName(ExtensionKind::Individual), "individual");
}

TEST_F(ClassExtensionTest, Validation) {
    EXPECT_THROW(grantClass(0), ValidationError);  // not later than the deadline (boundary)
    EXPECT_NO_THROW(grantClass(60));               // one minute later (boundary)
    EXPECT_THROW(grantClass(kSecondsPerDay, "   "), ValidationError);
    EXPECT_THROW(svc().grantClassExtension(*school.otherLecturer, school.assignment,
                                           school.deadline + kSecondsPerDay, "x"),
                 AuthorizationError);
    EXPECT_THROW(svc().grantClassExtension(*school.student, school.assignment,
                                           school.deadline + kSecondsPerDay, "x"),
                 AuthorizationError);
    EXPECT_THROW(svc().grantClassExtension(*school.hod, school.assignment,
                                           school.deadline + kSecondsPerDay, "x"),
                 AuthorizationError);
    EXPECT_THROW(svc().grantClassExtension(*school.lecturer, 999, school.deadline + 60, "x"),
                 NotFoundError);
}

TEST_F(ClassExtensionTest, RegrantingReplacesTheClassExtension) {
    grantClass(2 * kSecondsPerDay, "first");
    grantClass(kSecondsPerDay, "revised");
    const auto c = svc().classExtension(*school.lecturer, school.assignment);
    ASSERT_TRUE(c.has_value());
    EXPECT_EQ(c->reason, "revised");
    EXPECT_EQ(c->newDeadline, school.deadline + kSecondsPerDay);
    EXPECT_EQ(c->grantedBy, school.lecturer->id());
    EXPECT_THROW(svc().classExtension(*school.otherLecturer, school.assignment),
                 AuthorizationError);
}

TEST_F(ClassExtensionTest, ReclassifiesCoveredLateSubmissions) {
    school.setTimeRelativeToDeadline(kSecondsPerHour);
    const int covered =
        school.sys->submissions().submit(*school.student, school.assignment, "a").submissionId;
    school.setTimeRelativeToDeadline(3 * kSecondsPerDay);
    const int notCovered =
        school.sys->submissions().submit(*school.student2, school.assignment, "b").submissionId;
    grantClass(kSecondsPerDay);
    auto& repo = school.sys->repositories().submissions;
    EXPECT_EQ(repo.findById(covered)->status, SubmissionStatus::Submitted);
    EXPECT_EQ(repo.findById(notCovered)->status, SubmissionStatus::Late);
}

TEST_F(ClassExtensionTest, RefusedOnceMarksAreModerated) {
    const int id = school.submitAndMark(*school.student, 20);
    EXPECT_NO_THROW(grantClass(kSecondsPerDay));  // marked but not moderated: allowed
    school.sys->moderation().approve(*school.hod, id);
    EXPECT_THROW(grantClass(2 * kSecondsPerDay), BusinessRuleError);
}

TEST_F(ClassExtensionTest, LatePenaltyMeasuredFromClassDeadline) {
    school.setTimeRelativeToDeadline(kSecondsPerHour);
    const int id =
        school.sys->submissions().submit(*school.student, school.assignment, "a").submissionId;
    grantClass(kSecondsPerDay);
    EXPECT_DOUBLE_EQ(school.sys->marking().mark(*school.lecturer, id, 20, "ok").finalMark, 20);
}

TEST_F(ClassExtensionTest, StudentsMayResubmitUntilTheClassDeadline) {
    grantClass(kSecondsPerDay);
    school.setTimeRelativeToDeadline(kSecondsPerHour);
    EXPECT_EQ(school.sys->submissions().submit(*school.student, school.assignment, "v1").status,
              SubmissionStatus::Submitted);
    EXPECT_EQ(school.sys->submissions().submit(*school.student, school.assignment, "v2").version, 2);
    school.setTimeRelativeToDeadline(kSecondsPerDay + 1);
    EXPECT_THROW(school.sys->submissions().submit(*school.student, school.assignment, "v3"),
                 BusinessRuleError);
}

TEST_F(ClassExtensionTest, EveryEnrolledStudentIsNotified) {
    grantClass(kSecondsPerDay);
    for (const User* s : {school.student.get(), school.student2.get()}) {
        const auto list = school.sys->notifications().unreadFor(*s);
        ASSERT_EQ(list.size(), 1u);
        EXPECT_NE(list[0].message.find("extended for the whole class"), std::string::npos);
    }
    EXPECT_TRUE(school.sys->notifications().unreadFor(*school.outsider).empty());
}

}  // namespace
