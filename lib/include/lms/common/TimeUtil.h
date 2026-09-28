#pragma once

#include <optional>
#include <string>

#include "lms/common/Clock.h"

namespace lms::time_util {

/// Format a timestamp as local time "YYYY-MM-DD HH:MM".
std::string formatLocal(Timestamp t);

/// Parse "YYYY-MM-DD HH:MM" (local time). Returns std::nullopt when the text is not a
/// valid calendar date/time, e.g. "2026-02-30 10:00" or "2026-13-01 25:00".
std::optional<Timestamp> parseLocal(const std::string& text);

/// Human readable duration such as "2 days 3 hours" (used for "time remaining").
std::string describeDuration(std::int64_t seconds);

}  // namespace lms::time_util
