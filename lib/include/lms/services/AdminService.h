#pragma once

#include <optional>
#include <string>
#include <vector>

#include "lms/common/Clock.h"
#include "lms/data/Database.h"
#include "lms/data/Repositories.h"
#include "lms/domain/User.h"
#include "lms/security/PasswordHasher.h"

namespace lms {

/// Account information that is safe to show on screen (no password hash).
struct UserSummary {
    int id = 0;
    std::string username;
    std::string fullName;
    Role role = Role::Student;
    std::optional<int> departmentId;
    bool active = true;
    bool locked = false;
};

/// Administrator use cases: accounts, departments, courses, lecturer assignment and
/// enrolment (FR-03 .. FR-06).
class AdminService {
public:
    AdminService(Repositories repos, const IPasswordHasher& hasher, const IClock& clock);

    /// True when the database contains no active administrator (first run).
    bool needsBootstrap() const;
    /// Creates the first administrator; only allowed while needsBootstrap() is true.
    int bootstrapAdministrator(const std::string& username, const std::string& fullName,
                               const std::string& password);

    int createUser(const User& actor, Role role, const std::string& username,
                   const std::string& fullName, const std::string& initialPassword,
                   std::optional<int> departmentId);
    void setActive(const User& actor, int userId, bool active);
    /// Sets a new password and clears any lockout.
    void resetPassword(const User& actor, int userId, const std::string& newPassword);
    std::vector<UserSummary> listUsers(const User& actor) const;

    int createDepartment(const User& actor, const std::string& name);
    std::vector<Department> listDepartments(const User& actor) const;

    int createCourse(const User& actor, const std::string& code, const std::string& title,
                     int departmentId);
    std::vector<Course> listCourses(const User& actor) const;
    void assignLecturer(const User& actor, int courseId, int lecturerId);
    void enrolStudent(const User& actor, int courseId, int studentId);

private:
    UserRecord requireUser(int userId) const;
    Course requireCourse(int courseId) const;
    static std::string validateUsername(const std::string& username);

    Repositories repos_;
    const IPasswordHasher& hasher_;
    const IClock& clock_;
};

}  // namespace lms
