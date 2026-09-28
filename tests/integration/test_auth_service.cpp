// IT-AUTH: AuthService - login, lockout, password change (FR-01, NFR-04, NFR-05).
#include <gtest/gtest.h>

#include "TestSchool.h"
#include "lms/common/Errors.h"

using namespace lms;
using namespace lms::test;

namespace {

class AuthTest : public ::testing::Test {
protected:
    TestSchool school;
    AuthService& auth() { return school.sys->auth(); }
};

TEST_F(AuthTest, LoginReturnsPolymorphicUserForEachRole) {
    EXPECT_EQ(auth().login("admin", kPassword)->role(), Role::Administrator);
    EXPECT_EQ(auth().login("lecturer", kPassword)->role(), Role::Lecturer);
    EXPECT_EQ(auth().login("hod", kPassword)->role(), Role::HeadOfDepartment);
    EXPECT_EQ(auth().login("student", kPassword)->role(), Role::Student);
}

TEST_F(AuthTest, UsernameIsCaseInsensitiveAndTrimmed) {
    EXPECT_EQ(auth().login("  Student ", kPassword)->username(), "student");
}

TEST_F(AuthTest, WrongPasswordAndUnknownUserGiveSameMessage) {
    std::string wrongPassword;
    std::string unknownUser;
    try {
        auth().login("student", "WrongPass1");
    } catch (const AuthenticationError& e) {
        wrongPassword = e.what();
    }
    try {
        auth().login("nobody", "WrongPass1");
    } catch (const AuthenticationError& e) {
        unknownUser = e.what();
    }
    EXPECT_EQ(wrongPassword, "Invalid username or password.");
    EXPECT_EQ(unknownUser, wrongPassword);
}

TEST_F(AuthTest, EmptyCredentialsRejected) {
    EXPECT_THROW(auth().login("", kPassword), AuthenticationError);
    EXPECT_THROW(auth().login("student", ""), AuthenticationError);
}

TEST_F(AuthTest, PasswordsAreStoredAsSaltedArgon2idHashes) {
    auto rec = school.sys->repositories().users.findByUsername("student");
    ASSERT_TRUE(rec);
    EXPECT_NE(rec->passwordHash, kPassword);
    EXPECT_EQ(rec->passwordHash.rfind("$argon2id$", 0), 0u);
    auto other = school.sys->repositories().users.findByUsername("student2");
    EXPECT_NE(rec->passwordHash, other->passwordHash);  // same password, different salt
}

TEST_F(AuthTest, InactiveAccountCannotLogIn) {
    school.sys->admin().setActive(*school.admin, school.student->id(), false);
    EXPECT_THROW(auth().login("student", kPassword), AuthenticationError);
    school.sys->admin().setActive(*school.admin, school.student->id(), true);
    EXPECT_NO_THROW(auth().login("student", kPassword));
}

TEST_F(AuthTest, FiveFailuresLockAccountForFifteenMinutes) {
    for (int i = 0; i < AuthService::kMaxFailedAttempts; ++i) {
        EXPECT_THROW(auth().login("student", "WrongPass1"), AuthenticationError);
    }
    // Even the correct password is refused while locked.
    try {
        auth().login("student", kPassword);
        FAIL() << "expected lockout";
    } catch (const AuthenticationError& e) {
        EXPECT_NE(std::string(e.what()).find("locked"), std::string::npos);
    }
    school.clock->advance(AuthService::kLockoutSeconds - 1);
    EXPECT_THROW(auth().login("student", kPassword), AuthenticationError);
    school.clock->advance(1);
    EXPECT_NO_THROW(auth().login("student", kPassword));
}

TEST_F(AuthTest, FourFailuresDoNotLockAndSuccessResetsCounter) {
    for (int i = 0; i < AuthService::kMaxFailedAttempts - 1; ++i) {
        EXPECT_THROW(auth().login("student", "WrongPass1"), AuthenticationError);
    }
    EXPECT_NO_THROW(auth().login("student", kPassword));
    EXPECT_EQ(school.sys->repositories().users.findByUsername("student")->failedLoginAttempts, 0);
    EXPECT_THROW(auth().login("student", "WrongPass1"), AuthenticationError);
    EXPECT_NO_THROW(auth().login("student", kPassword));
}

TEST_F(AuthTest, AdministratorResetUnlocksAccount) {
    for (int i = 0; i < AuthService::kMaxFailedAttempts; ++i) {
        EXPECT_THROW(auth().login("student", "WrongPass1"), AuthenticationError);
    }
    school.sys->admin().resetPassword(*school.admin, school.student->id(), "NewPass2026");
    EXPECT_NO_THROW(auth().login("student", "NewPass2026"));
}

TEST_F(AuthTest, ChangePasswordRequiresCurrentPasswordAndPolicy) {
    EXPECT_THROW(auth().changePassword(*school.student, "WrongPass1", "Another2026"),
                 AuthenticationError);
    EXPECT_THROW(auth().changePassword(*school.student, kPassword, "short1"), ValidationError);
    auth().changePassword(*school.student, kPassword, "Another2026");
    EXPECT_THROW(auth().login("student", kPassword), AuthenticationError);
    EXPECT_NO_THROW(auth().login("student", "Another2026"));
}

}  // namespace
