#pragma once

#include <memory>
#include <string>

#include "lms/common/Clock.h"
#include "lms/data/Repositories.h"
#include "lms/domain/User.h"
#include "lms/security/PasswordHasher.h"

namespace lms {

/// Authentication and credential management (FR-01, NFR-04, NFR-05).
class AuthService {
public:
    static constexpr int kMaxFailedAttempts = 5;
    static constexpr std::int64_t kLockoutSeconds = 15 * kSecondsPerMinute;

    AuthService(IUserRepository& users, const IPasswordHasher& hasher, const IClock& clock);

    /// Verifies the credentials and returns the polymorphic user. Throws
    /// AuthenticationError with a deliberately generic message on any failure, so an
    /// attacker cannot tell whether a username exists. Five consecutive failures lock
    /// the account for 15 minutes.
    std::unique_ptr<User> login(const std::string& username, const std::string& password);

    /// Changes the actor's own password after re-verifying the current one.
    void changePassword(const User& actor, const std::string& currentPassword,
                        const std::string& newPassword);

private:
    IUserRepository& users_;
    const IPasswordHasher& hasher_;
    const IClock& clock_;
    /// Hash of a fixed value, verified when the username is unknown so that a failed
    /// login takes the same time whether or not the account exists (review finding R1).
    std::string dummyHash_;
};

}  // namespace lms
