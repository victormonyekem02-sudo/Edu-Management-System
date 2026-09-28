#include "lms/ui/ConsoleIO.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

#include "lms/common/TextUtil.h"
#include "lms/common/TimeUtil.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace lms::ui {

namespace {

/// RAII: disables terminal echo for the lifetime of the object.
class EchoGuard {
public:
    explicit EchoGuard(bool active) : active_(active) {
        if (!active_) {
            return;
        }
#ifdef _WIN32
        handle_ = GetStdHandle(STD_INPUT_HANDLE);
        if (handle_ == INVALID_HANDLE_VALUE || GetConsoleMode(handle_, &oldMode_) == 0) {
            active_ = false;
            return;
        }
        SetConsoleMode(handle_, oldMode_ & ~static_cast<DWORD>(ENABLE_ECHO_INPUT));
#else
        if (!isatty(STDIN_FILENO) || tcgetattr(STDIN_FILENO, &old_) != 0) {
            active_ = false;
            return;
        }
        termios noEcho = old_;
        noEcho.c_lflag &= ~static_cast<tcflag_t>(ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &noEcho);
#endif
    }
    ~EchoGuard() {
        if (!active_) {
            return;
        }
#ifdef _WIN32
        SetConsoleMode(handle_, oldMode_);
#else
        tcsetattr(STDIN_FILENO, TCSANOW, &old_);
#endif
    }
    EchoGuard(const EchoGuard&) = delete;
    EchoGuard& operator=(const EchoGuard&) = delete;
    EchoGuard(EchoGuard&&) = delete;
    EchoGuard& operator=(EchoGuard&&) = delete;

private:
    bool active_;
#ifdef _WIN32
    HANDLE handle_ = nullptr;
    DWORD oldMode_ = 0;
#else
    termios old_{};
#endif
};

}  // namespace

ConsoleIO::ConsoleIO(std::istream& in, std::ostream& out, bool interactiveTerminal)
    : in_(in), out_(out), interactive_(interactiveTerminal) {}

std::string ConsoleIO::nextLine() {
    std::string line;
    if (!std::getline(in_, line)) {
        throw InputClosed();
    }
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    return line;
}

void ConsoleIO::println(const std::string& text) {
    out_ << text << '\n';
}

void ConsoleIO::heading(const std::string& text) {
    out_ << '\n' << text << '\n' << std::string(text.size(), '=') << '\n';
}

void ConsoleIO::success(const std::string& text) {
    out_ << "[OK] " << text << '\n';
}

void ConsoleIO::error(const std::string& text) {
    out_ << "[ERROR] " << text << '\n';
}

void ConsoleIO::info(const std::string& text) {
    out_ << "[i] " << text << '\n';
}

std::string ConsoleIO::readLine(const std::string& prompt) {
    out_ << prompt << ": " << std::flush;
    return nextLine();
}

std::string ConsoleIO::readRequired(const std::string& prompt) {
    for (;;) {
        std::string value = text::trim(readLine(prompt));
        if (!value.empty()) {
            return value;
        }
        error("A value is required.");
    }
}

int ConsoleIO::readInt(const std::string& prompt, int min, int max) {
    for (;;) {
        std::optional<int> value = readOptionalInt(prompt, min, max);
        if (value) {
            return *value;
        }
        error("A number is required.");
    }
}

std::optional<int> ConsoleIO::readOptionalInt(const std::string& prompt, int min, int max) {
    for (;;) {
        const std::string raw = text::trim(readLine(prompt));
        if (raw.empty()) {
            return std::nullopt;
        }
        std::istringstream parser(raw);
        long long value = 0;
        char extra = '\0';
        if (parser >> value && !(parser >> extra) && value >= min && value <= max) {
            return static_cast<int>(value);
        }
        error("Please enter a whole number from " + std::to_string(min) + " to " +
              std::to_string(max) + ".");
    }
}

double ConsoleIO::readDouble(const std::string& prompt, double min, double max) {
    for (;;) {
        const std::string raw = text::trim(readLine(prompt));
        std::istringstream parser(raw);
        double value = 0.0;
        char extra = '\0';
        if (!raw.empty() && parser >> value && !(parser >> extra) && std::isfinite(value) &&
            value >= min && value <= max) {
            return value;
        }
        error("Please enter a number from " + text::formatMark(min) + " to " +
              text::formatMark(max) + ".");
    }
}

Timestamp ConsoleIO::readDateTime(const std::string& prompt) {
    for (;;) {
        const std::string raw = text::trim(readLine(prompt + " (YYYY-MM-DD HH:MM)"));
        if (std::optional<Timestamp> t = time_util::parseLocal(raw)) {
            return *t;
        }
        error("That is not a valid date and time. Example: 2026-10-15 17:00");
    }
}

std::string ConsoleIO::readPassword(const std::string& prompt) {
    out_ << prompt << ": " << std::flush;
    std::string value;
    {
        const EchoGuard guard(interactive_);
        value = nextLine();
    }
    if (interactive_) {
        out_ << '\n';
    }
    return value;
}

bool ConsoleIO::confirm(const std::string& prompt) {
    for (;;) {
        std::string answer = text::trim(readLine(prompt + " (y/n)"));
        std::transform(answer.begin(), answer.end(), answer.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (answer == "y" || answer == "yes") {
            return true;
        }
        if (answer == "n" || answer == "no") {
            return false;
        }
        error("Please answer y or n.");
    }
}

std::string ConsoleIO::readMultiline(const std::string& prompt) {
    out_ << prompt << " (finish with a line containing only '.'):\n";
    std::string result;
    for (;;) {
        const std::string line = nextLine();
        if (line == ".") {
            break;
        }
        if (!result.empty()) {
            result += '\n';
        }
        result += line;
    }
    return result;
}

void ConsoleIO::table(const std::vector<std::string>& headers,
                      const std::vector<std::vector<std::string>>& rows) {
    if (rows.empty()) {
        info("(none)");
        return;
    }
    std::vector<std::size_t> widths(headers.size());
    for (std::size_t c = 0; c < headers.size(); ++c) {
        widths[c] = headers[c].size();
        for (const auto& row : rows) {
            if (c < row.size()) {
                widths[c] = std::max(widths[c], row[c].size());
            }
        }
    }
    const auto printRow = [&](const std::vector<std::string>& cells) {
        for (std::size_t c = 0; c < headers.size(); ++c) {
            const std::string cell = c < cells.size() ? cells[c] : "";
            out_ << cell;
            if (c + 1 < headers.size()) {
                out_ << std::string(widths[c] - cell.size() + 2, ' ');
            }
        }
        out_ << '\n';
    };
    printRow(headers);
    std::size_t total = 0;
    for (const std::size_t w : widths) {
        total += w + 2;
    }
    out_ << std::string(total > 2 ? total - 2 : total, '-') << '\n';
    for (const auto& row : rows) {
        printRow(row);
    }
}

}  // namespace lms::ui
