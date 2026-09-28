#pragma once

#include <iosfwd>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "lms/common/Clock.h"

namespace lms::ui {

/// Thrown when standard input is closed (Ctrl+Z / Ctrl+D or end of a scripted test).
class InputClosed : public std::runtime_error {
public:
    InputClosed() : std::runtime_error("Input closed") {}
};

/// All console input/output goes through this class, which validates every value
/// and re-prompts on invalid input (NFR-08). Streams are injected so the system tests
/// can drive the real UI with scripted input.
class ConsoleIO {
public:
    ConsoleIO(std::istream& in, std::ostream& out, bool interactiveTerminal);

    void println(const std::string& text = "");
    void heading(const std::string& text);
    void success(const std::string& text);
    void error(const std::string& text);
    void info(const std::string& text);

    std::string readLine(const std::string& prompt);
    std::string readRequired(const std::string& prompt);
    int readInt(const std::string& prompt, int min, int max);
    /// Blank input returns std::nullopt.
    std::optional<int> readOptionalInt(const std::string& prompt, int min, int max);
    double readDouble(const std::string& prompt, double min, double max);
    /// Accepts "YYYY-MM-DD HH:MM" (local time).
    Timestamp readDateTime(const std::string& prompt);
    /// Reads a password without echoing it when attached to a real terminal.
    std::string readPassword(const std::string& prompt);
    bool confirm(const std::string& prompt);
    /// Reads lines until a line containing only "." (multi-line answers and feedback).
    std::string readMultiline(const std::string& prompt);

    /// Prints a plain-text table. Column widths adapt to the content; no colour is
    /// used to convey meaning, so output works with screen readers (NFR-10).
    void table(const std::vector<std::string>& headers,
               const std::vector<std::vector<std::string>>& rows);

private:
    std::string nextLine();

    std::istream& in_;
    std::ostream& out_;
    bool interactive_;
};

}  // namespace lms::ui
