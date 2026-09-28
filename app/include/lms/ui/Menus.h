#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "lms/domain/User.h"
#include "lms/services/LmsSystem.h"
#include "lms/ui/ConsoleIO.h"

namespace lms::ui {

struct MenuItem {
    std::string label;
    std::function<void()> action;
};

/// Template Method: run() fixes the session skeleton (greeting, notifications, the menu
/// loop and error handling) while each role supplies its own title() and items().
class RoleMenu {
public:
    RoleMenu(LmsSystem& system, ConsoleIO& io, const User& user);
    virtual ~RoleMenu() = default;
    RoleMenu(const RoleMenu&) = delete;
    RoleMenu& operator=(const RoleMenu&) = delete;

    /// Runs until the user chooses "Log out".
    void run();

protected:
    virtual std::string title() const = 0;
    virtual std::vector<MenuItem> items() = 0;

    // Shared helpers for the concrete menus.
    void showNotifications(bool unreadOnly);
    void changePassword();
    void printAudit(const std::vector<MarkAuditEntry>& entries);
    std::string userName(int userId);
    std::string courseLabel(int courseId);

    LmsSystem& sys_;
    ConsoleIO& io_;
    const User& user_;
};

class AdminMenu final : public RoleMenu {
public:
    using RoleMenu::RoleMenu;

protected:
    std::string title() const override { return "Administrator menu"; }
    std::vector<MenuItem> items() override;

private:
    void listUsers();
    void createUser();
    void toggleActive();
    void resetPassword();
    void listDepartments();
    void createDepartment();
    void listCourses();
    void createCourse();
    void assignLecturer();
    void enrolStudent();
};

class LecturerMenu final : public RoleMenu {
public:
    using RoleMenu::RoleMenu;

protected:
    std::string title() const override { return "Lecturer menu"; }
    std::vector<MenuItem> items() override;

private:
    void listCoursesAndAssignments();
    void createAssignment();
    void viewSubmissions();
    void markSubmission();
    void grantExtension();
    void viewExtensions();
    int chooseOwnCourse();
};

class HodMenu final : public RoleMenu {
public:
    using RoleMenu::RoleMenu;

protected:
    std::string title() const override { return "Head of Department menu"; }
    std::vector<MenuItem> items() override;

private:
    void showQueue(bool pending);
    void approve();
    void adjust();
    void release();
    void submissionAudit();
    void departmentAudit();
};

class StudentMenu final : public RoleMenu {
public:
    using RoleMenu::RoleMenu;

protected:
    std::string title() const override { return "Student menu"; }
    std::vector<MenuItem> items() override;

private:
    void listCourses();
    void listAssignments();
    void submitWork();
    void viewResults();
};

/// Factory: chooses the menu class for the user's role.
std::unique_ptr<RoleMenu> createMenuFor(LmsSystem& system, ConsoleIO& io, const User& user);

}  // namespace lms::ui
