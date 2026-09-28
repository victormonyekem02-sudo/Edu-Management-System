#include "lms/services/ModerationService.h"

#include "lms/common/Errors.h"
#include "lms/common/TextUtil.h"

namespace lms {

ModerationService::ModerationService(Repositories repos, ITransactionRunner& tx,
                                     const IClock& clock, SubmissionEventPublisher& events)
    : repos_(repos), tx_(tx), clock_(clock), events_(events) {}

ModerationService::Context ModerationService::loadInScope(const User& actor,
                                                          int submissionId) const {
    std::optional<Submission> s = repos_.submissions.findById(submissionId);
    if (!s) {
        throw NotFoundError("Submission " + std::to_string(submissionId) + " does not exist.");
    }
    std::optional<Assignment> a = repos_.assignments.findById(s->assignmentId);
    std::optional<Course> c = a ? repos_.courses.findById(a->courseId) : std::nullopt;
    if (!a || !c) {
        throw NotFoundError("The assignment for this submission no longer exists.");
    }
    if (!actor.hasDepartmentScope(c->departmentId)) {
        throw AuthorizationError("This submission belongs to another department.");
    }
    return Context{*s, *a, *c};
}

std::vector<ModerationItem> ModerationService::itemsWithStatus(const User& hod,
                                                               SubmissionStatus status) const {
    hod.require(Permission::ModerateMarks);
    std::vector<ModerationItem> items;
    if (!hod.departmentId()) {
        return items;
    }
    for (const Submission& s :
         repos_.submissions.listByDepartmentAndStatus(*hod.departmentId(), status)) {
        ModerationItem item;
        item.submission = s;
        if (std::optional<Assignment> a = repos_.assignments.findById(s.assignmentId)) {
            item.assignment = *a;
            if (std::optional<Course> c = repos_.courses.findById(a->courseId)) {
                item.course = *c;
            }
        }
        if (std::optional<UserRecord> st = repos_.users.findById(s.studentId)) {
            item.studentName = st->fullName;
        }
        if (s.markedBy) {
            if (std::optional<UserRecord> m = repos_.users.findById(*s.markedBy)) {
                item.markerName = m->fullName;
            }
        }
        items.push_back(item);
    }
    return items;
}

std::vector<ModerationItem> ModerationService::pendingModeration(const User& hod) const {
    return itemsWithStatus(hod, SubmissionStatus::Marked);
}

std::vector<ModerationItem> ModerationService::readyForRelease(const User& hod) const {
    return itemsWithStatus(hod, SubmissionStatus::Moderated);
}

void ModerationService::approve(const User& hod, int submissionId, const std::string& note) {
    hod.require(Permission::ModerateMarks);
    Context ctx = loadInScope(hod, submissionId);
    if (ctx.submission.status != SubmissionStatus::Marked) {
        throw BusinessRuleError("Only a marked submission that awaits moderation can be approved.");
    }
    SubmissionLifecycle::requireTransition(ctx.submission.status, SubmissionStatus::Moderated);
    const Timestamp now = clock_.now();
    ctx.submission.status = SubmissionStatus::Moderated;
    ctx.submission.moderatedBy = hod.id();
    ctx.submission.moderatedAt = now;

    MarkAuditEntry audit;
    audit.submissionId = submissionId;
    audit.changedBy = hod.id();
    audit.changedAt = now;
    audit.action = AuditAction::ModerationApprove;
    audit.oldMark = ctx.submission.finalMark;
    audit.newMark = ctx.submission.finalMark;
    const std::string cleanNote = text::trim(note);
    audit.reason = cleanNote.empty() ? "Mark approved at moderation"
                                     : "Mark approved at moderation: " + cleanNote;
    tx_.run([&] {
        repos_.submissions.update(ctx.submission);
        repos_.audit.add(audit);
    });

    SubmissionEvent event;
    event.type = SubmissionEventType::Moderated;
    event.assignmentId = ctx.assignment.id;
    event.studentId = ctx.submission.studentId;
    event.actorId = hod.id();
    event.submissionId = submissionId;
    events_.publish(event);
}

void ModerationService::adjust(const User& hod, int submissionId, double newFinalMark,
                               const std::string& reason) {
    hod.require(Permission::ModerateMarks);
    Context ctx = loadInScope(hod, submissionId);
    if (ctx.submission.status != SubmissionStatus::Marked &&
        ctx.submission.status != SubmissionStatus::Moderated) {
        throw BusinessRuleError("Only marked or moderated (unreleased) submissions can be adjusted.");
    }
    // NOLINTNEXTLINE(readability-simplify-boolean-expr): written this way to reject NaN
    if (!(newFinalMark >= 0.0 && newFinalMark <= ctx.assignment.maxMark)) {
        throw ValidationError("Mark must be between 0 and " +
                              std::to_string(ctx.assignment.maxMark) + ".");
    }
    const std::string why = text::requireText(reason, "Reason for the adjustment", 500);
    SubmissionLifecycle::requireTransition(ctx.submission.status, SubmissionStatus::Moderated);

    const Timestamp now = clock_.now();
    MarkAuditEntry audit;
    audit.submissionId = submissionId;
    audit.changedBy = hod.id();
    audit.changedAt = now;
    audit.action = AuditAction::ModerationAdjust;
    audit.oldMark = ctx.submission.finalMark;
    audit.newMark = newFinalMark;
    audit.reason = why;

    ctx.submission.finalMark = newFinalMark;
    ctx.submission.status = SubmissionStatus::Moderated;
    ctx.submission.moderatedBy = hod.id();
    ctx.submission.moderatedAt = now;
    tx_.run([&] {
        repos_.submissions.update(ctx.submission);
        repos_.audit.add(audit);
    });

    SubmissionEvent event;
    event.type = SubmissionEventType::Moderated;
    event.assignmentId = ctx.assignment.id;
    event.studentId = ctx.submission.studentId;
    event.actorId = hod.id();
    event.submissionId = submissionId;
    events_.publish(event);
}

ReleaseSummary ModerationService::releaseAssignment(const User& hod, int assignmentId) {
    hod.require(Permission::ReleaseMarks);
    std::optional<Assignment> a = repos_.assignments.findById(assignmentId);
    if (!a) {
        throw NotFoundError("Assignment " + std::to_string(assignmentId) + " does not exist.");
    }
    std::optional<Course> c = repos_.courses.findById(a->courseId);
    if (!c || !hod.hasDepartmentScope(c->departmentId)) {
        throw AuthorizationError("This assignment belongs to another department.");
    }

    ReleaseSummary summary;
    std::vector<Submission> toRelease;
    for (const Submission& s : repos_.submissions.listByAssignment(assignmentId)) {
        switch (s.status) {
            case SubmissionStatus::Moderated:
                toRelease.push_back(s);
                break;
            case SubmissionStatus::Marked:
                ++summary.awaitingModeration;
                break;
            case SubmissionStatus::Submitted:
            case SubmissionStatus::Late:
                ++summary.notYetMarked;
                break;
            case SubmissionStatus::Released:
                break;
        }
    }

    const Timestamp now = clock_.now();
    tx_.run([&] {
        for (Submission& s : toRelease) {
            SubmissionLifecycle::requireTransition(s.status, SubmissionStatus::Released);
            s.status = SubmissionStatus::Released;
            s.releasedBy = hod.id();
            s.releasedAt = now;
            repos_.submissions.update(s);

            MarkAuditEntry audit;
            audit.submissionId = s.id;
            audit.changedBy = hod.id();
            audit.changedAt = now;
            audit.action = AuditAction::Release;
            audit.oldMark = s.finalMark;
            audit.newMark = s.finalMark;
            audit.reason = "Moderated mark released to student";
            repos_.audit.add(audit);
        }
    });
    summary.released = static_cast<int>(toRelease.size());

    for (const Submission& s : toRelease) {
        SubmissionEvent event;
        event.type = SubmissionEventType::Released;
        event.assignmentId = assignmentId;
        event.studentId = s.studentId;
        event.actorId = hod.id();
        event.submissionId = s.id;
        events_.publish(event);
    }
    return summary;
}

std::vector<MarkAuditEntry> ModerationService::auditTrail(const User& actor,
                                                          int submissionId) const {
    actor.require(Permission::ViewAuditLog);
    loadInScope(actor, submissionId);
    return repos_.audit.listForSubmission(submissionId);
}

std::vector<MarkAuditEntry> ModerationService::auditLog(const User& actor) const {
    actor.require(Permission::ViewAuditLog);
    if (actor.role() == Role::Administrator) {
        return repos_.audit.listAll();
    }
    if (!actor.departmentId()) {
        return {};
    }
    return repos_.audit.listForDepartment(*actor.departmentId());
}

}  // namespace lms
