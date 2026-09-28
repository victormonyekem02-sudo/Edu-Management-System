#pragma once

#include <string>
#include <vector>

#include "lms/common/Clock.h"
#include "lms/data/Database.h"
#include "lms/data/Repositories.h"
#include "lms/domain/User.h"
#include "lms/events/SubmissionEvents.h"

namespace lms {

struct ModerationItem {
    Submission submission;
    Assignment assignment;
    Course course;
    std::string studentName;
    std::string markerName;
};

struct ReleaseSummary {
    int released = 0;
    int awaitingModeration = 0;  ///< MARKED, not yet approved - stay hidden from students
    int notYetMarked = 0;        ///< SUBMITTED or LATE
};

/// Head of Department use cases: moderate, release and inspect the audit trail
/// (FR-16, FR-17, FR-18).
class ModerationService {
public:
    ModerationService(Repositories repos, ITransactionRunner& tx, const IClock& clock,
                      SubmissionEventPublisher& events);

    /// All MARKED submissions in the HoD's department, oldest first.
    std::vector<ModerationItem> pendingModeration(const User& hod) const;

    /// All MODERATED submissions in the HoD's department that are ready to release.
    std::vector<ModerationItem> readyForRelease(const User& hod) const;

    /// Confirms the lecturer's mark: MARKED -> MODERATED.
    void approve(const User& hod, int submissionId, const std::string& note = "");

    /// Replaces the final mark (MARKED or MODERATED -> MODERATED). Reason is mandatory.
    void adjust(const User& hod, int submissionId, double newFinalMark,
                const std::string& reason);

    /// Releases every MODERATED submission of one assignment (MODERATED -> RELEASED).
    /// Submissions that are not yet moderated stay hidden (BR-07).
    ReleaseSummary releaseAssignment(const User& hod, int assignmentId);

    /// Audit trail of one submission (HoD of its department or an Administrator).
    std::vector<MarkAuditEntry> auditTrail(const User& actor, int submissionId) const;
    /// Audit trail of the actor's department (HoD) or of the whole school (Administrator).
    std::vector<MarkAuditEntry> auditLog(const User& actor) const;

private:
    struct Context {
        Submission submission;
        Assignment assignment;
        Course course;
    };
    Context loadInScope(const User& actor, int submissionId) const;
    std::vector<ModerationItem> itemsWithStatus(const User& hod, SubmissionStatus status) const;

    Repositories repos_;
    ITransactionRunner& tx_;
    const IClock& clock_;
    SubmissionEventPublisher& events_;
};

}  // namespace lms
