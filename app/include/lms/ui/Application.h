#pragma once

#include "lms/services/LmsSystem.h"
#include "lms/ui/ConsoleIO.h"

namespace lms::ui {

/// Top-level console application: first-run set-up, login loop, role menus.
class Application {
public:
    Application(LmsSystem& system, ConsoleIO& io);
    /// Returns the process exit code.
    int run();

private:
    void bootstrap();
    void loginAndServe();

    LmsSystem& sys_;
    ConsoleIO& io_;
};

/// Creates a small demonstration school (departments, users, courses, assignments).
/// Only runs on an empty database. Returns false if data already existed.
bool seedDemoData(LmsSystem& system);

/// Password given to every demo account.
inline constexpr const char* kDemoPassword = "Passw0rd2026";

}  // namespace lms::ui
