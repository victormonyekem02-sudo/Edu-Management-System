// UT-TXT: text and date/time helpers used for input validation (NFR-08).
#include <gtest/gtest.h>

#include "lms/common/Errors.h"
#include "lms/common/TextUtil.h"
#include "lms/common/TimeUtil.h"

using namespace lms;

namespace {

TEST(UT_TXT, TrimRemovesSurroundingWhitespace) {
    EXPECT_EQ(text::trim("  abc \t\r\n"), "abc");
    EXPECT_EQ(text::trim("a b"), "a b");
    EXPECT_EQ(text::trim("   "), "");
    EXPECT_EQ(text::trim(""), "");
}

TEST(UT_TXT, RequireTextValidatesPresenceAndLength) {
    EXPECT_EQ(text::requireText("  Essay ", "Title", 10), "Essay");
    EXPECT_THROW(text::requireText("   ", "Title", 10), ValidationError);
    EXPECT_NO_THROW(text::requireText(std::string(10, 'x'), "Title", 10));
    EXPECT_THROW(text::requireText(std::string(11, 'x'), "Title", 10), ValidationError);
}

TEST(UT_TXT, FormatMarkDropsTrailingZeros) {
    EXPECT_EQ(text::formatMark(17.5), "17.5");
    EXPECT_EQ(text::formatMark(20.0), "20");
    EXPECT_EQ(text::formatMark(0.0), "0");
    EXPECT_EQ(text::formatMark(12.345), "12.35");
    EXPECT_EQ(text::formatMark(0.1), "0.1");
}

TEST(UT_TXT, ParseLocalAcceptsValidDates) {
    const auto t = time_util::parseLocal("2026-10-15 17:00");
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(time_util::formatLocal(*t), "2026-10-15 17:00");
    EXPECT_TRUE(time_util::parseLocal("2028-02-29 00:00").has_value());  // leap year
    EXPECT_TRUE(time_util::parseLocal("2026-12-31 23:59").has_value());
}

TEST(UT_TXT, ParseLocalRejectsInvalidDates) {
    for (const char* bad :
         {"", "tomorrow", "2026-02-29 10:00", "2026-02-30 10:00", "2026-13-01 10:00",
          "2026-00-10 10:00", "2026-04-31 10:00", "2026-10-15 24:00", "2026-10-15 23:60",
          "2026-10-15", "2026-10-15 17:00x", "15/10/2026 17:00", "1969-12-31 23:59"}) {
        EXPECT_FALSE(time_util::parseLocal(bad).has_value()) << bad;
    }
}

TEST(UT_TXT, DescribeDuration) {
    EXPECT_EQ(time_util::describeDuration(0), "0 minutes");
    EXPECT_EQ(time_util::describeDuration(60), "1 minute");
    EXPECT_EQ(time_util::describeDuration(3 * kSecondsPerHour), "3 hours");
    EXPECT_EQ(time_util::describeDuration(kSecondsPerDay + kSecondsPerHour), "1 day 1 hour");
    EXPECT_EQ(time_util::describeDuration(-2 * kSecondsPerDay), "2 days 0 hours");
}

}  // namespace
