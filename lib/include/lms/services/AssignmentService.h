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

/// An assignment as one student sees it: with their own effective deadline.
struct StudentAssignmentView {
    Assignment assignment;
    Course course;
    Timestamp effectiveDeadline = 0;
    bool extended = false;
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

    /// Deadline that applies to this student: the later of the assignment deadline and
    /// any extension granted to them.
    Timestamp effectiveDeadline(const Assignment& assignment, int studentId) const;

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
