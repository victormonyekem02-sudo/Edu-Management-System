#pragma once

#include <optional>
#include <string>
#include <vector>

#include "lms/common/Clock.h"
#include "lms/data/Database.h"
#include "lms/data/Repositories.h"
#include "lms/domain/User.h"
#include "lms/events/SubmissionEvents.h"
#include "lms/services/AssignmentService.h"

namespace lms {

struct SubmitResult {
    int submissionId = 0;
    int version = 1;
    SubmissionStatus status = SubmissionStatus::Submitted;
    bool resubmission = false;
    bool late = false;
    Timestamp effectiveDeadline = 0;
};

/// What a student may see about one of their assignments. Mark and feedback are only
/// filled in once the Head of Department has released the mark (BR-07).
struct StudentResultView {
    Assignment assignment;
    Course course;
    SubmissionStatus status = SubmissionStatus::Submitted;
    bool released = false;
    std::optional<double> finalMark;
    std::optional<double> rawMark;
    int daysLate = 0;
    double lateDeduction = 0.0;
    std::string feedback;
};

/// Student use cases: submit, resubmit, view results (FR-10, FR-11, FR-12, FR-13).
class SubmissionService {
public:
    static constexpr std::size_t kMaxContentLength = 100000;

    SubmissionService(Repositories repos, ITransactionRunner& tx, const IClock& clock,
                      SubmissionEventPublisher& events, const AssignmentService& assignments);

    /// UC-01 "Submit assignment". On-time work is SUBMITTED, late work is accepted as LATE
    /// (never rejected, BR-04). Resubmission replaces the work and increments the version,
    /// but only up to the student's effective deadline and only before marking (BR-05).
    SubmitResult submit(const User& actor, int assignmentId, const std::string& content);

    std::optional<Submission> findOwn(const User& student, int assignmentId) const;
    std::vector<StudentResultView> resultsFor(const User& student) const;

private:
    Repositories repos_;
    ITransactionRunner& tx_;
    const IClock& clock_;
    SubmissionEventPublisher& events_;
    const AssignmentService& assignments_;
};

}  // namespace lms
