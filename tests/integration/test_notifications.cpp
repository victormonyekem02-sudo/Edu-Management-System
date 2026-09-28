// IT-NOT: NotificationService as an Observer of submission events (FR-19).
#include <gtest/gtest.h>

#include "TestSchool.h"

using namespace lms;
using namespace lms::test;

namespace {

class NotificationTest : public ::testing::Test {
protected:
    TestSchool school;
    std::vector<Notification> unread(const User& u) {
        return school.sys->notifications().unreadFor(u);
    }
    bool contains(const std::vector<Notification>& list, const std::string& text) {
        for (const auto& n : list) {
            if (n.message.find(text) != std::string::npos) {
                return true;
            }
        }
        return false;
    }
};

TEST_F(NotificationTest, LecturerIsToldAboutSubmissionsIncludingLateOnes) {
    school.sys->submissions().submit(*school.student, school.assignment, "on time");
    school.setTimeRelativeToDeadline(5);
    school.sys->submissions().submit(*school.student2, school.assignment, "late");
    const auto list = unread(*school.lecturer);
    ASSERT_EQ(list.size(), 2u);
    EXPECT_EQ(list[0].message, "Student One submitted 'Homework 1' (MAT101).");
    EXPECT_EQ(list[1].message, "Student Two submitted 'Homework 1' (MAT101) LATE.");
    EXPECT_TRUE(unread(*school.otherLecturer).empty());
}

TEST_F(NotificationTest, ResubmissionIsReported) {
    school.sys->submissions().submit(*school.student, school.assignment, "one");
    school.sys->submissions().submit(*school.student, school.assignment, "two");
    EXPECT_TRUE(contains(unread(*school.lecturer), "resubmitted"));
}

TEST_F(NotificationTest, HeadOfDepartmentIsToldWhenMarkAwaitsModeration) {
    school.submitAndMark(*school.student, 20);
    EXPECT_TRUE(contains(unread(*school.hod), "awaits moderation"));
    EXPECT_TRUE(unread(*school.otherHod).empty());
}

TEST_F(NotificationTest, StudentLearnsNothingBeforeRelease) {
    const int id = school.submitAndMark(*school.student, 20);
    school.sys->moderation().approve(*school.hod, id);
    EXPECT_TRUE(unread(*school.student).empty());
    school.sys->moderation().releaseAssignment(*school.hod, school.assignment);
    const auto list = unread(*school.student);
    ASSERT_EQ(list.size(), 1u);
    EXPECT_NE(list[0].message.find("released"), std::string::npos);
}

TEST_F(NotificationTest, StudentIsToldAboutExtension) {
    school.sys->assignments().grantExtension(*school.lecturer, school.assignment,
                                             school.student->id(),
                                             school.deadline + kSecondsPerDay, "Sick note");
    EXPECT_TRUE(contains(unread(*school.student), "granted an extension"));
}

TEST_F(NotificationTest, MarkAllReadClearsUnreadButKeepsHistory) {
    school.sys->submissions().submit(*school.student, school.assignment, "x");
    school.sys->notifications().markAllRead(*school.lecturer);
    EXPECT_TRUE(unread(*school.lecturer).empty());
    const auto all = school.sys->notifications().allFor(*school.lecturer);
    ASSERT_EQ(all.size(), 1u);
    EXPECT_TRUE(all[0].read);
}

}  // namespace
