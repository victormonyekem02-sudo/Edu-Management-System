// UT-SEC: password hashing, random source and password policy (NFR-04, NFR-05).
#include <gtest/gtest.h>

#include <set>

#include "lms/common/Errors.h"
#include "lms/security/PasswordHasher.h"

using namespace lms;

namespace {

class HasherTest : public ::testing::Test {
protected:
    Argon2idPasswordHasher hasher{Argon2Params::fastForTesting()};
};

TEST_F(HasherTest, ProducesEncodedArgon2idHash) {
    const std::string h = hasher.hash("Secret123");
    EXPECT_EQ(h.rfind("$argon2id$v=19$m=64,t=1,p=1$", 0), 0u) << h;
    EXPECT_EQ(h.find("Secret123"), std::string::npos);
}

TEST_F(HasherTest, VerifiesCorrectPasswordOnly) {
    const std::string h = hasher.hash("Secret123");
    EXPECT_TRUE(hasher.verify("Secret123", h));
    EXPECT_FALSE(hasher.verify("secret123", h));
    EXPECT_FALSE(hasher.verify("Secret1234", h));
    EXPECT_FALSE(hasher.verify("", h));
}

TEST_F(HasherTest, SaltMakesEveryHashUnique) {
    std::set<std::string> hashes;
    for (int i = 0; i < 5; ++i) {
        hashes.insert(hasher.hash("SamePassword1"));
    }
    EXPECT_EQ(hashes.size(), 5u);
}

TEST_F(HasherTest, RejectsMalformedOrForeignHashes) {
    EXPECT_FALSE(hasher.verify("x", ""));
    EXPECT_FALSE(hasher.verify("x", "plaintext"));
    EXPECT_FALSE(hasher.verify("x", "$argon2i$v=19$m=64,t=1,p=1$abc$def"));
    EXPECT_FALSE(hasher.verify("x", "$argon2id$garbage"));
}

TEST_F(HasherTest, HashFromOtherParametersStillVerifies) {
    // Parameters are stored inside the hash, so raising the cost later does not
    // invalidate existing accounts.
    Argon2idPasswordHasher stronger({2, 128, 1, 16, 32});
    const std::string h = stronger.hash("Rotate123");
    EXPECT_TRUE(hasher.verify("Rotate123", h));
}

TEST(UT_SEC, ProductionParametersMeetOwaspMinimum) {
    const Argon2Params p = Argon2Params::production();
    EXPECT_GE(p.memoryKiB, 19456u);
    EXPECT_GE(p.timeCost, 2u);
    EXPECT_GE(p.parallelism, 1u);
    EXPECT_GE(p.saltLength, 16u);
    EXPECT_GE(p.hashLength, 32u);
}

TEST(UT_SEC, InvalidParametersRejected) {
    EXPECT_THROW(Argon2idPasswordHasher({1, 64, 1, 4, 32}), SecurityError);   // salt too short
    EXPECT_THROW(Argon2idPasswordHasher({0, 64, 1, 16, 32}), SecurityError);  // zero iterations
}

TEST(UT_SEC, SecureRandomBytesHaveRequestedLengthAndVary) {
    EXPECT_TRUE(secureRandomBytes(0).empty());
    const auto a = secureRandomBytes(32);
    const auto b = secureRandomBytes(32);
    EXPECT_EQ(a.size(), 32u);
    EXPECT_NE(a, b);
}

struct PolicyCase {
    const char* password;
    bool valid;
};

class PasswordPolicyCases : public ::testing::TestWithParam<PolicyCase> {};

TEST_P(PasswordPolicyCases, EnforcesRules) {
    const PolicyCase& c = GetParam();
    if (c.valid) {
        EXPECT_NO_THROW(PasswordPolicy::validate(c.password, "jdoe")) << c.password;
    } else {
        EXPECT_THROW(PasswordPolicy::validate(c.password, "jdoe"), ValidationError) << c.password;
    }
}

INSTANTIATE_TEST_SUITE_P(UT_SEC, PasswordPolicyCases,
                         ::testing::Values(PolicyCase{"", false},
                                           PolicyCase{"abc1234", false},    // 7 chars (boundary)
                                           PolicyCase{"abcd1234", true},    // 8 chars (boundary)
                                           PolicyCase{"abcdefgh", false},   // no digit
                                           PolicyCase{"12345678", false},   // no letter
                                           PolicyCase{"Correct horse 9", true}));

TEST(UT_SEC, PasswordLengthUpperBoundary) {
    EXPECT_NO_THROW(PasswordPolicy::validate(std::string(127, 'a') + "1", "u"));   // 128
    EXPECT_THROW(PasswordPolicy::validate(std::string(128, 'a') + "1", "u"),       // 129
                 ValidationError);
}

TEST(UT_SEC, PasswordMustDifferFromUsername) {
    EXPECT_THROW(PasswordPolicy::validate("Teacher01", "teacher01"), ValidationError);
    EXPECT_NO_THROW(PasswordPolicy::validate("Teacher01", "teacher02"));
}

}  // namespace
