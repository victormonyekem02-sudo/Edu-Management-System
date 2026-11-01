#include <array>

#include "lms/common/Errors.h"
#include "lms/ui/Menus.h"

namespace lms::ui {

std::vector<MenuItem> AdminMenu::items() {
    return {
        {"List user accounts", [this] { listUsers(); }},
        {"Create user account", [this] { createUser(); }},
        {"Activate / deactivate an account", [this] { toggleActive(); }},
        {"Reset a user's password", [this] { resetPassword(); }},
        {"List departments", [this] { listDepartments(); }},
        {"Create department", [this] { createDepartment(); }},
        {"List courses", [this] { listCourses(); }},
        {"Create course", [this] { createCourse(); }},
        {"Assign lecturer to course", [this] { assignLecturer(); }},
        {"Enrol student on course", [this] { enrolStudent(); }},
        {"View mark audit log (whole school)",
         [this] { printAudit(sys_.moderation().auditLog(user_)); }},
    };
}

void AdminMenu::listUsers() {
    std::vector<std::vector<std::string>> rows;
    for (const UserSummary& u : sys_.admin().listUsers(user_)) {
        std::string dept = "-";
        if (u.departmentId) {
            if (auto d = sys_.directory().findDepartment(*u.departmentId)) {
                dept = d->name;
            }
        }
        std::string state = u.active ? "active" : "inactive";
        if (u.locked) {
            state += ", locked";
        }
        rows.push_back({std::to_string(u.id), u.username, u.fullName, roleDisplayName(u.role),
                        dept, state});
    }
    io_.table({"ID", "Username", "Name", "Role", "Department", "State"}, rows);
}

void AdminMenu::createUser() {
    io_.println("Roles: 1 Administrator, 2 Lecturer, 3 Head of Department, 4 Student");
    const int roleChoice = io_.readInt("Role", 1, 4);
    static constexpr std::array<Role, 4> kRoles{Role::Administrator, Role::Lecturer,
                                                Role::HeadOfDepartment, Role::Student};
    const Role role = kRoles.at(static_cast<std::size_t>(roleChoice - 1));
    const std::string username = io_.readRequired("Username");
    const std::string fullName = io_.readRequired("Full name");
    std::optional<int> department;
    if (role != Role::Administrator) {
        listDepartments();
        const std::string hint = role == Role::Student ? " (blank for none)" : "";
        department = io_.readOptionalInt("Department ID" + hint, 1, 1000000);
    }
    const std::string password =
        io_.readPassword("Initial password (min 8 chars, letters and digits)");
    const int id =
        sys_.admin().createUser(user_, role, username, fullName, password, department);
    io_.success("Created " + roleDisplayName(role) + " '" + username + "' with ID " +
                std::to_string(id) + ".");
}

void AdminMenu::toggleActive() {
    listUsers();
    const int id = io_.readInt("User ID", 1, 1000000000);
    const bool activate = io_.confirm("Activate the account? (answer n to deactivate)");
    sys_.admin().setActive(user_, id, activate);
    io_.success(activate ? "Account activated." : "Account deactivated.");
}

void AdminMenu::resetPassword() {
    listUsers();
    const int id = io_.readInt("User ID", 1, 1000000000);
    const std::string password = io_.readPassword("New password");
    sys_.admin().resetPassword(user_, id, password);
    io_.success("Password reset; the account is unlocked.");
}

void AdminMenu::listDepartments() {
    std::vector<std::vector<std::string>> rows;
    for (const Department& d : sys_.admin().listDepartments(user_)) {
        std::string hod = "-";
        if (auto h = sys_.directory().headOfDepartment(d.id)) {
            hod = h->fullName;
        }
        rows.push_back({std::to_string(d.id), d.name, hod});
    }
    io_.table({"ID", "Department", "Head of Department"}, rows);
}

void AdminMenu::createDepartment() {
    const std::string name = io_.readRequired("Department name");
    const int id = sys_.admin().createDepartment(user_, name);
    io_.success("Department created with ID " + std::to_string(id) + ".");
}

void AdminMenu::listCourses() {
    std::vector<std::vector<std::string>> rows;
    for (const Course& c : sys_.admin().listCourses(user_)) {
        std::string lecturers;
        for (const UserSummary& l : sys_.directory().lecturersOf(c.id)) {
            if (!lecturers.empty()) {
                lecturers += ", ";
            }
            lecturers += l.fullName;
        }
        const auto students = sys_.directory().studentsOf(c.id).size();
        std::string dept;
        if (auto d = sys_.directory().findDepartment(c.departmentId)) {
            dept = d->name;
        }
        rows.push_back({std::to_string(c.id), c.code, c.title, dept,
                        lecturers.empty() ? "-" : lecturers, std::to_string(students)});
    }
    io_.table({"ID", "Code", "Title", "Department", "Lecturers", "Students"}, rows);
}

void AdminMenu::createCourse() {
    const std::string code = io_.readRequired("Course code (e.g. MAT101)");
    const std::string courseTitle = io_.readRequired("Course title");
    listDepartments();
    const int dept = io_.readInt("Department ID", 1, 1000000);
    const int id = sys_.admin().createCourse(user_, code, courseTitle, dept);
    io_.success("Course created with ID " + std::to_string(id) + ".");
}

void AdminMenu::assignLecturer() {
    listCourses();
    const int course = io_.readInt("Course ID", 1, 1000000);
    std::vector<std::vector<std::string>> rows;
    for (const UserSummary& u : sys_.directory().usersWithRole(Role::Lecturer)) {
        rows.push_back({std::to_string(u.id), u.username, u.fullName});
    }
    io_.table({"ID", "Username", "Lecturer"}, rows);
    const int lecturer = io_.readInt("Lecturer ID", 1, 1000000000);
    sys_.admin().assignLecturer(user_, course, lecturer);
    io_.success("Lecturer assigned.");
}

void AdminMenu::enrolStudent() {
    listCourses();
    const int course = io_.readInt("Course ID", 1, 1000000);
    const std::string username = io_.readRequired("Student username");
    auto student = sys_.directory().findUserByUsername(username);
    if (!student) {
        throw NotFoundError("No user called '" + username + "'.");
    }
    sys_.admin().enrolStudent(user_, course, student->id);
    io_.success(student->fullName + " enrolled.");
}

}  // namespace lms::ui
