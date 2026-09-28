#include "lms/services/AdminService.h"

#include <algorithm>
#include <cctype>

#include "lms/common/Errors.h"
#include "lms/common/TextUtil.h"

namespace lms {

AdminService::AdminService(Repositories repos, const IPasswordHasher& hasher, const IClock& clock)
    : repos_(repos), hasher_(hasher), clock_(clock) {}

std::string AdminService::validateUsername(const std::string& username) {
    std::string name = text::trim(username);
    if (name.size() < 3 || name.size() > 32) {
        throw ValidationError("Username must be 3 to 32 characters long.");
    }
    const bool allowed = std::all_of(name.begin(), name.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '.' || c == '_' ||
               c == '-';
    });
    if (!allowed) {
        throw ValidationError(
            "Username may contain only letters, digits, '.', '_' and '-'.");
    }
    return name;
}

bool AdminService::needsBootstrap() const {
    return repos_.users.countByRole(Role::Administrator) == 0;
}

int AdminService::bootstrapAdministrator(const std::string& username, const std::string& fullName,
                                         const std::string& password) {
    if (!needsBootstrap()) {
        throw BusinessRuleError("An administrator account already exists.");
    }
    UserRecord record;
    record.username = validateUsername(username);
    record.fullName = text::requireText(fullName, "Full name", 100);
    PasswordPolicy::validate(password, record.username);
    record.passwordHash = hasher_.hash(password);
    record.role = Role::Administrator;
    record.createdAt = clock_.now();
    return repos_.users.add(record);
}

int AdminService::createUser(const User& actor, Role role, const std::string& username,
                             const std::string& fullName, const std::string& initialPassword,
                             std::optional<int> departmentId) {
    actor.require(Permission::ManageUsers);
    UserRecord record;
    record.username = validateUsername(username);
    record.fullName = text::requireText(fullName, "Full name", 100);
    record.role = role;
    PasswordPolicy::validate(initialPassword, record.username);

    if (repos_.users.findByUsername(record.username)) {
        throw ValidationError("The username '" + record.username + "' is already taken.");
    }
    const bool needsDepartment = role == Role::Lecturer || role == Role::HeadOfDepartment;
    if (role == Role::Administrator) {
        departmentId.reset();
    }
    if (needsDepartment && !departmentId) {
        throw ValidationError("A " + roleDisplayName(role) + " must belong to a department.");
    }
    if (departmentId && !repos_.departments.findById(*departmentId)) {
        throw NotFoundError("Department " + std::to_string(*departmentId) + " does not exist.");
    }
    if (role == Role::HeadOfDepartment && repos_.users.findHeadOfDepartment(*departmentId)) {
        throw BusinessRuleError("That department already has an active Head of Department.");
    }
    record.departmentId = departmentId;
    record.passwordHash = hasher_.hash(initialPassword);
    record.createdAt = clock_.now();
    return repos_.users.add(record);
}

UserRecord AdminService::requireUser(int userId) const {
    std::optional<UserRecord> record = repos_.users.findById(userId);
    if (!record) {
        throw NotFoundError("User " + std::to_string(userId) + " does not exist.");
    }
    return *record;
}

Course AdminService::requireCourse(int courseId) const {
    std::optional<Course> course = repos_.courses.findById(courseId);
    if (!course) {
        throw NotFoundError("Course " + std::to_string(courseId) + " does not exist.");
    }
    return *course;
}

void AdminService::setActive(const User& actor, int userId, bool active) {
    actor.require(Permission::ManageUsers);
    if (userId == actor.id() && !active) {
        throw BusinessRuleError("You cannot deactivate your own account.");
    }
    UserRecord record = requireUser(userId);
    if (active && record.role == Role::HeadOfDepartment && record.departmentId) {
        std::optional<UserRecord> current = repos_.users.findHeadOfDepartment(*record.departmentId);
        if (current && current->id != record.id) {
            throw BusinessRuleError("That department already has an active Head of Department.");
        }
    }
    record.active = active;
    repos_.users.update(record);
}

void AdminService::resetPassword(const User& actor, int userId, const std::string& newPassword) {
    actor.require(Permission::ManageUsers);
    UserRecord record = requireUser(userId);
    PasswordPolicy::validate(newPassword, record.username);
    record.passwordHash = hasher_.hash(newPassword);
    record.failedLoginAttempts = 0;
    record.lockedUntil = 0;
    repos_.users.update(record);
}

std::vector<UserSummary> AdminService::listUsers(const User& actor) const {
    actor.require(Permission::ManageUsers);
    const Timestamp now = clock_.now();
    std::vector<UserSummary> result;
    for (const UserRecord& r : repos_.users.listAll()) {
        result.push_back(UserSummary{r.id, r.username, r.fullName, r.role, r.departmentId,
                                     r.active, r.lockedUntil > now});
    }
    return result;
}

int AdminService::createDepartment(const User& actor, const std::string& name) {
    actor.require(Permission::ManageDepartments);
    const std::string clean = text::requireText(name, "Department name", 100);
    for (const Department& d : repos_.departments.listAll()) {
        std::string a = d.name;
        std::string b = clean;
        const auto lower = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };
        std::transform(a.begin(), a.end(), a.begin(), lower);
        std::transform(b.begin(), b.end(), b.begin(), lower);
        if (a == b) {
            throw ValidationError("A department called '" + d.name + "' already exists.");
        }
    }
    return repos_.departments.add(clean);
}

std::vector<Department> AdminService::listDepartments(const User& actor) const {
    actor.require(Permission::ManageDepartments);
    return repos_.departments.listAll();
}

int AdminService::createCourse(const User& actor, const std::string& code,
                               const std::string& title, int departmentId) {
    actor.require(Permission::ManageCourses);
    Course course;
    course.code = text::requireText(code, "Course code", 20);
    course.title = text::requireText(title, "Course title", 100);
    if (!repos_.departments.findById(departmentId)) {
        throw NotFoundError("Department " + std::to_string(departmentId) + " does not exist.");
    }
    for (const Course& existing : repos_.courses.listAll()) {
        if (existing.code.size() == course.code.size() &&
            std::equal(existing.code.begin(), existing.code.end(), course.code.begin(),
                       [](char x, char y) {
                           return std::tolower(static_cast<unsigned char>(x)) ==
                                  std::tolower(static_cast<unsigned char>(y));
                       })) {
            throw ValidationError("Course code '" + course.code + "' is already in use.");
        }
    }
    course.departmentId = departmentId;
    return repos_.courses.add(course);
}

std::vector<Course> AdminService::listCourses(const User& actor) const {
    actor.require(Permission::ManageCourses);
    return repos_.courses.listAll();
}

void AdminService::assignLecturer(const User& actor, int courseId, int lecturerId) {
    actor.require(Permission::AssignLecturers);
    const Course course = requireCourse(courseId);
    const UserRecord lecturer = requireUser(lecturerId);
    if (lecturer.role != Role::Lecturer || !lecturer.active) {
        throw BusinessRuleError(lecturer.username + " is not an active lecturer.");
    }
    if (lecturer.departmentId != course.departmentId) {
        throw BusinessRuleError("A lecturer can only teach courses of their own department.");
    }
    repos_.courses.assignLecturer(course.id, lecturer.id);
}

void AdminService::enrolStudent(const User& actor, int courseId, int studentId) {
    actor.require(Permission::EnrolStudents);
    const Course course = requireCourse(courseId);
    const UserRecord student = requireUser(studentId);
    if (student.role != Role::Student || !student.active) {
        throw BusinessRuleError(student.username + " is not an active student.");
    }
    repos_.courses.enrol(course.id, student.id, clock_.now());
}

}  // namespace lms
