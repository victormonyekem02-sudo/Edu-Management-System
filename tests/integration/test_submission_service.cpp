// IT-SUB: SubmissionService - UC-01 "Submit assignment" main, alternative and exception
// flows (FR-10..FR-13, BR-04, BR-05, BR-07).
#include <gtest/gtest.h>

#include "TestSchool.h"
#include "lms/common/Errors.h"

using namespace lms;
using namespace lms::test;

namespace {

class SubmissionTest : public ::testing::Test {
protected:
    TestSchool school;
    SubmissionService& svc() { return school.sys->submissions(); }
    SubmitResult submit(const User& u, const std::string& text = "My answers") {
        return svc().submit(u, school.assignment, text);
    }
};

TEST_F(SubmissionTest, MainFlowOnTimeSubmission) {
    const SubmitResult r = submit(*school.student);
    EXPECT_EQ(r.status, SubmissionStatus::Submitted);
    EXPECT_EQ(r.version, 1);
    EXPECT_FALSE(r.late);
    EXPECT_FALSE(r.resubmission);
    const Submission s = *svc().findOwn(*school.student, school.assignment);
    EXPECT_EQ(s.content, "My answers");
    EXPECT_EQ(s.submittedAt, kT0);
}

TEST_F(SubmissionTest, SubmissionExactlyAtDeadlineIsOnTime) {
    school.setTimeRelativeToDeadline(0);
    EXPECT_EQ(submit(*school.student).status, SubmissionStatus::Submitted);
}

TEST_F(SubmissionTest, AlternativeFlowResubmitBeforeDeadline) {
    const SubmitResult first = submit(*school.student, "draft");
    school.setTimeRelativeToDeadline(-60);
    const SubmitResult second = submit(*school.student, "final answer");
    EXPECT_TRUE(second.resubmission);
    EXPECT_EQ(second.submissionId, first.submissionId);
    EXPECT_EQ(second.version, 2);
    const Submission s = *svc().findOwn(*school.student, school.assignment);
    EXPECT_EQ(s.content, "final answer");
    EXPECT_EQ(s.submittedAt, school.deadline - 60);
    EXPECT_EQ(school.sys->repositories().submissions.countVersions(s.id), 2);
}

TEST_F(SubmissionTest, AlternativeFlowLateSubmissionIsAcceptedNotRejected) {
    school.setTimeRelativeToDeadline(1);
    const SubmitResult r = submit(*school.student);
    EXPECT_EQ(r.status, SubmissionStatus::Late);
    EXPECT_TRUE(r.late);
}

TEST_F(SubmissionTest, LateWorkIsAcceptedEvenWeeksAfterDeadline) {
    school.setTimeRelativeToDeadline(30 * kSecondsPerDay);
    EXPECT_EQ(submit(*school.student).status, SubmissionStatus::Late);
}

TEST_F(SubmissionTest, ExceptionFlowResubmitAfterDeadlineKeepsOriginal) {
    submit(*school.student, "original");
    school.setTimeRelativeToDeadline(1);
    EXPECT_THROW(submit(*school.student, "changed after deadline"), BusinessRuleError);
    EXPECT_EQ(svc().findOwn(*school.student, school.assignment)->content, "original");
}

TEST_F(SubmissionTest, LateSubmissionCannotBeResubmitted) {
    school.setTimeRelativeToDeadline(10);
    submit(*school.student, "late one");
    EXPECT_THROW(submit(*school.student, "late two"), BusinessRuleError);
}

TEST_F(SubmissionTest, ExtensionAllowsSubmitAndResubmitAfterOriginalDeadline) {
    school.sys->assignments().grantExtension(*school.lecturer, school.assignment,
                                             school.student->id(),
                                             school.deadline + kSecondsPerDay, "Bereavement");
    school.setTimeRelativeToDeadline(kSecondsPerHour);
    EXPECT_EQ(submit(*school.student, "v1").status, SubmissionStatus::Submitted);
    EXPECT_EQ(submit(*school.student, "v2").version, 2);
    // A classmate without the extension is late at the same moment.
    EXPECT_EQ(submit(*school.student2).status, SubmissionStatus::Late);
}

TEST_F(SubmissionTest, ExceptionFlowNotEnrolled) {
    EXPECT_THROW(submit(*school.outsider), AuthorizationError);
}

TEST_F(SubmissionTest, ExceptionFlowUnknownAssignment) {
    EXPECT_THROW(svc().submit(*school.student, 999, "x"), NotFoundError);
}

TEST_F(SubmissionTest, ExceptionFlowEmptyOrOversizedWork) {
    EXPECT_THROW(submit(*school.student, "   \n  "), ValidationError);
    EXPECT_THROW(submit(*school.student, std::string(SubmissionService::kMaxContentLength + 1, 'a')),
                 ValidationError);
    EXPECT_NO_THROW(submit(*school.student, std::string(SubmissionService::kMaxContentLength, 'a')));
}

TEST_F(SubmissionTest, ExceptionFlowAlreadyMarked) {
    school.submitAndMark(*school.student, 18);
    EXPECT_THROW(submit(*school.student, "improved"), BusinessRuleError);
}

TEST_F(SubmissionTest, OnlyStudentsSubmit) {
    EXPECT_THROW(submit(*school.lecturer), AuthorizationError);
    EXPECT_THROW(submit(*school.hod), AuthorizationError);
    EXPECT_THROW(submit(*school.admin), AuthorizationError);
}

TEST_F(SubmissionTest, MarkIsHiddenUntilReleased) {
    const int sid = school.submitAndMark(*school.student, 20);
    auto view = svc().resultsFor(*school.student);
    ASSERT_EQ(view.size(), 1u);
    EXPECT_FALSE(view[0].released);
    EXPECT_FALSE(view[0].finalMark.has_value());
    EXPECT_TRUE(view[0].feedback.empty());

    school.sys->moderation().approve(*school.hod, sid);
    view = svc().resultsFor(*school.student);
    EXPECT_FALSE(view[0].finalMark.has_value());  // moderated but not yet released

    school.sys->moderation().releaseAssignment(*school.hod, school.assignment);
    view = svc().resultsFor(*school.student);
    EXPECT_TRUE(view[0].released);
    EXPECT_DOUBLE_EQ(view[0].finalMark.value_or(-1), 20);
    EXPECT_EQ(view[0].feedback, "Good work");
    EXPECT_EQ(view[0].course.code, "MAT101");
}

TEST_F(SubmissionTest, StudentsNeverSeeEachOthersWork) {
    submit(*school.student, "mine");
    EXPECT_FALSE(svc().findOwn(*school.student2, school.assignment).has_value());
    EXPECT_TRUE(svc().resultsFor(*school.student2).empty());
    EXPECT_THROW(svc().resultsFor(*school.lecturer), AuthorizationError);
}

}  // namespace
