// IT-MOD: ModerationService - UC-03 "Moderate and release marks" (FR-16..FR-18, BR-07, BR-08).
#include <gtest/gtest.h>

#include "TestSchool.h"
#include "lms/common/Errors.h"

using namespace lms;
using namespace lms::test;

namespace {

class ModerationTest : public ::testing::Test {
protected:
    TestSchool school;
    ModerationService& svc() { return school.sys->moderation(); }
    Submission load(int id) { return *school.sys->repositories().submissions.findById(id); }
};

TEST_F(ModerationTest, QueueShowsOnlyMarkedWorkOfOwnDepartment) {
    school.submitAndMark(*school.student, 20);
    school.sys->submissions().submit(*school.student2, school.assignment, "unmarked");
    const auto queue = svc().pendingModeration(*school.hod);
    ASSERT_EQ(queue.size(), 1u);
    EXPECT_EQ(queue[0].studentName, "Student One");
    EXPECT_EQ(queue[0].markerName, "Lecturer Sciences");
    EXPECT_EQ(queue[0].course.code, "MAT101");
    EXPECT_TRUE(svc().pendingModeration(*school.otherHod).empty());
    EXPECT_THROW(svc().pendingModeration(*school.lecturer), AuthorizationError);
}

TEST_F(ModerationTest, ApproveMovesToModeratedAndAudits) {
    const int id = school.submitAndMark(*school.student, 20);
    svc().approve(*school.hod, id, "Consistent with rubric");
    const Submission s = load(id);
    EXPECT_EQ(s.status, SubmissionStatus::Moderated);
    EXPECT_EQ(*s.moderatedBy, school.hod->id());
    const auto audit = svc().auditTrail(*school.hod, id);
    ASSERT_EQ(audit.size(), 2u);
    EXPECT_EQ(audit[1].action, AuditAction::ModerationApprove);
    EXPECT_NE(audit[1].reason.find("Consistent with rubric"), std::string::npos);
    EXPECT_EQ(svc().readyForRelease(*school.hod).size(), 1u);
}

TEST_F(ModerationTest, ApproveRequiresMarkedState) {
    const int id =
        school.sys->submissions().submit(*school.student, school.assignment, "x").submissionId;
    EXPECT_THROW(svc().approve(*school.hod, id), BusinessRuleError);
    school.sys->marking().mark(*school.lecturer, id, 10, "ok");
    svc().approve(*school.hod, id);
    EXPECT_THROW(svc().approve(*school.hod, id), BusinessRuleError);
    EXPECT_THROW(svc().approve(*school.hod, 999), NotFoundError);
}

TEST_F(ModerationTest, AdjustChangesMarkWithMandatoryReason) {
    const int id = school.submitAndMark(*school.student, 20);
    EXPECT_THROW(svc().adjust(*school.hod, id, 18, " "), ValidationError);
    EXPECT_THROW(svc().adjust(*school.hod, id, 26, "too high"), ValidationError);
    EXPECT_THROW(svc().adjust(*school.hod, id, -1, "negative"), ValidationError);
    svc().adjust(*school.hod, id, 18, "Marked too generously against rubric item 3");
    const Submission s = load(id);
    EXPECT_EQ(s.status, SubmissionStatus::Moderated);
    EXPECT_DOUBLE_EQ(*s.finalMark, 18);
    EXPECT_DOUBLE_EQ(*s.rawMark, 20);  // lecturer's original judgement is preserved
    const auto audit = svc().auditTrail(*school.hod, id);
    EXPECT_EQ(audit.back().action, AuditAction::ModerationAdjust);
    EXPECT_DOUBLE_EQ(*audit.back().oldMark, 20);
    EXPECT_DOUBLE_EQ(*audit.back().newMark, 18);
    // A moderated mark may be adjusted again before release.
    svc().adjust(*school.hod, id, 19, "Second opinion");
    EXPECT_DOUBLE_EQ(*load(id).finalMark, 19);
}

TEST_F(ModerationTest, OtherDepartmentAndOtherRolesCannotModerate) {
    const int id = school.submitAndMark(*school.student, 20);
    EXPECT_THROW(svc().approve(*school.otherHod, id), AuthorizationError);
    EXPECT_THROW(svc().adjust(*school.otherHod, id, 10, "x"), AuthorizationError);
    EXPECT_THROW(svc().approve(*school.lecturer, id), AuthorizationError);
    EXPECT_THROW(svc().approve(*school.admin, id), AuthorizationError);
    EXPECT_THROW(svc().releaseAssignment(*school.otherHod, school.assignment), AuthorizationError);
    EXPECT_THROW(svc().releaseAssignment(*school.lecturer, school.assignment), AuthorizationError);
    EXPECT_THROW(svc().releaseAssignment(*school.hod, 999), NotFoundError);
}

TEST_F(ModerationTest, ReleasePublishesOnlyModeratedMarks) {
    const int moderated = school.submitAndMark(*school.student, 20);
    const int marked = school.submitAndMark(*school.student2, 12);
    svc().approve(*school.hod, moderated);

    const ReleaseSummary r = svc().releaseAssignment(*school.hod, school.assignment);
    EXPECT_EQ(r.released, 1);
    EXPECT_EQ(r.awaitingModeration, 1);
    EXPECT_EQ(r.notYetMarked, 0);
    EXPECT_EQ(load(moderated).status, SubmissionStatus::Released);
    EXPECT_EQ(*load(moderated).releasedBy, school.hod->id());
    EXPECT_EQ(load(marked).status, SubmissionStatus::Marked);
    EXPECT_FALSE(school.sys->submissions().resultsFor(*school.student2)[0].finalMark.has_value());

    // Releasing again is harmless.
    EXPECT_EQ(svc().releaseAssignment(*school.hod, school.assignment).released, 0);
}

TEST_F(ModerationTest, ReleaseCountsUnmarkedWork) {
    school.sys->submissions().submit(*school.student, school.assignment, "x");
    const ReleaseSummary r = svc().releaseAssignment(*school.hod, school.assignment);
    EXPECT_EQ(r.released, 0);
    EXPECT_EQ(r.notYetMarked, 1);
}

TEST_F(ModerationTest, ReleasedMarksAreFinal) {
    const int id = school.submitAndMark(*school.student, 20);
    svc().approve(*school.hod, id);
    svc().releaseAssignment(*school.hod, school.assignment);
    EXPECT_THROW(svc().adjust(*school.hod, id, 10, "after release"), BusinessRuleError);
    EXPECT_THROW(school.sys->marking().mark(*school.lecturer, id, 10, "x", "y"), BusinessRuleError);
}

TEST_F(ModerationTest, AuditTrailRecordsWhoWhenOldNewWhy) {
    const int id = school.sys->submissions().submit(*school.student, school.assignment, "w")
                       .submissionId;
    school.clock->advance(kSecondsPerHour);
    school.sys->marking().mark(*school.lecturer, id, 15, "ok");
    school.clock->advance(kSecondsPerHour);
    school.sys->marking().mark(*school.lecturer, id, 17, "ok", "Rechecked Q2");
    school.clock->advance(kSecondsPerHour);
    svc().adjust(*school.hod, id, 16, "Moderation sample");
    school.clock->advance(kSecondsPerHour);
    svc().releaseAssignment(*school.hod, school.assignment);

    const auto trail = svc().auditTrail(*school.hod, id);
    ASSERT_EQ(trail.size(), 4u);
    const AuditAction expected[] = {AuditAction::Mark, AuditAction::Remark,
                                    AuditAction::ModerationAdjust, AuditAction::Release};
    const double newMarks[] = {15, 17, 16, 16};
    for (std::size_t i = 0; i < trail.size(); ++i) {
        EXPECT_EQ(trail[i].action, expected[i]);
        EXPECT_DOUBLE_EQ(*trail[i].newMark, newMarks[i]);
        EXPECT_EQ(trail[i].changedAt, kT0 + static_cast<Timestamp>(i + 1) * kSecondsPerHour);
        EXPECT_FALSE(trail[i].reason.empty());
    }
    EXPECT_EQ(trail[0].changedBy, school.lecturer->id());
    EXPECT_EQ(trail[2].changedBy, school.hod->id());
    EXPECT_DOUBLE_EQ(*trail[2].oldMark, 17);
}

TEST_F(ModerationTest, AuditLogScope) {
    school.submitAndMark(*school.student, 20);
    EXPECT_EQ(svc().auditLog(*school.hod).size(), 1u);
    EXPECT_TRUE(svc().auditLog(*school.otherHod).empty());
    EXPECT_EQ(svc().auditLog(*school.admin).size(), 1u);
    EXPECT_THROW(svc().auditLog(*school.lecturer), AuthorizationError);
    EXPECT_THROW(svc().auditLog(*school.student), AuthorizationError);
}

TEST_F(ModerationTest, AdministratorCanInspectAnySubmissionTrail) {
    const int id = school.submitAndMark(*school.student, 20);
    EXPECT_EQ(svc().auditTrail(*school.admin, id).size(), 1u);
    EXPECT_THROW(svc().auditTrail(*school.otherHod, id), AuthorizationError);
}

}  // namespace
