#pragma once

#include <optional>
#include <string>
#include <vector>

#include "lms/common/Clock.h"
#include "lms/data/Repositories.h"
#include "lms/services/AdminService.h"

namespace lms {

/// Read-only look-ups used by the presentation layer to turn IDs into names.
/// It exposes UserSummary (never UserRecord), so password hashes and lockout counters
/// cannot leak into the user interface.
class DirectoryService {
public:
    DirectoryService(Repositories repos, const IClock& clock);

    std::optional<UserSummary> findUser(int userId) const;
    std::optional<UserSummary> findUserByUsername(const std::string& username) const;
    std::vector<UserSummary> usersWithRole(Role role) const;
    std::optional<UserSummary> headOfDepartment(int departmentId) const;
    std::optional<Department> findDepartment(int departmentId) const;
    std::optional<Course> findCourse(int courseId) const;
    std::optional<Assignment> findAssignment(int assignmentId) const;
    std::vector<UserSummary> lecturersOf(int courseId) const;
    std::vector<UserSummary> studentsOf(int courseId) const;

    /// "Full Name (username)" or "#id" when the user no longer exists.
    std::string displayName(int userId) const;

private:
    UserSummary summarise(const UserRecord& r) const;

    Repositories repos_;
    const IClock& clock_;
};

}  // namespace lms
