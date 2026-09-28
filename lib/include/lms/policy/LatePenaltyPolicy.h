#pragma once

#include <memory>
#include <string>

#include "lms/common/Clock.h"

namespace lms {

/// Outcome of applying a late-penalty rule to a raw mark.
struct PenaltyResult {
    double rawMark = 0.0;
    int daysLate = 0;
    double deduction = 0.0;   ///< marks removed (never more than rawMark)
    double finalMark = 0.0;   ///< rawMark - deduction, never below zero
    std::string explanation;  ///< shown to the lecturer and recorded in the audit reason
};

/// Strategy pattern: the late-penalty rule is an interchangeable object, so a school
/// can change its policy (or a course can waive penalties) without touching the
/// MarkingService (Open/Closed principle).
class ILatePenaltyPolicy {
public:
    virtual ~ILatePenaltyPolicy() = default;
    virtual PenaltyResult apply(double rawMark, int maxMark, int daysLate) const = 0;
    virtual std::string description() const = 0;
};

/// Head of Department's rule (BR-01/BR-02): lose `percentPerDay` % of the *available*
/// (maximum) mark for each day late, up to `maxPenalisedDays` days; later work scores zero.
class PercentPerDayPenaltyPolicy final : public ILatePenaltyPolicy {
public:
    explicit PercentPerDayPenaltyPolicy(double percentPerDay = 10.0, int maxPenalisedDays = 3);
    PenaltyResult apply(double rawMark, int maxMark, int daysLate) const override;
    std::string description() const override;

private:
    double percentPerDay_;
    int maxPenalisedDays_;
};

/// No deduction at all (e.g. for a course that has formally waived penalties).
class NoLatePenaltyPolicy final : public ILatePenaltyPolicy {
public:
    PenaltyResult apply(double rawMark, int maxMark, int daysLate) const override;
    std::string description() const override { return "No late penalty"; }
};

/// Computes how many (started) days late a submission is.
class LatenessCalculator {
public:
    /// 0 if submittedAt <= effectiveDeadline, otherwise the number of started 24-hour
    /// periods after the deadline: 1 second late -> 1 day, exactly 24h late -> 1 day,
    /// 24h + 1s late -> 2 days (BR-02).
    static int daysLate(Timestamp submittedAt, Timestamp effectiveDeadline);
};

}  // namespace lms
