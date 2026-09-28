#include "lms/domain/SubmissionStatus.h"

#include "lms/common/Errors.h"
#include "lms/domain/Entities.h"

namespace lms {

std::string statusToCode(SubmissionStatus s) {
    switch (s) {
        case SubmissionStatus::Submitted:
            return "SUBMITTED";
        case SubmissionStatus::Late:
            return "LATE";
        case SubmissionStatus::Marked:
            return "MARKED";
        case SubmissionStatus::Moderated:
            return "MODERATED";
        case SubmissionStatus::Released:
            return "RELEASED";
    }
    throw ValidationError("Unknown submission status");
}

SubmissionStatus statusFromCode(const std::string& code) {
    if (code == "SUBMITTED") {
        return SubmissionStatus::Submitted;
    }
    if (code == "LATE") {
        return SubmissionStatus::Late;
    }
    if (code == "MARKED") {
        return SubmissionStatus::Marked;
    }
    if (code == "MODERATED") {
        return SubmissionStatus::Moderated;
    }
    if (code == "RELEASED") {
        return SubmissionStatus::Released;
    }
    throw ValidationError("Unknown submission status code: " + code);
}

std::string statusDisplayName(SubmissionStatus s) {
    switch (s) {
        case SubmissionStatus::Submitted:
            return "Submitted";
        case SubmissionStatus::Late:
            return "Submitted late";
        case SubmissionStatus::Marked:
            return "Marked (awaiting moderation)";
        case SubmissionStatus::Moderated:
            return "Moderated (awaiting release)";
        case SubmissionStatus::Released:
            return "Released";
    }
    return "Unknown";
}

bool SubmissionLifecycle::canTransition(SubmissionStatus from, SubmissionStatus to) {
    using S = SubmissionStatus;
    switch (from) {
        case S::Submitted:  // resubmission keeps SUBMITTED; marking moves on
        case S::Late:       // an extension may retrospectively cover the submission
            return to == S::Submitted || to == S::Marked;
        case S::Marked:
            // re-mark, or HoD approves/adjusts
            return to == S::Marked || to == S::Moderated;
        case S::Moderated:
            // HoD may adjust again before release
            return to == S::Moderated || to == S::Released;
        case S::Released:
            return false;  // terminal: released marks are final
    }
    return false;
}

void SubmissionLifecycle::requireTransition(SubmissionStatus from, SubmissionStatus to) {
    if (!canTransition(from, to)) {
        throw BusinessRuleError("A submission that is '" + statusDisplayName(from) +
                                "' cannot become '" + statusDisplayName(to) + "'.");
    }
}

bool SubmissionLifecycle::isMarkable(SubmissionStatus s) {
    return s == SubmissionStatus::Submitted || s == SubmissionStatus::Late ||
           s == SubmissionStatus::Marked;
}

std::string auditActionToCode(AuditAction a) {
    switch (a) {
        case AuditAction::Mark:
            return "MARK";
        case AuditAction::Remark:
            return "REMARK";
        case AuditAction::ModerationApprove:
            return "MODERATION_APPROVE";
        case AuditAction::ModerationAdjust:
            return "MODERATION_ADJUST";
        case AuditAction::Release:
            return "RELEASE";
    }
    throw ValidationError("Unknown audit action");
}

AuditAction auditActionFromCode(const std::string& code) {
    if (code == "MARK") {
        return AuditAction::Mark;
    }
    if (code == "REMARK") {
        return AuditAction::Remark;
    }
    if (code == "MODERATION_APPROVE") {
        return AuditAction::ModerationApprove;
    }
    if (code == "MODERATION_ADJUST") {
        return AuditAction::ModerationAdjust;
    }
    if (code == "RELEASE") {
        return AuditAction::Release;
    }
    throw ValidationError("Unknown audit action code: " + code);
}

}  // namespace lms
