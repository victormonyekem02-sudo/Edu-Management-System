#include "lms/services/DirectoryService.h"

namespace lms {

DirectoryService::DirectoryService(Repositories repos, const IClock& clock)
    : repos_(repos), clock_(clock) {}

UserSummary DirectoryService::summarise(const UserRecord& r) const {
    return UserSummary{r.id,           r.username, r.fullName,
                       r.role,         r.departmentId, r.active,
                       r.lockedUntil > clock_.now()};
}

std::optional<UserSummary> DirectoryService::findUser(int userId) const {
    if (std::optional<UserRecord> r = repos_.users.findById(userId)) {
        return summarise(*r);
    }
    return std::nullopt;
}

std::optional<UserSummary> DirectoryService::findUserByUsername(const std::string& username) const {
    if (std::optional<UserRecord> r = repos_.users.findByUsername(username)) {
        return summarise(*r);
    }
    return std::nullopt;
}

std::vector<UserSummary> DirectoryService::usersWithRole(Role role) const {
    std::vector<UserSummary> result;
    for (const UserRecord& r : repos_.users.listByRole(role)) {
        result.push_back(summarise(r));
    }
    return result;
}

std::optional<UserSummary> DirectoryService::headOfDepartment(int departmentId) const {
    if (std::optional<UserRecord> r = repos_.users.findHeadOfDepartment(departmentId)) {
        return summarise(*r);
    }
    return std::nullopt;
}

std::optional<Department> DirectoryService::findDepartment(int departmentId) const {
    return repos_.departments.findById(departmentId);
}

std::optional<Course> DirectoryService::findCourse(int courseId) const {
    return repos_.courses.findById(courseId);
}

std::optional<Assignment> DirectoryService::findAssignment(int assignmentId) const {
    return repos_.assignments.findById(assignmentId);
}

std::vector<UserSummary> DirectoryService::lecturersOf(int courseId) const {
    std::vector<UserSummary> result;
    for (int id : repos_.courses.lecturerIds(courseId)) {
        if (std::optional<UserSummary> u = findUser(id)) {
            result.push_back(*u);
        }
    }
    return result;
}

std::vector<UserSummary> DirectoryService::studentsOf(int courseId) const {
    std::vector<UserSummary> result;
    for (int id : repos_.courses.studentIds(courseId)) {
        if (std::optional<UserSummary> u = findUser(id)) {
            result.push_back(*u);
        }
    }
    return result;
}

std::string DirectoryService::displayName(int userId) const {
    if (std::optional<UserSummary> u = findUser(userId)) {
        return u->fullName + " (" + u->username + ")";
    }
    return "#" + std::to_string(userId);
}

}  // namespace lms
