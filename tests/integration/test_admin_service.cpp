// IT-ADM: AdminService - accounts, departments, courses, enrolment (FR-03..FR-06).
#include <gtest/gtest.h>

#include "TestSchool.h"
#include "lms/common/Errors.h"

using namespace lms;
using namespace lms::test;

namespace {

class AdminTest : public ::testing::Test {
protected:
    TestSchool school;
    AdminService& admin() { return school.sys->admin(); }
};

TEST_F(AdminTest, BootstrapOnlyAllowedOnce) {
    EXPECT_FALSE(admin().needsBootstrap());
    EXPECT_THROW(admin().bootstrapAdministrator("root", "Root", "RootPass1"), BusinessRuleError);
}

TEST_F(AdminTest, OnlyAdministratorsManageAccounts) {
    for (const User* u : {school.lecturer.get(), school.hod.get(), school.student.get()}) {
        EXPECT_THROW(admin().createUser(*u, Role::Student, "newkid", "New Kid", kPassword,
                                        std::nullopt),
                     AuthorizationError);
        EXPECT_THROW(admin().listUsers(*u), AuthorizationError);
        EXPECT_THROW(admin().createDepartment(*u, "Arts"), AuthorizationError);
        EXPECT_THROW(admin().createCourse(*u, "ART1", "Art", school.sciences), AuthorizationError);
        EXPECT_THROW(admin().enrolStudent(*u, school.course, school.outsider->id()),
                     AuthorizationError);
    }
}

TEST_F(AdminTest, CreatesUsersOfEveryRole) {
    const int dept = admin().createDepartment(*school.admin, "Arts");
    EXPECT_GT(admin().createUser(*school.admin, Role::Administrator, "admin2", "Second Admin",
                                 kPassword, std::nullopt),
              0);
    EXPECT_GT(admin().createUser(*school.admin, Role::Lecturer, "artlect", "Art Lecturer",
                                 kPassword, dept),
              0);
    EXPECT_GT(admin().createUser(*school.admin, Role::HeadOfDepartment, "arthod", "Art Head",
                                 kPassword, dept),
              0);
    EXPECT_GT(admin().createUser(*school.admin, Role::Student, "artkid", "Art Student", kPassword,
                                 dept),
              0);
    EXPECT_EQ(school.sys->auth().login("arthod", kPassword)->role(), Role::HeadOfDepartment);
}

TEST_F(AdminTest, RejectsInvalidAccounts) {
    auto create = [&](Role r, const std::string& user, std::optional<int> dept,
                      const std::string& pw = kPassword) {
        return admin().createUser(*school.admin, r, user, "Some Name", pw, dept);
    };
    EXPECT_THROW(create(Role::Student, "STUDENT", std::nullopt), ValidationError);  // duplicate
    EXPECT_THROW(create(Role::Student, "ab", std::nullopt), ValidationError);       // too short
    EXPECT_THROW(create(Role::Student, std::string(33, 'a'), std::nullopt), ValidationError);
    EXPECT_THROW(create(Role::Student, "bad name", std::nullopt), ValidationError);
    EXPECT_THROW(create(Role::Student, "o'brien", std::nullopt), ValidationError);
    EXPECT_THROW(create(Role::Student, "weakpw", std::nullopt, "password"), ValidationError);
    EXPECT_THROW(create(Role::Lecturer, "nodept", std::nullopt), ValidationError);
    EXPECT_THROW(create(Role::Lecturer, "baddept", 999), NotFoundError);
    EXPECT_THROW(create(Role::HeadOfDepartment, "secondhod", school.sciences), BusinessRuleError);
    EXPECT_THROW(admin().createUser(*school.admin, Role::Student, "noname", "  ", kPassword,
                                    std::nullopt),
                 ValidationError);
    EXPECT_NO_THROW(create(Role::Student, "abc", std::nullopt));                // 3 chars ok
    EXPECT_NO_THROW(create(Role::Student, std::string(32, 'b'), std::nullopt));  // 32 chars ok
}

TEST_F(AdminTest, CannotDeactivateOwnAccount) {
    EXPECT_THROW(admin().setActive(*school.admin, school.admin->id(), false), BusinessRuleError);
}

TEST_F(AdminTest, ReactivatingHodBlockedWhenDepartmentHasAnother) {
    admin().setActive(*school.admin, school.hod->id(), false);
    admin().createUser(*school.admin, Role::HeadOfDepartment, "newhod", "New Head", kPassword,
                       school.sciences);
    EXPECT_THROW(admin().setActive(*school.admin, school.hod->id(), true), BusinessRuleError);
}

TEST_F(AdminTest, ListUsersHidesPasswordHashes) {
    const auto users = admin().listUsers(*school.admin);
    EXPECT_EQ(users.size(), 8u);
    // UserSummary has no password field at all; this documents the design decision.
    static_assert(sizeof(UserSummary) < sizeof(UserRecord), "summary must not carry the hash");
}

TEST_F(AdminTest, DepartmentAndCourseNamesAreUnique) {
    EXPECT_THROW(admin().createDepartment(*school.admin, "sciences"), ValidationError);
    EXPECT_THROW(admin().createDepartment(*school.admin, " "), ValidationError);
    EXPECT_THROW(admin().createCourse(*school.admin, "mat101", "Again", school.sciences),
                 ValidationError);
    EXPECT_THROW(admin().createCourse(*school.admin, "BIO1", "Biology", 999), NotFoundError);
    EXPECT_EQ(admin().listDepartments(*school.admin).size(), 2u);
    EXPECT_EQ(admin().listCourses(*school.admin).size(), 2u);
}

TEST_F(AdminTest, LecturerOnlyTeachesOwnDepartment) {
    EXPECT_THROW(admin().assignLecturer(*school.admin, school.otherCourse, school.lecturer->id()),
                 BusinessRuleError);
    EXPECT_THROW(admin().assignLecturer(*school.admin, school.course, school.student->id()),
                 BusinessRuleError);
    EXPECT_THROW(admin().assignLecturer(*school.admin, 999, school.lecturer->id()), NotFoundError);
}

TEST_F(AdminTest, EnrolOnlyActiveStudentsAndIsIdempotent) {
    EXPECT_THROW(admin().enrolStudent(*school.admin, school.course, school.lecturer->id()),
                 BusinessRuleError);
    EXPECT_THROW(admin().enrolStudent(*school.admin, school.course, 999), NotFoundError);
    admin().enrolStudent(*school.admin, school.course, school.outsider->id());
    admin().enrolStudent(*school.admin, school.course, school.outsider->id());
    EXPECT_EQ(school.sys->directory().studentsOf(school.course).size(), 3u);
}

TEST_F(AdminTest, DirectoryLooksUpNamesWithoutSecrets) {
    DirectoryService& d = school.sys->directory();
    EXPECT_EQ(d.displayName(school.student->id()), "Student One (student)");
    EXPECT_EQ(d.displayName(4242), "#4242");
    EXPECT_EQ(d.headOfDepartment(school.sciences)->username, "hod");
    EXPECT_EQ(d.lecturersOf(school.course).size(), 1u);
    EXPECT_EQ(d.usersWithRole(Role::Student).size(), 3u);
    EXPECT_TRUE(d.findUserByUsername("admin").has_value());
    EXPECT_EQ(d.findCourse(school.course)->code, "MAT101");
    EXPECT_EQ(d.findDepartment(school.sciences)->name, "Sciences");
    EXPECT_EQ(d.findAssignment(school.assignment)->title, "Homework 1");
}

}  // namespace
