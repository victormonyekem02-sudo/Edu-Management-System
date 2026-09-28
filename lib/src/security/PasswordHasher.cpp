#include "lms/security/PasswordHasher.h"

#include <argon2.h>

#include <algorithm>
#include <cctype>
#include <fstream>

#include "lms/common/Errors.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>
#endif

namespace lms {

std::vector<std::uint8_t> secureRandomBytes(std::size_t count) {
    std::vector<std::uint8_t> bytes(count);
    if (count == 0) {
        return bytes;
    }
#ifdef _WIN32
    const NTSTATUS status = BCryptGenRandom(nullptr, bytes.data(), static_cast<ULONG>(count),
                                            BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (!BCRYPT_SUCCESS(status)) {
        throw SecurityError("The operating system random number generator is unavailable.");
    }
#else
    std::ifstream urandom("/dev/urandom", std::ios::binary);
    if (!urandom.read(reinterpret_cast<char*>(bytes.data()),
                      static_cast<std::streamsize>(count))) {
        throw SecurityError("The operating system random number generator is unavailable.");
    }
#endif
    return bytes;
}

Argon2idPasswordHasher::Argon2idPasswordHasher(const Argon2Params& params) : params_(params) {
    if (params_.saltLength < ARGON2_MIN_SALT_LENGTH || params_.hashLength < 16 ||
        params_.timeCost < 1 || params_.parallelism < 1) {
        throw SecurityError("Invalid Argon2 parameters.");
    }
}

std::string Argon2idPasswordHasher::hash(const std::string& password) const {
    const std::vector<std::uint8_t> salt = secureRandomBytes(params_.saltLength);
    const std::size_t encodedLength =
        argon2_encodedlen(params_.timeCost, params_.memoryKiB, params_.parallelism,
                          params_.saltLength, params_.hashLength, Argon2_id);
    std::string encoded(encodedLength, '\0');
    const int rc = argon2id_hash_encoded(params_.timeCost, params_.memoryKiB, params_.parallelism,
                                         password.data(), password.size(), salt.data(),
                                         salt.size(), params_.hashLength, encoded.data(),
                                         encoded.size());
    if (rc != ARGON2_OK) {
        throw SecurityError(std::string("Password hashing failed: ") + argon2_error_message(rc));
    }
    encoded.resize(std::char_traits<char>::length(encoded.c_str()));
    return encoded;
}

bool Argon2idPasswordHasher::verify(const std::string& password,
                                    const std::string& encodedHash) const {
    if (encodedHash.rfind("$argon2id$", 0) != 0) {
        return false;
    }
    // argon2id_verify recomputes the hash with the parameters and salt stored in the
    // encoded string and compares in constant time.
    return argon2id_verify(encodedHash.c_str(), password.data(), password.size()) == ARGON2_OK;
}

void PasswordPolicy::validate(const std::string& password, const std::string& username) {
    if (password.size() < kMinLength) {
        throw ValidationError("Password must be at least " + std::to_string(kMinLength) +
                              " characters long.");
    }
    if (password.size() > kMaxLength) {
        throw ValidationError("Password must be at most " + std::to_string(kMaxLength) +
                              " characters long.");
    }
    const auto isLetter = [](char c) { return std::isalpha(static_cast<unsigned char>(c)) != 0; };
    const auto isDigit = [](char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; };
    if (std::none_of(password.begin(), password.end(), isLetter) ||
        std::none_of(password.begin(), password.end(), isDigit)) {
        throw ValidationError("Password must contain at least one letter and one digit.");
    }
    std::string lowerPassword = password;
    std::string lowerUser = username;
    const auto toLower = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };
    std::transform(lowerPassword.begin(), lowerPassword.end(), lowerPassword.begin(), toLower);
    std::transform(lowerUser.begin(), lowerUser.end(), lowerUser.begin(), toLower);
    if (!lowerUser.empty() && lowerPassword == lowerUser) {
        throw ValidationError("Password must not be the same as the username.");
    }
}

}  // namespace lms
