#include "lms/services/NotificationService.h"

#include "lms/common/TimeUtil.h"

namespace lms {

NotificationService::NotificationService(Repositories repos, const IClock& clock)
    : repos_(repos), clock_(clock) {}

void NotificationService::notify(int userId, const std::string& message) {
    Notification n;
    n.userId = userId;
    n.message = message;
    n.createdAt = clock_.now();
    repos_.notifications.add(n);
}

void NotificationService::onSubmissionEvent(const SubmissionEvent& event) {
    std::optional<Assignment> assignment = repos_.assignments.findById(event.assignmentId);
    if (!assignment) {
        return;
    }
    std::optional<Course> course = repos_.courses.findById(assignment->courseId);
    std::optional<UserRecord> student = repos_.users.findById(event.studentId);
    const std::string what = "'" + assignment->title + "'" +
                             (course ? " (" + course->code + ")" : std::string());
    const std::string who = student ? student->fullName : "A student";

    switch (event.type) {
        case SubmissionEventType::Submitted:
        case SubmissionEventType::Resubmitted:
            notifyLecturers(event, *assignment, who, what);
            break;
        case SubmissionEventType::Marked:
            notifyHeadOfDepartment(course, who, what);
            break;
        case SubmissionEventType::Moderated:
            // Deliberately no notification: the student must not learn anything before release.
            break;
        case SubmissionEventType::Released:
            notify(event.studentId, "Your mark and feedback for " + what + " have been released.");
            break;
        case SubmissionEventType::ExtensionGranted:
            notifyExtension(event, what);
            break;
        case SubmissionEventType::ClassExtensionGranted:
            notifyClassExtension(*assignment, what);
            break;
    }
}

void NotificationService::notifyLecturers(const SubmissionEvent& event,
                                          const Assignment& assignment, const std::string& who,
                                          const std::string& what) {
    const std::string verb =
        event.type == SubmissionEventType::Resubmitted ? " resubmitted " : " submitted ";
    const std::string message = who + verb + what + (event.late ? " LATE." : ".");
    for (const int lecturerId : repos_.courses.lecturerIds(assignment.courseId)) {
        notify(lecturerId, message);
    }
}

void NotificationService::notifyHeadOfDepartment(const std::optional<Course>& course,
                                                 const std::string& who,
                                                 const std::string& what) {
    if (!course) {
        return;
    }
    if (std::optional<UserRecord> hod = repos_.users.findHeadOfDepartment(course->departmentId)) {
        notify(hod->id, "A mark for " + who + " on " + what + " awaits moderation.");
    }
}

void NotificationService::notifyClassExtension(const Assignment& assignment,
                                               const std::string& what) {
    if (std::optional<ClassExtension> c = repos_.assignments.findClassExtension(assignment.id)) {
        const std::string message = "The deadline for " + what +
                                    " has been extended for the whole class until " +
                                    time_util::formatLocal(c->newDeadline) + ".";
        for (const int studentId : repos_.courses.studentIds(assignment.courseId)) {
            notify(studentId, message);
        }
    }
}

void NotificationService::notifyExtension(const SubmissionEvent& event, const std::string& what) {
    if (std::optional<Extension> e =
            repos_.assignments.findExtension(event.assignmentId, event.studentId)) {
        notify(event.studentId, "You have been granted an extension for " + what + " until " +
                                    time_util::formatLocal(e->newDeadline) + ".");
    }
}

std::vector<Notification> NotificationService::unreadFor(const User& user) const {
    return repos_.notifications.listForUser(user.id(), true);
}

std::vector<Notification> NotificationService::allFor(const User& user) const {
    return repos_.notifications.listForUser(user.id(), false);
}

void NotificationService::markAllRead(const User& user) {
    repos_.notifications.markAllRead(user.id());
}

}  // namespace lms
