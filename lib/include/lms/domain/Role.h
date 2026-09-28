#pragma once

#include <string>

namespace lms {

enum class Role { Administrator, Lecturer, HeadOfDepartment, Student };

/// Database/storage code of a role: ADMIN, LECTURER, HOD, STUDENT.
std::string roleToCode(Role role);

/// Inverse of roleToCode. Throws ValidationError for an unknown code.
Role roleFromCode(const std::string& code);

/// Display name, e.g. "Head of Department".
std::string roleDisplayName(Role role);

/// Permissions granted to roles (FR-02). Checked by every service operation.
enum class Permission {
    ManageUsers,
    ManageDepartments,
    ManageCourses,
    AssignLecturers,
    EnrolStudents,
    CreateAssignment,
    GrantExtension,
    MarkSubmission,
    SubmitWork,
    ViewReleasedResults,
    ModerateMarks,
    ReleaseMarks,
    ViewAuditLog
};

std::string permissionName(Permission p);

}  // namespace lms
