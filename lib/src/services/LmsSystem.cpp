#include "lms/services/LmsSystem.h"

#include <utility>

namespace lms {

LmsSystem::LmsSystem(const std::string& databasePath, std::unique_ptr<IClock> clock,
                     std::unique_ptr<IPasswordHasher> hasher,
                     std::unique_ptr<ILatePenaltyPolicy> penaltyPolicy)
    : databasePath_(databasePath),
      db_(databasePath),
      clock_(std::move(clock)),
      hasher_(std::move(hasher)),
      penaltyPolicy_(std::move(penaltyPolicy)),
      store_(db_),
      auth_(store_.repositories().users, *hasher_, *clock_),
      admin_(store_.repositories(), *hasher_, *clock_),
      assignments_(store_.repositories(), store_.transactions(), *clock_, events_),
      submissions_(store_.repositories(), store_.transactions(), *clock_, events_, assignments_),
      marking_(store_.repositories(), store_.transactions(), *clock_, events_, assignments_,
               *penaltyPolicy_),
      moderation_(store_.repositories(), store_.transactions(), *clock_, events_),
      notifications_(store_.repositories(), *clock_),
      directory_(store_.repositories(), *clock_) {
    SchemaMigrator::migrate(db_);
    events_.subscribe(notifications_);
}

LmsSystem::~LmsSystem() {
    events_.unsubscribe(notifications_);
}

std::unique_ptr<LmsSystem> LmsSystem::createDefault(const std::string& databasePath) {
    return std::make_unique<LmsSystem>(databasePath, std::make_unique<SystemClock>(),
                                       std::make_unique<Argon2idPasswordHasher>(),
                                       std::make_unique<PercentPerDayPenaltyPolicy>(10.0, 3));
}

}  // namespace lms
