#pragma once

#include <memory>
#include <string>

#include "lms/common/Clock.h"
#include "lms/data/Database.h"
#include "lms/data/SqliteRepositories.h"
#include "lms/events/SubmissionEvents.h"
#include "lms/policy/LatePenaltyPolicy.h"
#include "lms/security/PasswordHasher.h"
#include "lms/services/AdminService.h"
#include "lms/services/AssignmentService.h"
#include "lms/services/AuthService.h"
#include "lms/services/DirectoryService.h"
#include "lms/services/MarkingService.h"
#include "lms/services/ModerationService.h"
#include "lms/services/NotificationService.h"
#include "lms/services/SubmissionService.h"

namespace lms {

/// Composition root: opens the database, applies migrations and wires every service to
/// its dependencies. The presentation layer and the tests construct exactly one of these.
class LmsSystem {
public:
    LmsSystem(const std::string& databasePath, std::unique_ptr<IClock> clock,
              std::unique_ptr<IPasswordHasher> hasher,
              std::unique_ptr<ILatePenaltyPolicy> penaltyPolicy);
    ~LmsSystem();
    LmsSystem(const LmsSystem&) = delete;
    LmsSystem& operator=(const LmsSystem&) = delete;

    /// Production configuration: system clock, Argon2id (OWASP parameters),
    /// 10 %-per-day late penalty capped at 3 days.
    static std::unique_ptr<LmsSystem> createDefault(const std::string& databasePath);

    AuthService& auth() { return auth_; }
    AdminService& admin() { return admin_; }
    AssignmentService& assignments() { return assignments_; }
    SubmissionService& submissions() { return submissions_; }
    MarkingService& marking() { return marking_; }
    ModerationService& moderation() { return moderation_; }
    NotificationService& notifications() { return notifications_; }
    DirectoryService& directory() { return directory_; }
    const IClock& clock() const { return *clock_; }
    const std::string& databasePath() const { return databasePath_; }
    Database& database() { return db_; }
    /// Direct repository access for tests and demo seeding only - the UI uses services.
    Repositories repositories() { return store_.repositories(); }

private:
    std::string databasePath_;
    Database db_;
    std::unique_ptr<IClock> clock_;
    std::unique_ptr<IPasswordHasher> hasher_;
    std::unique_ptr<ILatePenaltyPolicy> penaltyPolicy_;
    SqliteStore store_;
    SubmissionEventPublisher events_;
    AuthService auth_;
    AdminService admin_;
    AssignmentService assignments_;
    SubmissionService submissions_;
    MarkingService marking_;
    ModerationService moderation_;
    NotificationService notifications_;
    DirectoryService directory_;
};

}  // namespace lms
