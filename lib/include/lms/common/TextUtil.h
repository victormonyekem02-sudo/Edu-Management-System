#pragma once

#include <string>

namespace lms::text {

/// Remove leading and trailing whitespace.
std::string trim(const std::string& s);

/// Throws ValidationError if the trimmed value is empty or longer than maxLength.
/// Returns the trimmed value. `field` is used in the error message.
std::string requireText(const std::string& value, const std::string& field, std::size_t maxLength);

/// Format a mark with at most two decimals, e.g. 17.5 -> "17.5", 20 -> "20".
std::string formatMark(double mark);

}  // namespace lms::text
