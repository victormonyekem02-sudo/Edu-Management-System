#pragma once

#include <string>
#include <vector>

namespace lms {

enum class SubmissionEventType {
    Submitted,
    Resubmitted,
    Marked,
    Moderated,
    Released,
    ExtensionGranted
};

struct SubmissionEvent {
    SubmissionEventType type = SubmissionEventType::Submitted;
    int assignmentId = 0;
    int studentId = 0;
    int actorId = 0;
    int submissionId = 0;  ///< 0 for ExtensionGranted when nothing has been submitted yet
    bool late = false;
};

/// Observer pattern: services publish life-cycle events without knowing who reacts
/// (notifications today; e.g. e-mail or SMS in a later release).
class ISubmissionObserver {
public:
    virtual ~ISubmissionObserver() = default;
    virtual void onSubmissionEvent(const SubmissionEvent& event) = 0;
};

class SubmissionEventPublisher {
public:
    /// The observer must outlive the publisher or be unsubscribed first.
    void subscribe(ISubmissionObserver& observer);
    void unsubscribe(ISubmissionObserver& observer);
    void publish(const SubmissionEvent& event) const;
    std::size_t observerCount() const noexcept { return observers_.size(); }

private:
    std::vector<ISubmissionObserver*> observers_;
};

}  // namespace lms
