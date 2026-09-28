#include "lms/domain/Role.h"

#include "lms/common/Errors.h"

namespace lms {

std::string roleToCode(Role role) {
    switch (role) {
        case Role::Administrator:
            return "ADMIN";
        case Role::Lecturer:
            return "LECTURER";
        case Role::HeadOfDepartment:
            return "HOD";
        case Role::Student:
            return "STUDENT";
    }
    throw ValidationError("Unknown role");
}

Role roleFromCode(const std::string& code) {
    if (code == "ADMIN") {
        return Role::Administrator;
    }
    if (code == "LECTURER") {
        return Role::Lecturer;
    }
    if (code == "HOD") {
        return Role::HeadOfDepartment;
    }
    if (code == "STUDENT") {
        return Role::Student;
    }
    throw ValidationError("Unknown role code: " + code);
}

std::string roleDisplayName(Role role) {
    switch (role) {
        case Role::Administrator:
            return "Administrator";
        case Role::Lecturer:
            return "Lecturer";
        case Role::HeadOfDepartment:
            return "Head of Department";
        case Role::Student:
            return "Student";
    }
    return "Unknown";
}

std::string permissionName(Permission p) {
    switch (p) {
        case Permission::ManageUsers:
            return "manage user accounts";
        case Permission::ManageDepartments:
            return "manage departments";
        case Permission::ManageCourses:
            return "manage courses";
        case Permission::AssignLecturers:
            return "assign lecturers to courses";
        case Permission::EnrolStudents:
            return "enrol students";
        case Permission::CreateAssignment:
            return "create assignments";
        case Permission::GrantExtension:
            return "grant extensions";
        case Permission::MarkSubmission:
            return "mark submissions";
        case Permission::SubmitWork:
            return "submit work";
        case Permission::ViewReleasedResults:
            return "view released results";
        case Permission::ModerateMarks:
            return "moderate marks";
        case Permission::ReleaseMarks:
            return "release marks";
        case Permission::ViewAuditLog:
            return "view the mark audit log";
    }
    return "perform this action";
}

}  // namespace lms
