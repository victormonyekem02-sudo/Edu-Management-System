// UT-USR: User hierarchy, Factory and role-based permissions (FR-02).
#include <gtest/gtest.h>

#include "lms/common/Errors.h"
#include "lms/domain/User.h"
#include "lms/domain/UserFactory.h"

using namespace lms;

namespace {

UserRecord record(Role role, std::optional<int> dept = 1) {
    UserRecord r;
    r.id = 7;
    r.username = "user";
    r.fullName = "Some User";
    r.role = role;
    r.departmentId = dept;
    return r;
}

TEST(UT_USR, FactoryCreatesConcreteClassForEachRole) {
    EXPECT_NE(dynamic_cast<Administrator*>(UserFactory::create(record(Role::Administrator)).get()),
              nullptr);
    EXPECT_NE(dynamic_cast<Lecturer*>(UserFactory::create(record(Role::Lecturer)).get()), nullptr);
    EXPECT_NE(dynamic_cast<HeadOfDepartment*>(
                  UserFactory::create(record(Role::HeadOfDepartment)).get()),
              nullptr);
    EXPECT_NE(dynamic_cast<Student*>(UserFactory::create(record(Role::Student)).get()), nullptr);
}

TEST(UT_USR, FactoryCopiesIdentity) {
    auto u = UserFactory::create(record(Role::Student, std::nullopt));
    EXPECT_EQ(u->id(), 7);
    EXPECT_EQ(u->username(), "user");
    EXPECT_EQ(u->fullName(), "Some User");
    EXPECT_FALSE(u->departmentId().has_value());
    EXPECT_EQ(u->role(), Role::Student);
    EXPECT_EQ(u->roleTitle(), "Student");
}

TEST(UT_USR, FactoryRejectsStaffWithoutDepartment) {
    EXPECT_THROW(UserFactory::create(record(Role::Lecturer, std::nullopt)), ValidationError);
    EXPECT_THROW(UserFactory::create(record(Role::HeadOfDepartment, std::nullopt)),
                 ValidationError);
}

// Full role x permission matrix (the access-control table in the SRS, section 3.2).
struct PermissionCase {
    Role role;
    Permission permission;
    bool allowed;
};

class PermissionMatrix : public ::testing::TestWithParam<PermissionCase> {};

TEST_P(PermissionMatrix, RoleGrantsExactlyTheSpecifiedPermissions) {
    const PermissionCase& c = GetParam();
    auto user = UserFactory::create(record(c.role));
    EXPECT_EQ(user->can(c.permission), c.allowed)
        << roleToCode(c.role) << " / " << permissionName(c.permission);
    if (c.allowed) {
        EXPECT_NO_THROW(user->require(c.permission));
    } else {
        EXPECT_THROW(user->require(c.permission), AuthorizationError);
    }
}

std::vector<PermissionCase> allPermissionCases() {
    const Permission all[] = {Permission::ManageUsers,     Permission::ManageDepartments,
                              Permission::ManageCourses,   Permission::AssignLecturers,
                              Permission::EnrolStudents,   Permission::CreateAssignment,
                              Permission::GrantExtension,  Permission::MarkSubmission,
                              Permission::SubmitWork,      Permission::ViewReleasedResults,
                              Permission::ModerateMarks,   Permission::ReleaseMarks,
                              Permission::ViewAuditLog};
    const auto allowed = [](Role r, Permission p) {
        switch (r) {
            case Role::Administrator:
                return p == Permission::ManageUsers || p == Permission::ManageDepartments ||
                       p == Permission::ManageCourses || p == Permission::AssignLecturers ||
                       p == Permission::EnrolStudents || p == Permission::ViewAuditLog;
            case Role::Lecturer:
                return p == Permission::CreateAssignment || p == Permission::GrantExtension ||
                       p == Permission::MarkSubmission;
            case Role::HeadOfDepartment:
                return p == Permission::ModerateMarks || p == Permission::ReleaseMarks ||
                       p == Permission::ViewAuditLog;
            case Role::Student:
                return p == Permission::SubmitWork || p == Permission::ViewReleasedResults;
        }
        return false;
    };
    std::vector<PermissionCase> cases;
    for (Role r : {Role::Administrator, Role::Lecturer, Role::HeadOfDepartment, Role::Student}) {
        for (Permission p : all) {
            cases.push_back({r, p, allowed(r, p)});
        }
    }
    return cases;
}

INSTANTIATE_TEST_SUITE_P(UT_USR, PermissionMatrix, ::testing::ValuesIn(allPermissionCases()));

TEST(UT_USR, AuthorizationMessageNamesRoleAndAction) {
    auto student = UserFactory::create(record(Role::Student));
    try {
        student->require(Permission::ModerateMarks);
        FAIL() << "expected AuthorizationError";
    } catch (const AuthorizationError& e) {
        EXPECT_STREQ(e.what(), "Access denied: a Student may not moderate marks.");
    }
}

TEST(UT_USR, DepartmentScopeIsPolymorphic) {
    auto admin = UserFactory::create(record(Role::Administrator));
    auto hod = UserFactory::create(record(Role::HeadOfDepartment, 3));
    auto lecturer = UserFactory::create(record(Role::Lecturer, 3));
    auto student = UserFactory::create(record(Role::Student, 3));
    EXPECT_TRUE(admin->hasDepartmentScope(3));
    EXPECT_TRUE(admin->hasDepartmentScope(99));
    EXPECT_TRUE(hod->hasDepartmentScope(3));
    EXPECT_FALSE(hod->hasDepartmentScope(4));
    EXPECT_FALSE(lecturer->hasDepartmentScope(3));
    EXPECT_FALSE(student->hasDepartmentScope(3));
}

TEST(UT_USR, AdministratorHasNoDepartment) {
    auto admin = UserFactory::create(record(Role::Administrator, 5));
    EXPECT_FALSE(admin->departmentId().has_value());
}

TEST(UT_USR, RoleCodesRoundTrip) {
    for (Role r : {Role::Administrator, Role::Lecturer, Role::HeadOfDepartment, Role::Student}) {
        EXPECT_EQ(roleFromCode(roleToCode(r)), r);
        EXPECT_FALSE(roleDisplayName(r).empty());
    }
    EXPECT_THROW(roleFromCode("SUPERUSER"), ValidationError);
    EXPECT_THROW(roleFromCode("admin"), ValidationError);  // codes are case-sensitive
}

}  // namespace
