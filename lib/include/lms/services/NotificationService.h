#pragma once

#include <vector>

#include "lms/common/Clock.h"
#include "lms/data/Repositories.h"
#include "lms/domain/User.h"
#include "lms/events/SubmissionEvents.h"

namespace lms {

/// Concrete observer: turns submission life-cycle events into in-app notifications
/// shown at login (FR-19). Recipients:
///  - Submitted / Resubmitted -> lecturers of the course
///  - Marked                  -> Head of the course's department
///  - Released, ExtensionGranted, Moderated -> the student (Moderated: no mark revealed)
class NotificationService final : public ISubmissionObserver {
public:
    NotificationService(Repositories repos, const IClock& clock);

    void onSubmissionEvent(const SubmissionEvent& event) override;

    std::vector<Notification> unreadFor(const User& user) const;
    std::vector<Notification> allFor(const User& user) const;
    void markAllRead(const User& user);

private:
    void notify(int userId, const std::string& message);

    Repositories repos_;
    const IClock& clock_;
};

}  // namespace lms
