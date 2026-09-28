#include "lms/services/SubmissionService.h"

#include "lms/common/Errors.h"
#include "lms/common/TextUtil.h"

namespace lms {

SubmissionService::SubmissionService(Repositories repos, ITransactionRunner& tx,
                                     const IClock& clock, SubmissionEventPublisher& events,
                                     const AssignmentService& assignments)
    : repos_(repos), tx_(tx), clock_(clock), events_(events), assignments_(assignments) {}

SubmitResult SubmissionService::submit(const User& actor, int assignmentId,
                                       const std::string& content) {
    actor.require(Permission::SubmitWork);
    std::optional<Assignment> assignment = repos_.assignments.findById(assignmentId);
    if (!assignment) {
        throw NotFoundError("Assignment " + std::to_string(assignmentId) + " does not exist.");
    }
    if (!repos_.courses.isEnrolled(assignment->courseId, actor.id())) {
        throw AuthorizationError("You are not enrolled on the course of this assignment.");
    }
    const std::string work = text::requireText(content, "Submission", kMaxContentLength);

    const Timestamp now = clock_.now();
    const Timestamp deadline = assignments_.effectiveDeadline(*assignment, actor.id());
    const bool late = now > deadline;

    SubmitResult result;
    result.effectiveDeadline = deadline;
    result.late = late;

    std::optional<Submission> existing = repos_.submissions.find(assignmentId, actor.id());
    if (existing) {
        if (existing->status != SubmissionStatus::Submitted &&
            existing->status != SubmissionStatus::Late) {
            throw BusinessRuleError("This work has already been marked and cannot be resubmitted.");
        }
        if (late) {
            throw BusinessRuleError(
                "The deadline has passed. Resubmission is only allowed before the deadline; "
                "your previous submission has been kept.");
        }
        SubmissionLifecycle::requireTransition(existing->status, SubmissionStatus::Submitted);
        existing->content = work;
        existing->submittedAt = now;
        existing->version += 1;
        existing->status = SubmissionStatus::Submitted;
        tx_.run([&] {
            repos_.submissions.update(*existing);
            repos_.submissions.addVersion(existing->id, existing->version, work, now);
        });
        result.submissionId = existing->id;
        result.version = existing->version;
        result.status = existing->status;
        result.resubmission = true;
    } else {
        Submission s;
        s.assignmentId = assignmentId;
        s.studentId = actor.id();
        s.content = work;
        s.submittedAt = now;
        s.version = 1;
        s.status = late ? SubmissionStatus::Late : SubmissionStatus::Submitted;
        tx_.run([&] {
            s.id = repos_.submissions.add(s);
            repos_.submissions.addVersion(s.id, 1, work, now);
        });
        result.submissionId = s.id;
        result.version = 1;
        result.status = s.status;
    }

    SubmissionEvent event;
    event.type = result.resubmission ? SubmissionEventType::Resubmitted
                                     : SubmissionEventType::Submitted;
    event.assignmentId = assignmentId;
    event.studentId = actor.id();
    event.actorId = actor.id();
    event.submissionId = result.submissionId;
    event.late = late;
    events_.publish(event);
    return result;
}

std::optional<Submission> SubmissionService::findOwn(const User& student, int assignmentId) const {
    student.require(Permission::SubmitWork);
    return repos_.submissions.find(assignmentId, student.id());
}

std::vector<StudentResultView> SubmissionService::resultsFor(const User& student) const {
    student.require(Permission::ViewReleasedResults);
    std::vector<StudentResultView> results;
    for (const Submission& s : repos_.submissions.listForStudent(student.id())) {
        std::optional<Assignment> a = repos_.assignments.findById(s.assignmentId);
        if (!a) {
            continue;
        }
        StudentResultView v;
        v.assignment = *a;
        if (std::optional<Course> c = repos_.courses.findById(a->courseId)) {
            v.course = *c;
        }
        v.status = s.status;
        v.released = SubmissionLifecycle::isVisibleToStudent(s.status);
        if (v.released) {
            // Principal's rule: nothing about the mark is disclosed before release.
            v.finalMark = s.finalMark;
            v.rawMark = s.rawMark;
            v.daysLate = s.daysLate;
            v.lateDeduction = s.lateDeduction;
            v.feedback = s.feedback;
        }
        results.push_back(v);
    }
    return results;
}

}  // namespace lms
