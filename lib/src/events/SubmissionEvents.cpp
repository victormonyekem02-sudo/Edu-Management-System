#include "lms/events/SubmissionEvents.h"

#include <algorithm>

namespace lms {

void SubmissionEventPublisher::subscribe(ISubmissionObserver& observer) {
    if (std::find(observers_.begin(), observers_.end(), &observer) == observers_.end()) {
        observers_.push_back(&observer);
    }
}

void SubmissionEventPublisher::unsubscribe(ISubmissionObserver& observer) {
    observers_.erase(std::remove(observers_.begin(), observers_.end(), &observer),
                     observers_.end());
}

void SubmissionEventPublisher::publish(const SubmissionEvent& event) const {
    for (ISubmissionObserver* observer : observers_) {
        observer->onSubmissionEvent(event);
    }
}

}  // namespace lms
