// IT-ASG: AssignmentService - creation and individual extensions (FR-07, FR-08, FR-09).
#include <gtest/gtest.h>

#include "TestSchool.h"
#include "lms/common/Errors.h"

using namespace lms;
using namespace lms::test;

namespace {

class AssignmentTest : public ::testing::Test {
protected:
    TestSchool school;
    AssignmentService& svc() { return school.sys->assignments(); }
};

TEST_F(AssignmentTest, LecturerCreatesAssignmentForOwnCourse) {
    const int id = svc().createAssignment(*school.lecturer, school.course, "  Quiz 2 ", "",
                                          kT0 + kSecondsPerDay, 10);
    const Assignment a = *school.sys->directory().findAssignment(id);
    EXPECT_EQ(a.title, "Quiz 2");
    EXPECT_EQ(a.maxMark, 10);
    EXPECT_EQ(a.createdBy, school.lecturer->id());
    EXPECT_EQ(svc().listForCourse(*school.lecturer, school.course).size(), 2u);
}

TEST_F(AssignmentTest, OnlyTheCoursesLecturerMayCreate) {
    EXPECT_THROW(svc().createAssignment(*school.otherLecturer, school.course, "X", "",
                                        kT0 + kSecondsPerDay, 10),
                 AuthorizationError);
    EXPECT_THROW(svc().createAssignment(*school.student, school.course, "X", "",
                                        kT0 + kSecondsPerDay, 10),
                 AuthorizationError);
    EXPECT_THROW(svc().createAssignment(*school.hod, school.course, "X", "", kT0 + kSecondsPerDay,
                                        10),
                 AuthorizationError);
    EXPECT_THROW(svc().createAssignment(*school.lecturer, 999, "X", "", kT0 + kSecondsPerDay, 10),
                 NotFoundError);
}

TEST_F(AssignmentTest, ValidatesDeadlineMaxMarkAndTitle) {
    auto create = [&](Timestamp deadline, int max, const std::string& title = "T") {
        return svc().createAssignment(*school.lecturer, school.course, title, "", deadline, max);
    };
    EXPECT_THROW(create(kT0, 10), ValidationError);       // deadline = now (boundary)
    EXPECT_THROW(create(kT0 - 1, 10), ValidationError);   // past
    EXPECT_NO_THROW(create(kT0 + 1, 10));                 // one second in the future
    EXPECT_THROW(create(kT0 + 60, 0), ValidationError);   // max mark below range
    EXPECT_NO_THROW(create(kT0 + 60, 1));                 // lower boundary
    EXPECT_NO_THROW(create(kT0 + 60, 1000));              // upper boundary
    EXPECT_THROW(create(kT0 + 60, 1001), ValidationError);
    EXPECT_THROW(create(kT0 + 60, 10, "   "), ValidationError);
    EXPECT_THROW(create(kT0 + 60, 10, std::string(201, 'x')), ValidationError);
    EXPECT_THROW(svc().createAssignment(*school.lecturer, school.course, "T",
                                        std::string(4001, 'd'), kT0 + 60, 10),
                 ValidationError);
}

TEST_F(AssignmentTest, ExtensionMovesThatStudentsDeadlineOnly) {
    const Timestamp extended = school.deadline + 2 * kSecondsPerDay;
    svc().grantExtension(*school.lecturer, school.assignment, school.student->id(), extended,
                         "Illness (medical note)");
    const Assignment a = *school.sys->directory().findAssignment(school.assignment);
    EXPECT_EQ(svc().effectiveDeadline(a, school.student->id()), extended);
    EXPECT_EQ(svc().effectiveDeadline(a, school.student2->id()), school.deadline);

    const auto views = svc().listForStudent(*school.student);
    ASSERT_EQ(views.size(), 1u);
    EXPECT_TRUE(views[0].extended);
    EXPECT_EQ(views[0].effectiveDeadline, extended);
    EXPECT_FALSE(svc().listForStudent(*school.student2)[0].extended);
}

TEST_F(AssignmentTest, RegrantingReplacesTheExtension) {
    svc().grantExtension(*school.lecturer, school.assignment, school.student->id(),
                         school.deadline + kSecondsPerDay, "Reason one");
    svc().grantExtension(*school.lecturer, school.assignment, school.student->id(),
                         school.deadline + 3 * kSecondsPerDay, "Reason two");
    const auto list = svc().listExtensions(*school.lecturer, school.assignment);
    ASSERT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0].reason, "Reason two");
    EXPECT_EQ(list[0].newDeadline, school.deadline + 3 * kSecondsPerDay);
}

TEST_F(AssignmentTest, ExtensionRules) {
    const int sid = school.student->id();
    EXPECT_THROW(svc().grantExtension(*school.lecturer, school.assignment, sid, school.deadline,
                                      "Same time"),
                 ValidationError);  // must be later than the deadline (boundary)
    EXPECT_THROW(svc().grantExtension(*school.lecturer, school.assignment, sid,
                                      school.deadline + 60, "  "),
                 ValidationError);
    EXPECT_THROW(svc().grantExtension(*school.lecturer, school.assignment,
                                      school.outsider->id(), school.deadline + 60, "x"),
                 BusinessRuleError);
    EXPECT_THROW(svc().grantExtension(*school.otherLecturer, school.assignment, sid,
                                      school.deadline + 60, "x"),
                 AuthorizationError);
    EXPECT_THROW(svc().grantExtension(*school.student, school.assignment, sid,
                                      school.deadline + 60, "x"),
                 AuthorizationError);
    EXPECT_THROW(svc().grantExtension(*school.lecturer, 999, sid, school.deadline + 60, "x"),
                 NotFoundError);
}

TEST_F(AssignmentTest, ExtensionReclassifiesUnmarkedLateSubmission) {
    school.setTimeRelativeToDeadline(kSecondsPerHour);
    const SubmitResult r =
        school.sys->submissions().submit(*school.student, school.assignment, "late work");
    ASSERT_EQ(r.status, SubmissionStatus::Late);
    svc().grantExtension(*school.lecturer, school.assignment, school.student->id(),
                         school.deadline + kSecondsPerDay, "Power cut at home");
    EXPECT_EQ(school.sys->repositories().submissions.findById(r.submissionId)->status,
              SubmissionStatus::Submitted);
}

TEST_F(AssignmentTest, ExtensionShorterThanLatenessKeepsLateStatus) {
    school.setTimeRelativeToDeadline(2 * kSecondsPerDay);
    const SubmitResult r =
        school.sys->submissions().submit(*school.student, school.assignment, "late work");
    svc().grantExtension(*school.lecturer, school.assignment, school.student->id(),
                         school.deadline + kSecondsPerDay, "Partial");
    EXPECT_EQ(school.sys->repositories().submissions.findById(r.submissionId)->status,
              SubmissionStatus::Late);
}

TEST_F(AssignmentTest, NoExtensionAfterModeration) {
    const int sid = school.submitAndMark(*school.student, 20);
    school.sys->moderation().approve(*school.hod, sid);
    EXPECT_THROW(svc().grantExtension(*school.lecturer, school.assignment, school.student->id(),
                                      school.deadline + kSecondsPerDay, "Too late"),
                 BusinessRuleError);
}

TEST_F(AssignmentTest, StudentsSeeOnlyEnrolledCourses) {
    EXPECT_EQ(svc().listForStudent(*school.student).size(), 1u);
    EXPECT_TRUE(svc().listForStudent(*school.outsider).empty());
    EXPECT_THROW(svc().listForStudent(*school.lecturer), AuthorizationError);
    EXPECT_THROW(svc().listForCourse(*school.outsider, school.course), AuthorizationError);
    EXPECT_EQ(svc().listForCourse(*school.student, school.course).size(), 1u);
    EXPECT_EQ(svc().listForCourse(*school.hod, school.course).size(), 1u);
    EXPECT_THROW(svc().listForCourse(*school.otherHod, school.course), AuthorizationError);
    EXPECT_EQ(svc().coursesOf(*school.student).size(), 1u);
    EXPECT_EQ(svc().coursesTaughtBy(*school.lecturer).size(), 1u);
}

}  // namespace
