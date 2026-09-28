#include <iostream>

#include "lms/common/Errors.h"
#include "lms/common/TextUtil.h"
#include "lms/common/TimeUtil.h"
#include "lms/ui/Menus.h"

namespace lms::ui {

RoleMenu::RoleMenu(LmsSystem& system, ConsoleIO& io, const User& user)
    : sys_(system), io_(io), user_(user) {}

void RoleMenu::run() {
    io_.heading("Welcome, " + user_.fullName() + " (" + user_.roleTitle() + ")");
    showNotifications(true);

    for (;;) {
        std::vector<MenuItem> menu = items();
        menu.push_back({"View all my notifications", [this] { showNotifications(false); }});
        menu.push_back({"Change my password", [this] { changePassword(); }});

        io_.heading(title());
        for (std::size_t i = 0; i < menu.size(); ++i) {
            io_.println(std::to_string(i + 1) + ". " + menu[i].label);
        }
        io_.println("0. Log out");
        const int choice = io_.readInt("Choose an option", 0, static_cast<int>(menu.size()));
        if (choice == 0) {
            io_.info("Logged out.");
            return;
        }
        try {
            menu[static_cast<std::size_t>(choice - 1)].action();
        } catch (const InputClosed&) {
            throw;
        } catch (const LmsError& e) {
            io_.error(e.what());
        } catch (const std::exception& e) {
            // Unexpected failure: report it and keep the session alive rather than crash.
            io_.error(std::string("Unexpected problem: ") + e.what());
        }
    }
}

void RoleMenu::showNotifications(bool unreadOnly) {
    const std::vector<Notification> list =
        unreadOnly ? sys_.notifications().unreadFor(user_) : sys_.notifications().allFor(user_);
    if (unreadOnly && list.empty()) {
        return;
    }
    io_.heading(unreadOnly ? "New notifications" : "All notifications");
    std::vector<std::vector<std::string>> rows;
    for (const Notification& n : list) {
        rows.push_back({time_util::formatLocal(n.createdAt), n.read ? "" : "NEW", n.message});
    }
    io_.table({"When", "", "Message"}, rows);
    sys_.notifications().markAllRead(user_);
}

void RoleMenu::changePassword() {
    const std::string current = io_.readPassword("Current password");
    const std::string first = io_.readPassword("New password (min 8 chars, letters and digits)");
    const std::string second = io_.readPassword("Repeat new password");
    if (first != second) {
        throw ValidationError("The two new passwords do not match.");
    }
    sys_.auth().changePassword(user_, current, first);
    io_.success("Password changed.");
}

std::string RoleMenu::userName(int userId) {
    return sys_.directory().displayName(userId);
}

std::string RoleMenu::courseLabel(int courseId) {
    if (std::optional<Course> c = sys_.directory().findCourse(courseId)) {
        return c->code + " " + c->title;
    }
    return "#" + std::to_string(courseId);
}

void RoleMenu::printAudit(const std::vector<MarkAuditEntry>& entries) {
    std::vector<std::vector<std::string>> rows;
    for (const MarkAuditEntry& e : entries) {
        rows.push_back({time_util::formatLocal(e.changedAt), std::to_string(e.submissionId),
                        userName(e.changedBy), auditActionToCode(e.action),
                        e.oldMark ? text::formatMark(*e.oldMark) : "-",
                        e.newMark ? text::formatMark(*e.newMark) : "-", e.reason});
    }
    io_.table({"When", "Subm.", "Changed by", "Action", "Old", "New", "Reason"}, rows);
}

std::unique_ptr<RoleMenu> createMenuFor(LmsSystem& system, ConsoleIO& io, const User& user) {
    switch (user.role()) {
        case Role::Administrator:
            return std::make_unique<AdminMenu>(system, io, user);
        case Role::Lecturer:
            return std::make_unique<LecturerMenu>(system, io, user);
        case Role::HeadOfDepartment:
            return std::make_unique<HodMenu>(system, io, user);
        case Role::Student:
            return std::make_unique<StudentMenu>(system, io, user);
    }
    throw AuthorizationError("No menu exists for this role.");
}

}  // namespace lms::ui
