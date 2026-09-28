#include <cstring>
#include <exception>
#include <iostream>
#include <string>

#include "lms/services/LmsSystem.h"
#include "lms/ui/Application.h"
#include "lms/ui/ConsoleIO.h"

#ifdef _WIN32
#include <io.h>
#define LMS_ISATTY _isatty
#define LMS_FILENO _fileno
#else
#include <unistd.h>
#define LMS_ISATTY isatty
#define LMS_FILENO fileno
#endif

namespace {

void printUsage() {
    std::cout << "Usage: lms [--db <file>] [--seed-demo] [--version] [--help]\n"
                 "  --db <file>   SQLite database file (default: lms.db in the current folder)\n"
                 "  --seed-demo   create a demonstration school in an empty database\n"
                 "  --version     print the version and exit\n";
}

}  // namespace

// The signature of main is fixed by the C++ standard.
// cppcheck-suppress constParameter
int main(int argc, char* argv[]) {
    std::string databasePath = "lms.db";
    bool seedDemo = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--db") {
            if (i + 1 >= argc) {
                std::cerr << "--db needs a file name.\n";
                return 2;
            }
            databasePath = argv[++i];
        } else if (arg == "--seed-demo") {
            seedDemo = true;
        } else if (arg == "--version") {
            std::cout << "Moderated LMS " << LMS_VERSION << '\n';
            return 0;
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << '\n';
            printUsage();
            return 2;
        }
    }

    try {
        std::unique_ptr<lms::LmsSystem> system = lms::LmsSystem::createDefault(databasePath);
        if (seedDemo) {
            if (lms::ui::seedDemoData(*system)) {
                std::cout << "Demo data created. Every demo account uses the password '"
                          << lms::ui::kDemoPassword << "'.\n";
            } else {
                std::cout << "The database already contains data; demo data was not added.\n";
            }
        }
        const bool interactive = LMS_ISATTY(LMS_FILENO(stdin)) != 0;
        lms::ui::ConsoleIO io(std::cin, std::cout, interactive);
        lms::ui::Application app(*system, io);
        return app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }
}
