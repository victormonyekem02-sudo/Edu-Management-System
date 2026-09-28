// UT-LAT: LatenessCalculator - equivalence partitions and boundary values (test plan 5.1).
#include <gtest/gtest.h>

#include <cctype>
#include <string>

#include "lms/policy/LatePenaltyPolicy.h"

using lms::kSecondsPerDay;
using lms::kSecondsPerHour;
using lms::LatenessCalculator;

namespace {

constexpr lms::Timestamp kDeadline = 1790000000;

struct LatenessCase {
    const char* name;
    std::int64_t offsetSeconds;  // submission time relative to deadline
    int expectedDays;
};

class LatenessBoundaries : public ::testing::TestWithParam<LatenessCase> {};

TEST_P(LatenessBoundaries, DaysLateMatchesSpecification) {
    const LatenessCase& c = GetParam();
    EXPECT_EQ(LatenessCalculator::daysLate(kDeadline + c.offsetSeconds, kDeadline),
              c.expectedDays)
        << c.name;
}

INSTANTIATE_TEST_SUITE_P(
    UT_LAT, LatenessBoundaries,
    ::testing::Values(
        // EP1: on time (submitted at or before the deadline)
        LatenessCase{"one week early", -7 * kSecondsPerDay, 0},
        LatenessCase{"one second early", -1, 0},
        LatenessCase{"exactly at deadline", 0, 0},
        // EP2: late by up to one day
        LatenessCase{"one second late", 1, 1},
        LatenessCase{"one hour late", kSecondsPerHour, 1},
        LatenessCase{"exactly 24h late", kSecondsPerDay, 1},
        // EP3: second and third day
        LatenessCase{"24h + 1s late", kSecondsPerDay + 1, 2},
        LatenessCase{"48h late", 2 * kSecondsPerDay, 2},
        LatenessCase{"48h + 1s late", 2 * kSecondsPerDay + 1, 3},
        LatenessCase{"exactly 72h late", 3 * kSecondsPerDay, 3},
        // EP4: more than three days late
        LatenessCase{"72h + 1s late", 3 * kSecondsPerDay + 1, 4},
        LatenessCase{"30 days late", 30 * kSecondsPerDay, 30}),
    [](const ::testing::TestParamInfo<LatenessCase>& paramInfo) {
        std::string name = paramInfo.param.name;
        for (char& ch : name) {
            if (!std::isalnum(static_cast<unsigned char>(ch))) {
                ch = '_';
            }
        }
        return name;
    });

}  // namespace
