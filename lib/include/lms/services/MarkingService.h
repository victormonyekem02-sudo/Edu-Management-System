#pragma once

#include <string>
#include <vector>

#include "lms/common/Clock.h"
#include "lms/data/Database.h"
#include "lms/data/Repositories.h"
#include "lms/domain/User.h"
#include "lms/events/SubmissionEvents.h"
#include "lms/policy/LatePenaltyPolicy.h"
#include "lms/services/AssignmentService.h"

namespace lms {

/// One row of the lecturer's marking list.
struct MarkingItem {
    Submission submission;
    std::string studentName;
    std::string studentUsername;
    Timestamp effectiveDeadline = 0;
};

/// Lecturer marking (FR-14, FR-15). Late penalties come from the injected strategy.
class MarkingService {
public:
    static constexpr std::size_t kMaxFeedbackLength = 5000;

    MarkingService(Repositories repos, ITransactionRunner& tx, const IClock& clock,
                   SubmissionEventPublisher& events, const AssignmentService& assignments,
                   const ILatePenaltyPolicy& penaltyPolicy);

    /// Records a raw mark and written feedback. The late penalty is calculated from the
    /// submission time and the student's effective deadline *at marking time*, so an
    /// extension granted after submission is honoured. A re-mark requires a reason.
    /// Every call writes one mark_audit row in the same transaction.
    PenaltyResult mark(const User& actor, int submissionId, double rawMark,
                       const std::string& feedback, const std::string& reason = "");

    std::vector<MarkingItem> listForAssignment(const User& actor, int assignmentId) const;

    const ILatePenaltyPolicy& policy() const noexcept { return penaltyPolicy_; }

private:
    Repositories repos_;
    ITransactionRunner& tx_;
    const IClock& clock_;
    SubmissionEventPublisher& events_;
    const AssignmentService& assignments_;
    const ILatePenaltyPolicy& penaltyPolicy_;
};

}  // namespace lms
