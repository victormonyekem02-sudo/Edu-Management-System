#include "lms/ui/Application.h"

#include "lms/common/Errors.h"
#include "lms/ui/Menus.h"

namespace lms::ui {

Application::Application(LmsSystem& system, ConsoleIO& io) : sys_(system), io_(io) {}

int Application::run() {
    io_.heading("Learning Management System v" LMS_VERSION);
    try {
        if (sys_.admin().needsBootstrap()) {
            bootstrap();
        }
        for (;;) {
            io_.println();
            io_.println("1. Log in");
            io_.println("0. Exit");
            if (io_.readInt("Choose an option", 0, 1) == 0) {
                io_.println("Goodbye.");
                return 0;
            }
            loginAndServe();
        }
    } catch (const InputClosed&) {
        io_.println();
        io_.println("Input closed - exiting.");
        return 0;
    }
}

void Application::bootstrap() {
    io_.info("First run: no administrator account exists. Create one now.");
    for (;;) {
        try {
            const std::string username = io_.readRequired("Administrator username");
            const std::string fullName = io_.readRequired("Full name");
            const std::string password =
                io_.readPassword("Password (min 8 chars, letters and digits)");
            const std::string again = io_.readPassword("Repeat password");
            if (password != again) {
                throw ValidationError("The passwords do not match.");
            }
            sys_.admin().bootstrapAdministrator(username, fullName, password);
            io_.success("Administrator created. Please log in.");
            return;
        } catch (const LmsError& e) {
            io_.error(e.what());
        }
    }
}

void Application::loginAndServe() {
    const std::string username = io_.readRequired("Username");
    const std::string password = io_.readPassword("Password");
    std::unique_ptr<User> user;
    try {
        user = sys_.auth().login(username, password);
    } catch (const AuthenticationError& e) {
        io_.error(e.what());
        return;
    }
    std::unique_ptr<RoleMenu> menu = createMenuFor(sys_, io_, *user);
    menu->run();
}

}  // namespace lms::ui
