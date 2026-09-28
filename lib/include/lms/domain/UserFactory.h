#pragma once

#include <memory>
#include <optional>
#include <string>

#include "lms/common/Clock.h"
#include "lms/domain/Role.h"
#include "lms/domain/User.h"

namespace lms {

/// Persistent representation of a user account (maps 1:1 to table app_user).
/// Kept separate from the polymorphic User hierarchy so that the password hash and
/// lockout counters never leave the data/authentication layers.
struct UserRecord {
    int id = 0;
    std::string username;
    std::string passwordHash;
    std::string fullName;
    Role role = Role::Student;
    std::optional<int> departmentId;
    bool active = true;
    int failedLoginAttempts = 0;
    Timestamp lockedUntil = 0;
    Timestamp createdAt = 0;
};

/// Factory Method: the single place that maps a stored role code to the
/// concrete User subclass. Callers only ever see `std::unique_ptr<User>`.
class UserFactory {
public:
    static std::unique_ptr<User> create(const UserRecord& record);
};

}  // namespace lms
