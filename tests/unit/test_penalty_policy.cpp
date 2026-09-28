// UT-PEN: late-penalty Strategy implementations (BR-01, BR-02).
#include <gtest/gtest.h>

#include <cmath>
#include <limits>

#include "lms/common/Errors.h"
#include "lms/policy/LatePenaltyPolicy.h"

using namespace lms;

namespace {

struct PenaltyCase {
    double raw;
    int maxMark;
    int daysLate;
    double expectedFinal;
    double expectedDeduction;
};

class TenPercentPolicy : public ::testing::TestWithParam<PenaltyCase> {
protected:
    PercentPerDayPenaltyPolicy policy{10.0, 3};
};

TEST_P(TenPercentPolicy, AppliesTenPercentOfMaximumPerDay) {
    const PenaltyCase& c = GetParam();
    const PenaltyResult r = policy.apply(c.raw, c.maxMark, c.daysLate);
    EXPECT_DOUBLE_EQ(r.finalMark, c.expectedFinal);
    EXPECT_DOUBLE_EQ(r.deduction, c.expectedDeduction);
    EXPECT_EQ(r.daysLate, c.daysLate);
    EXPECT_DOUBLE_EQ(r.rawMark, c.raw);
    EXPECT_FALSE(r.explanation.empty());
}

INSTANTIATE_TEST_SUITE_P(
    UT_PEN, TenPercentPolicy,
    ::testing::Values(
        PenaltyCase{20, 25, 0, 20, 0},       // on time: unchanged
        PenaltyCase{20, 25, 1, 17.5, 2.5},   // 1 day: -10% of 25
        PenaltyCase{20, 25, 2, 15, 5},       // 2 days: -20%
        PenaltyCase{20, 25, 3, 12.5, 7.5},   // 3 days: -30% (last penalised day)
        PenaltyCase{20, 25, 4, 0, 20},       // 4 days: zero
        PenaltyCase{25, 25, 10, 0, 25},      // far too late: zero
        PenaltyCase{25, 25, 1, 22.5, 2.5},   // raw = max (upper boundary)
        PenaltyCase{0, 25, 1, 0, 0},         // raw = 0 (lower boundary): never negative
        PenaltyCase{2, 25, 3, 0, 2},         // deduction capped at the raw mark
        PenaltyCase{73, 100, 2, 53, 20},
        PenaltyCase{1, 1, 1, 0.9, 0.1}));    // smallest maximum mark

TEST(UT_PEN, RejectsMarkAboveMaximum) {
    PercentPerDayPenaltyPolicy policy;
    EXPECT_THROW(policy.apply(25.01, 25, 0), ValidationError);
}

TEST(UT_PEN, RejectsNegativeMark) {
    PercentPerDayPenaltyPolicy policy;
    EXPECT_THROW(policy.apply(-0.5, 25, 0), ValidationError);
}

TEST(UT_PEN, RejectsNaNMark) {
    PercentPerDayPenaltyPolicy policy;
    EXPECT_THROW(policy.apply(std::numeric_limits<double>::quiet_NaN(), 25, 0), ValidationError);
}

TEST(UT_PEN, RejectsNegativeDaysAndNonPositiveMaximum) {
    PercentPerDayPenaltyPolicy policy;
    EXPECT_THROW(policy.apply(10, 25, -1), ValidationError);
    EXPECT_THROW(policy.apply(0, 0, 0), ValidationError);
}

TEST(UT_PEN, RejectsInvalidConfiguration) {
    EXPECT_THROW(PercentPerDayPenaltyPolicy(-1.0, 3), ValidationError);
    EXPECT_THROW(PercentPerDayPenaltyPolicy(101.0, 3), ValidationError);
    EXPECT_THROW(PercentPerDayPenaltyPolicy(10.0, -1), ValidationError);
}

TEST(UT_PEN, ConfigurableRateAndCap) {
    PercentPerDayPenaltyPolicy fivePercentTwoDays(5.0, 2);
    EXPECT_DOUBLE_EQ(fivePercentTwoDays.apply(40, 40, 2).finalMark, 36.0);
    EXPECT_DOUBLE_EQ(fivePercentTwoDays.apply(40, 40, 3).finalMark, 0.0);
}

TEST(UT_PEN, DescriptionStatesTheRule) {
    PercentPerDayPenaltyPolicy policy;
    EXPECT_EQ(policy.description(),
              "10% of the maximum mark per day late, up to 3 days; zero after that");
}

TEST(UT_PEN, NoPenaltyPolicyKeepsRawMark) {
    NoLatePenaltyPolicy policy;
    const PenaltyResult r = policy.apply(18, 25, 9);
    EXPECT_DOUBLE_EQ(r.finalMark, 18);
    EXPECT_DOUBLE_EQ(r.deduction, 0);
    EXPECT_EQ(policy.description(), "No late penalty");
    EXPECT_THROW(policy.apply(26, 25, 0), ValidationError);
}

TEST(UT_PEN, PolicyIsSubstitutableThroughInterface) {
    // Liskov substitution: callers only see ILatePenaltyPolicy.
    PercentPerDayPenaltyPolicy tenPercent;
    NoLatePenaltyPolicy none;
    const ILatePenaltyPolicy* policies[] = {&tenPercent, &none};
    for (const ILatePenaltyPolicy* p : policies) {
        const PenaltyResult r = p->apply(10, 25, 0);
        EXPECT_DOUBLE_EQ(r.finalMark, 10);
    }
}

}  // namespace
