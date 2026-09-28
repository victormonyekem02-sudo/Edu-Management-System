#include "lms/common/TimeUtil.h"

#include <array>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace lms::time_util {

namespace {

std::tm toLocalTm(Timestamp t) {
    const auto tt = static_cast<std::time_t>(t);
    std::tm result{};
#ifdef _WIN32
    const bool ok = localtime_s(&result, &tt) == 0;
#else
    const bool ok = localtime_r(&tt, &result) != nullptr;
#endif
    if (!ok) {
        result = std::tm{};  // out-of-range timestamp: show 1900-01-00 rather than garbage
    }
    return result;
}

bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int daysInMonth(int year, int month) {
    static constexpr std::array<int, 12> kDays{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year)) {
        return 29;
    }
    return kDays.at(static_cast<std::size_t>(month - 1));
}

/// True if `text` has the exact shape "dddd-dd-dd dd:dd" (d = decimal digit).
bool matchesDateTimePattern(const std::string& text) {
    static const std::string kPattern = "dddd-dd-dd dd:dd";
    if (text.size() != kPattern.size()) {
        return false;
    }
    for (std::size_t i = 0; i < text.size(); ++i) {
        const bool wantDigit = kPattern.at(i) == 'd';
        const bool isDigit = text.at(i) >= '0' && text.at(i) <= '9';
        if (wantDigit != isDigit || (!wantDigit && text.at(i) != kPattern.at(i))) {
            return false;
        }
    }
    return true;
}

int digitsAt(const std::string& text, std::size_t pos, std::size_t len) {
    int value = 0;
    for (std::size_t i = pos; i < pos + len; ++i) {
        value = (value * 10) + (text.at(i) - '0');
    }
    return value;
}

bool inRange(int value, int low, int high) {
    return value >= low && value <= high;
}

}  // namespace

std::string formatLocal(Timestamp t) {
    const std::tm tm = toLocalTm(t);
    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%d %H:%M");
    return out.str();
}

std::optional<Timestamp> parseLocal(const std::string& text) {
    if (!matchesDateTimePattern(text)) {
        return std::nullopt;
    }
    const int year = digitsAt(text, 0, 4);
    const int month = digitsAt(text, 5, 2);
    const int day = digitsAt(text, 8, 2);
    const int hour = digitsAt(text, 11, 2);
    const int minute = digitsAt(text, 14, 2);
    if (!inRange(year, 1970, 2100) || !inRange(month, 1, 12) ||
        !inRange(day, 1, daysInMonth(year, month)) || !inRange(hour, 0, 23) ||
        !inRange(minute, 0, 59)) {
        return std::nullopt;
    }
    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;  // let the C library decide whether daylight saving applies
    const std::time_t result = std::mktime(&tm);
    if (result == static_cast<std::time_t>(-1)) {
        return std::nullopt;
    }
    return static_cast<Timestamp>(result);
}

std::string describeDuration(std::int64_t seconds) {
    if (seconds < 0) {
        seconds = -seconds;
    }
    const std::int64_t days = seconds / kSecondsPerDay;
    const std::int64_t hours = (seconds % kSecondsPerDay) / kSecondsPerHour;
    const std::int64_t minutes = (seconds % kSecondsPerHour) / kSecondsPerMinute;
    std::ostringstream out;
    if (days > 0) {
        out << days << (days == 1 ? " day " : " days ");
    }
    if (days > 0 || hours > 0) {
        out << hours << (hours == 1 ? " hour" : " hours");
    } else {
        out << minutes << (minutes == 1 ? " minute" : " minutes");
    }
    return out.str();
}

}  // namespace lms::time_util
