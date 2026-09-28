#include "lms/services/AuthService.h"

#include "lms/common/Errors.h"
#include "lms/common/TextUtil.h"
#include "lms/domain/UserFactory.h"

namespace lms {

namespace {

const char* const kGenericLoginFailure = "Invalid username or password.";

}  // namespace

AuthService::AuthService(IUserRepository& users, const IPasswordHasher& hasher,
                         const IClock& clock)
    : users_(users), hasher_(hasher), clock_(clock) {}

std::unique_ptr<User> AuthService::login(const std::string& username,
                                         const std::string& password) {
    const std::string name = text::trim(username);
    if (name.empty() || password.empty()) {
        throw AuthenticationError(kGenericLoginFailure);
    }
    std::optional<UserRecord> record = users_.findByUsername(name);
    if (!record || !record->active) {
        throw AuthenticationError(kGenericLoginFailure);
    }

    const Timestamp now = clock_.now();
    if (record->lockedUntil > now) {
        throw AuthenticationError(
            "This account is temporarily locked after repeated failed logins. "
            "Try again later or ask the administrator to reset your password.");
    }

    if (!hasher_.verify(password, record->passwordHash)) {
        record->failedLoginAttempts += 1;
        if (record->failedLoginAttempts >= kMaxFailedAttempts) {
            record->lockedUntil = now + kLockoutSeconds;
            record->failedLoginAttempts = 0;
        }
        users_.update(*record);
        throw AuthenticationError(kGenericLoginFailure);
    }

    if (record->failedLoginAttempts != 0 || record->lockedUntil != 0) {
        record->failedLoginAttempts = 0;
        record->lockedUntil = 0;
        users_.update(*record);
    }
    return UserFactory::create(*record);
}

void AuthService::changePassword(const User& actor, const std::string& currentPassword,
                                 const std::string& newPassword) {
    std::optional<UserRecord> record = users_.findById(actor.id());
    if (!record) {
        throw NotFoundError("Your account no longer exists.");
    }
    if (!hasher_.verify(currentPassword, record->passwordHash)) {
        throw AuthenticationError("The current password is incorrect.");
    }
    PasswordPolicy::validate(newPassword, record->username);
    record->passwordHash = hasher_.hash(newPassword);
    users_.update(*record);
}

}  // namespace lms
