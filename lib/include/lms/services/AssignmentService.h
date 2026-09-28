#pragma once

#include <optional>
#include <string>
#include <vector>

#include "lms/common/Clock.h"
#include "lms/data/Database.h"
#include "lms/data/Repositories.h"
#include "lms/domain/User.h"
#include "lms/events/SubmissionEvents.h"

namespace lms {

/// Which extension, if any, decides a student's effective deadline.
enum class ExtensionKind { None, Individual, Class };

std::string extensionKindName(ExtensionKind kind);

/// A student's effective deadline and where it comes from.
struct DeadlineInfo {
    Timestamp deadline = 0;
    ExtensionKind kind = ExtensionKind::None;
};

/// An assignment as one student sees it: with their own effective deadline.
struct StudentAssignmentView {
    Assignment assignment;
    Course course;
    Timestamp effectiveDeadline = 0;
    bool extended = false;
    ExtensionKind extensionKind = ExtensionKind::None;  ///< v1.1 (CR1)
    std::optional<SubmissionStatus> status;  ///< empty when nothing submitted yet
    int version = 0;
};

/// Lecturer use cases around assignments and extensions (FR-07, FR-08, FR-09).
class AssignmentService {
public:
    static constexpr std::size_t kMaxTitleLength = 200;
    static constexpr std::size_t kMaxDescriptionLength = 4000;
    static constexpr std::size_t kMaxReasonLength = 500;

    AssignmentService(Repositories repos, ITransactionRunner& tx, const IClock& clock,
                      SubmissionEventPublisher& events);

    int createAssignment(const User& actor, int courseId, const std::string& title,
                         const std::string& description, Timestamp deadline, int maxMark);

    /// Grants (or replaces) an individual extension. The new deadline must be later than
    /// the assignment deadline. If the student's unmarked submission falls inside the new
    /// deadline it is reclassified from LATE to SUBMITTED (BR-03).
    void grantExtension(const User& actor, int assignmentId, int studentId,
                        Timestamp newDeadline, const std::string& reason);

    /// v1.1 (CR1): extends the deadline for every student of the course. The new deadline
    /// must be later than the assignment deadline; granting again replaces it. Refused once
    /// any submission of the assignment has been moderated. Unmarked LATE submissions that
    /// the new deadline covers become SUBMITTED.
    void grantClassExtension(const User& actor, int assignmentId, Timestamp newDeadline,
                             const std::string& reason);
    std::optional<ClassExtension> classExtension(const User& actor, int assignmentId) const;

    /// Deadline that applies to this student: the latest of the assignment deadline, the
    /// class extension and the student's individual extension (BR-03).
    Timestamp effectiveDeadline(const Assignment& assignment, int studentId) const;
    DeadlineInfo deadlineFor(const Assignment& assignment, int studentId) const;

    std::vector<StudentAssignmentView> listForStudent(const User& student) const;
    std::vector<Assignment> listForCourse(const User& actor, int courseId) const;
    std::vector<Course> coursesTaughtBy(const User& lecturer) const;
    std::vector<Course> coursesOf(const User& student) const;
    std::vector<Extension> listExtensions(const User& actor, int assignmentId) const;

    /// Loads the assignment and checks the actor teaches its course.
    Assignment requireOwnAssignment(const User& actor, int assignmentId) const;

private:
    Repositories repos_;
    ITransactionRunner& tx_;
    const IClock& clock_;
    SubmissionEventPublisher& events_;
};

}  // namespace lms
