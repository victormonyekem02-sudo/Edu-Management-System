#include "lms/services/MarkingService.h"

#include "lms/common/Errors.h"
#include "lms/common/TextUtil.h"

namespace lms {

MarkingService::MarkingService(Repositories repos, ITransactionRunner& tx, const IClock& clock,
                               SubmissionEventPublisher& events,
                               const AssignmentService& assignments,
                               const ILatePenaltyPolicy& penaltyPolicy)
    : repos_(repos),
      tx_(tx),
      clock_(clock),
      events_(events),
      assignments_(assignments),
      penaltyPolicy_(penaltyPolicy) {}

PenaltyResult MarkingService::mark(const User& actor, int submissionId, double rawMark,
                                   const std::string& feedback, const std::string& reason) {
    actor.require(Permission::MarkSubmission);
    std::optional<Submission> submission = repos_.submissions.findById(submissionId);
    if (!submission) {
        throw NotFoundError("Submission " + std::to_string(submissionId) + " does not exist.");
    }
    const Assignment assignment = assignments_.requireOwnAssignment(actor, submission->assignmentId);

    if (!SubmissionLifecycle::isMarkable(submission->status)) {
        throw BusinessRuleError(
            "This submission has been moderated by the Head of Department and can no longer be "
            "changed by the lecturer.");
    }
    if (!(rawMark >= 0.0 && rawMark <= assignment.maxMark)) {  // also rejects NaN
        throw ValidationError("Mark must be between 0 and " + std::to_string(assignment.maxMark) +
                              ".");
    }
    const std::string writtenFeedback =
        text::requireText(feedback, "Written feedback", kMaxFeedbackLength);
    const bool isRemark = submission->status == SubmissionStatus::Marked;
    std::string why = text::trim(reason);
    if (isRemark && why.empty()) {
        throw ValidationError("A reason is required when changing an existing mark.");
    }
    if (why.empty()) {
        why = "Initial marking";
    }

    const Timestamp deadline = assignments_.effectiveDeadline(assignment, submission->studentId);
    const int daysLate = LatenessCalculator::daysLate(submission->submittedAt, deadline);
    const PenaltyResult penalty = penaltyPolicy_.apply(rawMark, assignment.maxMark, daysLate);

    SubmissionLifecycle::requireTransition(submission->status, SubmissionStatus::Marked);
    const std::optional<double> oldMark = submission->finalMark;
    const Timestamp now = clock_.now();
    submission->status = SubmissionStatus::Marked;
    submission->rawMark = rawMark;
    submission->daysLate = daysLate;
    submission->lateDeduction = penalty.deduction;
    submission->finalMark = penalty.finalMark;
    submission->feedback = writtenFeedback;
    submission->markedBy = actor.id();
    submission->markedAt = now;

    MarkAuditEntry audit;
    audit.submissionId = submission->id;
    audit.changedBy = actor.id();
    audit.changedAt = now;
    audit.action = isRemark ? AuditAction::Remark : AuditAction::Mark;
    audit.oldMark = oldMark;
    audit.newMark = penalty.finalMark;
    audit.reason = why + " (raw " + text::formatMark(rawMark) + "/" +
                   std::to_string(assignment.maxMark) + "; " + penalty.explanation + ")";

    tx_.run([&] {
        repos_.submissions.update(*submission);
        repos_.audit.add(audit);
    });

    SubmissionEvent event;
    event.type = SubmissionEventType::Marked;
    event.assignmentId = assignment.id;
    event.studentId = submission->studentId;
    event.actorId = actor.id();
    event.submissionId = submission->id;
    events_.publish(event);
    return penalty;
}

std::vector<MarkingItem> MarkingService::listForAssignment(const User& actor,
                                                           int assignmentId) const {
    actor.require(Permission::MarkSubmission);
    const Assignment assignment = assignments_.requireOwnAssignment(actor, assignmentId);
    std::vector<MarkingItem> items;
    for (const Submission& s : repos_.submissions.listByAssignment(assignmentId)) {
        MarkingItem item;
        item.submission = s;
        if (std::optional<UserRecord> u = repos_.users.findById(s.studentId)) {
            item.studentName = u->fullName;
            item.studentUsername = u->username;
        }
        item.effectiveDeadline = assignments_.effectiveDeadline(assignment, s.studentId);
        items.push_back(item);
    }
    return items;
}

}  // namespace lms
