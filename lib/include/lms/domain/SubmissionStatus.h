#pragma once

#include <string>
#include <vector>

namespace lms {

/// Submission life cycle (see docs/05-design/diagrams/state-submission.puml).
///
///   [new] --submit on time--> SUBMITTED --mark--> MARKED --approve/adjust--> MODERATED --release--> RELEASED
///   [new] --submit late-----> LATE ------mark--/    ^  |  (re-mark)                  | (adjust)
///   SUBMITTED --resubmit before deadline--> SUBMITTED |__|                           |__|
///   LATE --extension now covers submission--> SUBMITTED
enum class SubmissionStatus { Submitted, Late, Marked, Moderated, Released };

std::string statusToCode(SubmissionStatus s);
SubmissionStatus statusFromCode(const std::string& code);
std::string statusDisplayName(SubmissionStatus s);

/// Encodes the legal transitions of the state machine in one place.
class SubmissionLifecycle {
public:
    static bool canTransition(SubmissionStatus from, SubmissionStatus to);

    /// Throws BusinessRuleError when the transition is not allowed.
    static void requireTransition(SubmissionStatus from, SubmissionStatus to);

    /// True while the lecturer may still (re)mark the submission.
    static bool isMarkable(SubmissionStatus s);

    /// True once the mark is visible to the student (principal's rule, BR-07).
    static bool isVisibleToStudent(SubmissionStatus s) { return s == SubmissionStatus::Released; }
};

}  // namespace lms
