#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace lms {

/// Abstraction over the password hashing algorithm so the AuthService does not depend
/// on a particular library (Dependency Inversion) and tests can use cheaper parameters.
class IPasswordHasher {
public:
    virtual ~IPasswordHasher() = default;
    /// Returns a self-describing encoded hash that embeds algorithm, parameters and salt.
    virtual std::string hash(const std::string& password) const = 0;
    /// Constant-time verification of a password against an encoded hash.
    virtual bool verify(const std::string& password, const std::string& encodedHash) const = 0;
};

/// Argon2id cost parameters (RFC 9106).
struct Argon2Params {
    std::uint32_t timeCost = 2;         ///< iterations
    std::uint32_t memoryKiB = 19456;    ///< 19 MiB
    std::uint32_t parallelism = 1;
    std::uint32_t saltLength = 16;      ///< bytes, from the OS CSPRNG
    std::uint32_t hashLength = 32;      ///< bytes

    /// OWASP Password Storage Cheat Sheet minimum for Argon2id (m=19 MiB, t=2, p=1).
    /// Chosen because the school's computers are old (NFR-01 login within 2 s).
    static Argon2Params production() { return {}; }

    /// Cheap parameters for the automated tests only. Never used by the application.
    static Argon2Params fastForTesting() { return {1, 64, 1, 16, 32}; }
};

/// Salted Argon2id hashing using the reference implementation from the
/// Password Hashing Competition (https://github.com/P-H-C/phc-winner-argon2).
class Argon2idPasswordHasher final : public IPasswordHasher {
public:
    explicit Argon2idPasswordHasher(Argon2Params params = Argon2Params::production());
    std::string hash(const std::string& password) const override;
    bool verify(const std::string& password, const std::string& encodedHash) const override;
    const Argon2Params& params() const noexcept { return params_; }

private:
    Argon2Params params_;
};

/// Cryptographically secure random bytes from the operating system
/// (BCryptGenRandom on Windows, /dev/urandom elsewhere). Throws SecurityError on failure.
std::vector<std::uint8_t> secureRandomBytes(std::size_t count);

/// Password rules (NFR-05): 8-128 characters, at least one letter and one digit,
/// and not equal to the username. Throws ValidationError describing the first failure.
class PasswordPolicy {
public:
    static constexpr std::size_t kMinLength = 8;
    static constexpr std::size_t kMaxLength = 128;
    static void validate(const std::string& password, const std::string& username);
};

}  // namespace lms
