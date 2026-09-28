#include "lms/domain/User.h"

#include <utility>

#include "lms/common/Errors.h"
#include "lms/domain/UserFactory.h"

namespace lms {

User::User(int id, std::string username, std::string fullName, std::optional<int> departmentId)
    : id_(id),
      username_(std::move(username)),
      fullName_(std::move(fullName)),
      departmentId_(departmentId) {}

void User::require(Permission p) const {
    if (!can(p)) {
        throw AuthorizationError("Access denied: a " + roleTitle() + " may not " +
                                 permissionName(p) + ".");
    }
}

Administrator::Administrator(int id, std::string username, std::string fullName)
    : User(id, std::move(username), std::move(fullName), std::nullopt) {}

const std::set<Permission>& Administrator::permissions() const noexcept {
    static const std::set<Permission> kPermissions{
        Permission::ManageUsers,     Permission::ManageDepartments, Permission::ManageCourses,
        Permission::AssignLecturers, Permission::EnrolStudents,     Permission::ViewAuditLog};
    return kPermissions;
}

Lecturer::Lecturer(int id, std::string username, std::string fullName, int departmentId)
    : User(id, std::move(username), std::move(fullName), departmentId) {}

const std::set<Permission>& Lecturer::permissions() const noexcept {
    static const std::set<Permission> kPermissions{
        Permission::CreateAssignment, Permission::GrantExtension, Permission::MarkSubmission};
    return kPermissions;
}

HeadOfDepartment::HeadOfDepartment(int id, std::string username, std::string fullName,
                                   int departmentId)
    : User(id, std::move(username), std::move(fullName), departmentId) {}

bool HeadOfDepartment::hasDepartmentScope(int departmentId) const {
    return this->departmentId().has_value() && *this->departmentId() == departmentId;
}

const std::set<Permission>& HeadOfDepartment::permissions() const noexcept {
    static const std::set<Permission> kPermissions{
        Permission::ModerateMarks, Permission::ReleaseMarks, Permission::ViewAuditLog};
    return kPermissions;
}

Student::Student(int id, std::string username, std::string fullName,
                 std::optional<int> departmentId)
    : User(id, std::move(username), std::move(fullName), departmentId) {}

const std::set<Permission>& Student::permissions() const noexcept {
    static const std::set<Permission> kPermissions{Permission::SubmitWork,
                                                   Permission::ViewReleasedResults};
    return kPermissions;
}

std::unique_ptr<User> UserFactory::create(const UserRecord& r) {
    switch (r.role) {
        case Role::Administrator:
            return std::make_unique<Administrator>(r.id, r.username, r.fullName);
        case Role::Lecturer:
            if (!r.departmentId) {
                throw ValidationError("A lecturer must belong to a department.");
            }
            return std::make_unique<Lecturer>(r.id, r.username, r.fullName, *r.departmentId);
        case Role::HeadOfDepartment:
            if (!r.departmentId) {
                throw ValidationError("A Head of Department must belong to a department.");
            }
            return std::make_unique<HeadOfDepartment>(r.id, r.username, r.fullName,
                                                      *r.departmentId);
        case Role::Student:
            return std::make_unique<Student>(r.id, r.username, r.fullName, r.departmentId);
    }
    throw ValidationError("Unknown role");
}

}  // namespace lms
