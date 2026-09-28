#pragma once

#include <stdexcept>
#include <string>

namespace lms {

/// Root of the LMS exception hierarchy. The presentation layer catches this type
/// so that no expected failure can terminate the program (NFR-08).
class LmsError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/// Input failed a syntactic or range check (e.g. empty title, mark above maximum).
class ValidationError : public LmsError {
public:
    using LmsError::LmsError;
};

/// Credentials were rejected or the account is locked/inactive.
class AuthenticationError : public LmsError {
public:
    using LmsError::LmsError;
};

/// The authenticated user is not allowed to perform the action (FR-02).
class AuthorizationError : public LmsError {
public:
    using LmsError::LmsError;
};

/// A referenced entity (user, course, assignment, ...) does not exist.
class NotFoundError : public LmsError {
public:
    using LmsError::LmsError;
};

/// The action is well-formed but violates a business rule (e.g. resubmitting after the deadline).
class BusinessRuleError : public LmsError {
public:
    using LmsError::LmsError;
};

/// SQLite reported an error; the message never contains user passwords.
class DatabaseError : public LmsError {
public:
    using LmsError::LmsError;
};

/// A cryptographic primitive failed (random source unavailable, hashing error).
class SecurityError : public LmsError {
public:
    using LmsError::LmsError;
};

}  // namespace lms
