#include "lms/services/AssignmentService.h"

#include <algorithm>

#include "lms/common/Errors.h"
#include "lms/common/TextUtil.h"

namespace lms {

AssignmentService::AssignmentService(Repositories repos, ITransactionRunner& tx,
                                     const IClock& clock, SubmissionEventPublisher& events)
    : repos_(repos), tx_(tx), clock_(clock), events_(events) {}

int AssignmentService::createAssignment(const User& actor, int courseId, const std::string& title,
                                        const std::string& description, Timestamp deadline,
                                        int maxMark) {
    actor.require(Permission::CreateAssignment);
    if (!repos_.courses.findById(courseId)) {
        throw NotFoundError("Course " + std::to_string(courseId) + " does not exist.");
    }
    if (!repos_.courses.isLecturerOf(courseId, actor.id())) {
        throw AuthorizationError("You can only create assignments for courses you teach.");
    }
    Assignment a;
    a.courseId = courseId;
    a.title = text::requireText(title, "Title", kMaxTitleLength);
    a.description = text::trim(description);
    if (a.description.size() > kMaxDescriptionLength) {
        throw ValidationError("Description must be at most " +
                              std::to_string(kMaxDescriptionLength) + " characters.");
    }
    if (deadline <= clock_.now()) {
        throw ValidationError("The deadline must be in the future.");
    }
    if (maxMark < 1 || maxMark > 1000) {
        throw ValidationError("Maximum mark must be between 1 and 1000.");
    }
    a.deadline = deadline;
    a.maxMark = maxMark;
    a.createdBy = actor.id();
    a.createdAt = clock_.now();
    return repos_.assignments.add(a);
}

Assignment AssignmentService::requireOwnAssignment(const User& actor, int assignmentId) const {
    std::optional<Assignment> a = repos_.assignments.findById(assignmentId);
    if (!a) {
        throw NotFoundError("Assignment " + std::to_string(assignmentId) + " does not exist.");
    }
    if (!repos_.courses.isLecturerOf(a->courseId, actor.id())) {
        throw AuthorizationError("You do not teach the course of this assignment.");
    }
    return *a;
}

void AssignmentService::grantExtension(const User& actor, int assignmentId, int studentId,
                                       Timestamp newDeadline, const std::string& reason) {
    actor.require(Permission::GrantExtension);
    const Assignment assignment = requireOwnAssignment(actor, assignmentId);
    if (!repos_.courses.isEnrolled(assignment.courseId, studentId)) {
        throw BusinessRuleError("That student is not enrolled on this course.");
    }
    if (newDeadline <= assignment.deadline) {
        throw ValidationError("An extension must be later than the original deadline.");
    }
    const std::string why = text::requireText(reason, "Reason", kMaxReasonLength);

    std::optional<Submission> existing = repos_.submissions.find(assignmentId, studentId);
    if (existing && (existing->status == SubmissionStatus::Moderated ||
                     existing->status == SubmissionStatus::Released)) {
        throw BusinessRuleError(
            "This submission has already been moderated; an extension can no longer change it.");
    }

    tx_.run([&] {
        Extension e;
        e.assignmentId = assignmentId;
        e.studentId = studentId;
        e.newDeadline = newDeadline;
        e.reason = why;
        e.grantedBy = actor.id();
        e.grantedAt = clock_.now();
        repos_.assignments.saveExtension(e);

        if (existing && existing->status == SubmissionStatus::Late &&
            existing->submittedAt <= newDeadline) {
            SubmissionLifecycle::requireTransition(existing->status, SubmissionStatus::Submitted);
            existing->status = SubmissionStatus::Submitted;
            repos_.submissions.update(*existing);
        }
    });

    SubmissionEvent event;
    event.type = SubmissionEventType::ExtensionGranted;
    event.assignmentId = assignmentId;
    event.studentId = studentId;
    event.actorId = actor.id();
    event.submissionId = existing ? existing->id : 0;
    events_.publish(event);
}

Timestamp AssignmentService::effectiveDeadline(const Assignment& assignment, int studentId) const {
    Timestamp deadline = assignment.deadline;
    if (std::optional<Extension> e = repos_.assignments.findExtension(assignment.id, studentId)) {
        deadline = std::max(deadline, e->newDeadline);
    }
    return deadline;
}

std::vector<StudentAssignmentView> AssignmentService::listForStudent(const User& student) const {
    student.require(Permission::SubmitWork);
    std::vector<StudentAssignmentView> views;
    for (const Course& course : repos_.courses.listForStudent(student.id())) {
        for (const Assignment& a : repos_.assignments.listByCourse(course.id)) {
            StudentAssignmentView v;
            v.assignment = a;
            v.course = course;
            v.effectiveDeadline = effectiveDeadline(a, student.id());
            v.extended = v.effectiveDeadline > a.deadline;
            if (std::optional<Submission> s = repos_.submissions.find(a.id, student.id())) {
                v.status = s->status;
                v.version = s->version;
            }
            views.push_back(v);
        }
    }
    std::sort(views.begin(), views.end(), [](const auto& x, const auto& y) {
        return x.effectiveDeadline < y.effectiveDeadline;
    });
    return views;
}

std::vector<Assignment> AssignmentService::listForCourse(const User& actor, int courseId) const {
    const bool teaches = actor.can(Permission::CreateAssignment) &&
                         repos_.courses.isLecturerOf(courseId, actor.id());
    const bool enrolled =
        actor.can(Permission::SubmitWork) && repos_.courses.isEnrolled(courseId, actor.id());
    std::optional<Course> course = repos_.courses.findById(courseId);
    const bool inScope = course && actor.hasDepartmentScope(course->departmentId);
    if (!teaches && !enrolled && !inScope) {
        throw AuthorizationError("You do not have access to this course.");
    }
    return repos_.assignments.listByCourse(courseId);
}

std::vector<Course> AssignmentService::coursesTaughtBy(const User& lecturer) const {
    lecturer.require(Permission::CreateAssignment);
    return repos_.courses.listForLecturer(lecturer.id());
}

std::vector<Course> AssignmentService::coursesOf(const User& student) const {
    student.require(Permission::SubmitWork);
    return repos_.courses.listForStudent(student.id());
}

std::vector<Extension> AssignmentService::listExtensions(const User& actor,
                                                         int assignmentId) const {
    actor.require(Permission::GrantExtension);
    requireOwnAssignment(actor, assignmentId);
    return repos_.assignments.listExtensions(assignmentId);
}

}  // namespace lms
