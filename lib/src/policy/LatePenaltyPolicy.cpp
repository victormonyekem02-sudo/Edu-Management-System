#include "lms/policy/LatePenaltyPolicy.h"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "lms/common/Errors.h"
#include "lms/common/TextUtil.h"

namespace lms {

namespace {

double roundToHundredths(double value) {
    return std::round(value * 100.0) / 100.0;
}

void validateInputs(double rawMark, int maxMark, int daysLate) {
    if (maxMark <= 0) {
        throw ValidationError("Maximum mark must be positive.");
    }
    if (rawMark < 0.0 || rawMark > maxMark) {
        throw ValidationError("Mark must be between 0 and " + std::to_string(maxMark) + ".");
    }
    if (daysLate < 0) {
        throw ValidationError("Days late cannot be negative.");
    }
}

}  // namespace

PercentPerDayPenaltyPolicy::PercentPerDayPenaltyPolicy(double percentPerDay, int maxPenalisedDays)
    : percentPerDay_(percentPerDay), maxPenalisedDays_(maxPenalisedDays) {
    if (percentPerDay_ < 0.0 || percentPerDay_ > 100.0 || maxPenalisedDays_ < 0) {
        throw ValidationError("Invalid late-penalty configuration.");
    }
}

PenaltyResult PercentPerDayPenaltyPolicy::apply(double rawMark, int maxMark, int daysLate) const {
    validateInputs(rawMark, maxMark, daysLate);
    PenaltyResult result;
    result.rawMark = rawMark;
    result.daysLate = daysLate;

    if (daysLate == 0) {
        result.finalMark = rawMark;
        result.explanation = "On time - no penalty";
        return result;
    }
    if (daysLate > maxPenalisedDays_) {
        result.deduction = rawMark;
        result.finalMark = 0.0;
        result.explanation = std::to_string(daysLate) + " days late (more than " +
                             std::to_string(maxPenalisedDays_) + ") - mark is zero";
        return result;
    }
    const double penalty = roundToHundredths(maxMark * percentPerDay_ / 100.0 * daysLate);
    result.deduction = std::min(penalty, rawMark);
    result.finalMark = roundToHundredths(rawMark - result.deduction);
    std::ostringstream why;
    why << daysLate << (daysLate == 1 ? " day" : " days") << " late - "
        << text::formatMark(percentPerDay_ * daysLate) << "% of " << maxMark << " = "
        << text::formatMark(penalty) << " deducted";
    result.explanation = why.str();
    return result;
}

std::string PercentPerDayPenaltyPolicy::description() const {
    return text::formatMark(percentPerDay_) + "% of the maximum mark per day late, up to " +
           std::to_string(maxPenalisedDays_) + " days; zero after that";
}

PenaltyResult NoLatePenaltyPolicy::apply(double rawMark, int maxMark, int daysLate) const {
    validateInputs(rawMark, maxMark, daysLate);
    PenaltyResult result;
    result.rawMark = rawMark;
    result.daysLate = daysLate;
    result.finalMark = rawMark;
    result.explanation = "No late penalty applies";
    return result;
}

int LatenessCalculator::daysLate(Timestamp submittedAt, Timestamp effectiveDeadline) {
    if (submittedAt <= effectiveDeadline) {
        return 0;
    }
    const std::int64_t secondsLate = submittedAt - effectiveDeadline;
    // ceil(secondsLate / day) without floating point
    return static_cast<int>((secondsLate + kSecondsPerDay - 1) / kSecondsPerDay);
}

}  // namespace lms
