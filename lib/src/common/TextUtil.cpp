#include "lms/common/TextUtil.h"

#include <cmath>
#include <iomanip>
#include <sstream>

#include "lms/common/Errors.h"

namespace lms::text {

std::string trim(const std::string& s) {
    const char* whitespace = " \t\r\n\f\v";
    const auto first = s.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return {};
    }
    const auto last = s.find_last_not_of(whitespace);
    return s.substr(first, last - first + 1);
}

std::string requireText(const std::string& value, const std::string& field, std::size_t maxLength) {
    std::string trimmed = trim(value);
    if (trimmed.empty()) {
        throw ValidationError(field + " must not be empty.");
    }
    if (trimmed.size() > maxLength) {
        throw ValidationError(field + " must be at most " + std::to_string(maxLength) +
                              " characters.");
    }
    return trimmed;
}

std::string formatMark(double mark) {
    const double rounded = std::round(mark * 100.0) / 100.0;
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << rounded;
    std::string s = out.str();
    // Drop redundant trailing zeros: 17.50 -> 17.5, 20.00 -> 20.
    while (!s.empty() && s.back() == '0') {
        s.pop_back();
    }
    if (!s.empty() && s.back() == '.') {
        s.pop_back();
    }
    return s;
}

}  // namespace lms::text
