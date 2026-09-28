#include "lms/common/TimeUtil.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace lms::time_util {

namespace {

std::tm toLocalTm(Timestamp t) {
    const std::time_t tt = static_cast<std::time_t>(t);
    std::tm result{};
#if defined(_WIN32)
    localtime_s(&result, &tt);
#else
    localtime_r(&tt, &result);
#endif
    return result;
}

bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int daysInMonth(int year, int month) {
    static const int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year)) {
        return 29;
    }
    return kDays[month - 1];
}

}  // namespace

std::string formatLocal(Timestamp t) {
    const std::tm tm = toLocalTm(t);
    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%d %H:%M");
    return out.str();
}

std::optional<Timestamp> parseLocal(const std::string& text) {
    // Exactly "YYYY-MM-DD HH:MM"; every other character must be a digit.
    static const char kPattern[] = "dddd-dd-dd dd:dd";
    if (text.size() != sizeof(kPattern) - 1) {
        return std::nullopt;
    }
    for (std::size_t i = 0; i < text.size(); ++i) {
        const bool wantDigit = kPattern[i] == 'd';
        const bool isDigit = text[i] >= '0' && text[i] <= '9';
        if (wantDigit != isDigit || (!wantDigit && text[i] != kPattern[i])) {
            return std::nullopt;
        }
    }
    const auto number = [&text](std::size_t pos, std::size_t len) {
        int value = 0;
        for (std::size_t i = pos; i < pos + len; ++i) {
            value = value * 10 + (text[i] - '0');
        }
        return value;
    };
    const int year = number(0, 4);
    const int month = number(5, 2);
    const int day = number(8, 2);
    const int hour = number(11, 2);
    const int minute = number(14, 2);
    if (year < 1970 || year > 2100 || month < 1 || month > 12 || day < 1 ||
        day > daysInMonth(year, month) || hour < 0 || hour > 23 || minute < 0 || minute > 59) {
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
