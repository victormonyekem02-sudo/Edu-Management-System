// ST: system tests derived from the use-case specifications. Each test drives the real
// console application (Application + role menus) with scripted keyboard input and checks
// what the user would see. IDs match docs/07-testing/test-plan.md section 6.
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "TestSchool.h"
#include "lms/common/TimeUtil.h"
#include "lms/ui/Application.h"

using namespace lms;
using namespace lms::test;

namespace {

struct RunResult {
    std::string output;
    int exitCode;
};

RunResult runApp(LmsSystem& system, const std::string& script) {
    std::istringstream in(script);
    std::ostringstream out;
    ui::ConsoleIO io(in, out, false);
    ui::Application app(system, io);
    const int code = app.run();
    return {out.str(), code};
}

std::string login(const std::string& user, const std::string& password = kPassword) {
    return "1\n" + user + "\n" + password + "\n";
}

const std::string kLogoutAndExit = "0\n0\n";

bool has(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

class SystemTest : public ::testing::Test {
protected:
    TestSchool school;
    RunResult run(const std::string& script) { return runApp(*school.sys, script); }
};

// ---- UC-01 Submit assignment ---------------------------------------------------------

TEST_F(SystemTest, ST01_SubmitOnTime_MainFlow) {
    const RunResult r = run(login("student") + "3\n1\nMy essay text\n.\n" + kLogoutAndExit);
    EXPECT_EQ(r.exitCode, 0);
    EXPECT_TRUE(has(r.output, "[OK] Submitted on time.")) << r.output;
    EXPECT_EQ(school.sys->submissions().findOwn(*school.student, school.assignment)->content,
              "My essay text");
}

TEST_F(SystemTest, ST02_ResubmitBeforeDeadline_AlternativeFlow) {
    const RunResult r = run(login("student") + "3\n1\nDraft\n.\n3\n1\nFinal version\n.\n" +
                            kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "[OK] Resubmitted (version 2)")) << r.output;
    EXPECT_EQ(school.sys->submissions().findOwn(*school.student, school.assignment)->content,
              "Final version");
}

TEST_F(SystemTest, ST03_SubmitLate_AlternativeFlow) {
    school.setTimeRelativeToDeadline(kSecondsPerHour);
    const RunResult r = run(login("student") + "2\n3\n1\nLate essay\n.\n" + kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "deadline passed")) << r.output;
    EXPECT_TRUE(has(r.output, "[OK] Submitted LATE.")) << r.output;
    EXPECT_TRUE(has(r.output, "10% of the maximum mark per day late")) << r.output;
}

TEST_F(SystemTest, ST04_ResubmitAfterDeadline_ExceptionFlow) {
    school.sys->submissions().submit(*school.student, school.assignment, "Original");
    school.setTimeRelativeToDeadline(1);
    const RunResult r = run(login("student") + "3\n1\nToo late change\n.\n" + kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "[ERROR] The deadline has passed.")) << r.output;
    EXPECT_EQ(school.sys->submissions().findOwn(*school.student, school.assignment)->content,
              "Original");
}

TEST_F(SystemTest, ST05_UnknownAssignment_ExceptionFlowSessionContinues) {
    const RunResult r = run(login("student") + "3\n99\nText\n.\n1\n" + kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "[ERROR] Assignment 99 does not exist.")) << r.output;
    EXPECT_TRUE(has(r.output, "MAT101  Mathematics")) << r.output;  // still logged in
    EXPECT_EQ(r.exitCode, 0);
}

TEST_F(SystemTest, ST06_EmptySubmission_ExceptionFlow) {
    const RunResult r = run(login("student") + "3\n1\n   \n.\n" + kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "[ERROR] Submission must not be empty.")) << r.output;
}

TEST_F(SystemTest, ST07_NotEnrolled_ExceptionFlow) {
    const RunResult r = run(login("outsider") + "3\n1\nText\n.\n" + kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "[ERROR] You are not enrolled")) << r.output;
}

// ---- UC-02 Mark submission -------------------------------------------------------------

TEST_F(SystemTest, ST08_LecturerMarksLateWorkWithPenalty) {
    school.setTimeRelativeToDeadline(10);
    school.sys->submissions().submit(*school.student, school.assignment, "Answers");
    const RunResult r =
        run(login("lecturer") + "3\n1\n1\n4\n1\n20\nGood method.\nCheck Q3.\n.\n\n" +
            kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "1 day(s) late")) << r.output;
    EXPECT_TRUE(has(r.output, "[OK] Marked. Raw 20, final 17.5 (1 day late - 10% of 25 = 2.5 deducted)."))
        << r.output;
    EXPECT_EQ(school.sys->submissions().findOwn(*school.student, school.assignment)->feedback,
              "Good method.\nCheck Q3.");
}

TEST_F(SystemTest, ST09_RemarkWithoutReason_ExceptionFlow) {
    school.submitAndMark(*school.student, 10);
    const RunResult r = run(login("lecturer") + "4\n1\n12\nBetter\n.\n\n" + kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "[ERROR] A reason is required when changing an existing mark."))
        << r.output;
}

TEST_F(SystemTest, ST10_MarkOutOfRangeIsReprompted) {
    school.sys->submissions().submit(*school.student, school.assignment, "Answers");
    const RunResult r = run(login("lecturer") + "4\n1\n-3\nabc\n30\nFeedback\n.\n\n" +
                            kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "Please enter a number from 0 to 1000.")) << r.output;
    EXPECT_TRUE(has(r.output, "[ERROR] Mark must be between 0 and 25.")) << r.output;
}

// ---- UC-03 Moderate and release marks --------------------------------------------------

TEST_F(SystemTest, ST11_ModerateReleaseAndStudentSeesMark) {
    school.submitAndMark(*school.student, 20);
    const RunResult hod = run(login("hod") + "1\n2\n1\nAgreed\n5\n1\ny\n" + kLogoutAndExit);
    EXPECT_TRUE(has(hod.output, "awaits moderation")) << hod.output;  // notification at login
    EXPECT_TRUE(has(hod.output, "[OK] Mark approved.")) << hod.output;
    EXPECT_TRUE(has(hod.output, "[OK] 1 mark(s) released.")) << hod.output;

    const RunResult student = run(login("student") + "4\n" + kLogoutAndExit);
    EXPECT_TRUE(has(student.output, "have been released")) << student.output;
    EXPECT_TRUE(has(student.output, "20/25")) << student.output;
    EXPECT_TRUE(has(student.output, "Good work")) << student.output;
}

TEST_F(SystemTest, ST12_StudentCannotSeeUnreleasedMark) {
    school.submitAndMark(*school.student, 20);
    const RunResult r = run(login("student") + "4\n" + kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "Not released yet")) << r.output;
    EXPECT_FALSE(has(r.output, "20/25")) << r.output;
    EXPECT_FALSE(has(r.output, "Good work")) << r.output;
}

TEST_F(SystemTest, ST13_HodAdjustsMarkWithReason) {
    school.submitAndMark(*school.student, 20);
    const RunResult r = run(login("hod") + "3\n1\n18\nToo generous on Q2\n7\n" + kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "[OK] Mark adjusted")) << r.output;
    EXPECT_TRUE(has(r.output, "MODERATION_ADJUST")) << r.output;
    EXPECT_TRUE(has(r.output, "Too generous on Q2")) << r.output;
}

TEST_F(SystemTest, ST14_HodOfOtherDepartmentIsRefused) {
    school.submitAndMark(*school.student, 20);
    const RunResult r = run(login("otherhod") + "2\n1\n\n" + kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "[ERROR] This submission belongs to another department."))
        << r.output;
}

TEST_F(SystemTest, ST15_ReleaseCancelledKeepsMarksHidden) {
    const int id = school.submitAndMark(*school.student, 20);
    school.sys->moderation().approve(*school.hod, id);
    const RunResult r = run(login("hod") + "5\n1\nn\n" + kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "Nothing released.")) << r.output;
    EXPECT_FALSE(school.sys->submissions().resultsFor(*school.student)[0].released);
}

// ---- Authentication, access control and robustness -------------------------------------

TEST_F(SystemTest, ST16_WrongPasswordIsRejected) {
    const RunResult r = run(login("student", "Wrong1234") + "0\n");
    EXPECT_TRUE(has(r.output, "[ERROR] Invalid username or password.")) << r.output;
    EXPECT_FALSE(has(r.output, "Student menu"));
}

TEST_F(SystemTest, ST17_EachRoleSeesOnlyItsOwnMenu) {
    const RunResult student = run(login("student") + kLogoutAndExit);
    EXPECT_TRUE(has(student.output, "Student menu"));
    EXPECT_FALSE(has(student.output, "Approve a mark"));
    EXPECT_FALSE(has(student.output, "Mark or re-mark"));
    EXPECT_FALSE(has(student.output, "Create user account"));

    const RunResult hod = run(login("hod") + kLogoutAndExit);
    EXPECT_TRUE(has(hod.output, "Head of Department menu"));
    EXPECT_FALSE(has(hod.output, "Submit or resubmit work"));
    EXPECT_FALSE(has(hod.output, "Create an assignment"));

    const RunResult lecturer = run(login("lecturer") + kLogoutAndExit);
    EXPECT_TRUE(has(lecturer.output, "Lecturer menu"));
    EXPECT_FALSE(has(lecturer.output, "Release moderated marks"));

    const RunResult admin = run(login("admin") + kLogoutAndExit);
    EXPECT_TRUE(has(admin.output, "Administrator menu"));
    EXPECT_FALSE(has(admin.output, "Submit or resubmit work"));
}

TEST_F(SystemTest, ST18_InvalidMenuInputDoesNotCrash) {
    const RunResult r = run("abc\n7\n\n" + login("student") + "x\n99\n-1\n" + kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "Please enter a whole number from 0 to 1.")) << r.output;
    EXPECT_TRUE(has(r.output, "Please enter a whole number from 0 to 6.")) << r.output;
    EXPECT_TRUE(has(r.output, "Goodbye."));
    EXPECT_EQ(r.exitCode, 0);
}

TEST_F(SystemTest, ST19_EndOfInputExitsCleanly) {
    const RunResult r = run(login("student") + "3\n1\nhalf an ans");
    EXPECT_TRUE(has(r.output, "Input closed - exiting.")) << r.output;
    EXPECT_EQ(r.exitCode, 0);
}

TEST(SystemFirstRun, ST20_FirstRunCreatesAdministrator) {
    LmsSystem system(":memory:", std::make_unique<FixedClock>(kT0),
                     std::make_unique<Argon2idPasswordHasher>(Argon2Params::fastForTesting()),
                     std::make_unique<PercentPerDayPenaltyPolicy>());
    const RunResult r = runApp(system, "root\nRoot User\nshort\nshort\n"
                                       "root\nRoot User\nRootPass1\nRootPass1\n" +
                                           login("root", "RootPass1") + kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "First run")) << r.output;
    EXPECT_TRUE(has(r.output, "[ERROR] Password must be at least 8 characters long.")) << r.output;
    EXPECT_TRUE(has(r.output, "[OK] Administrator created.")) << r.output;
    EXPECT_TRUE(has(r.output, "Administrator menu")) << r.output;
}

// ---- Administration and extensions ------------------------------------------------------

TEST_F(SystemTest, ST21_AdminCreatesAndEnrolsStudentWhoCanThenLogIn) {
    const RunResult admin =
        run(login("admin") + "2\n4\nnewkid\nNew Kid\n\nKidPass2026\n10\n1\nnewkid\n" +
            kLogoutAndExit);
    EXPECT_TRUE(has(admin.output, "[OK] Created Student 'newkid'")) << admin.output;
    EXPECT_TRUE(has(admin.output, "[OK] New Kid enrolled.")) << admin.output;
    const RunResult kid = run(login("newkid", "KidPass2026") + "2\n" + kLogoutAndExit);
    EXPECT_TRUE(has(kid.output, "Homework 1")) << kid.output;
}

TEST_F(SystemTest, ST22_LecturerGrantsExtensionShownToStudent) {
    const std::string newDeadline = time_util::formatLocal(school.deadline + kSecondsPerDay);
    const RunResult lect = run(login("lecturer") + "5\n1\nstudent\n" + newDeadline +
                               "\nMedical certificate\n" + kLogoutAndExit);
    EXPECT_TRUE(has(lect.output, "[OK] Extension granted to Student One")) << lect.output;
    const RunResult stud = run(login("student") + "2\n" + kLogoutAndExit);
    EXPECT_TRUE(has(stud.output, "granted an extension")) << stud.output;
    EXPECT_TRUE(has(stud.output, newDeadline + " (extended)")) << stud.output;
}

TEST_F(SystemTest, ST23_ChangePassword) {
    const RunResult r =
        run(login("student") + "6\n" + kPassword + "\nBrandNew2026\nBrandNew2026\n" +
            kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "[OK] Password changed.")) << r.output;
    EXPECT_NO_THROW(school.sys->auth().login("student", "BrandNew2026"));
}

TEST_F(SystemTest, ST24_AccountLocksAfterFiveFailures) {
    std::string script;
    for (int i = 0; i < 5; ++i) {
        script += login("student", "Wrong1234");
    }
    script += login("student") + "0\n";
    const RunResult r = run(script);
    EXPECT_TRUE(has(r.output, "temporarily locked")) << r.output;
    EXPECT_FALSE(has(r.output, "Student menu"));
}

// ---- File submission (review finding R3) -------------------------------------------------

std::string writeTempFile(const std::string& name, const std::string& content) {
    const auto path = std::filesystem::temp_directory_path() / name;
    std::ofstream(path, std::ios::binary) << content;
    return path.string();
}

TEST_F(SystemTest, ST25_SubmitFileByPath) {
    const std::string path = writeTempFile("lms_st25_essay.txt", "Essay from a file\nLine 2");
    const RunResult r = run(login("student") + "3\n1\n@" + path + "\n.\n" + kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "[OK] Submitted on time.")) << r.output;
    EXPECT_EQ(school.sys->submissions().findOwn(*school.student, school.assignment)->content,
              "Essay from a file\nLine 2");
    std::filesystem::remove(path);
}

TEST_F(SystemTest, ST26_MissingOrOversizedFileIsRejected) {
    const std::string big =
        writeTempFile("lms_st26_big.txt", std::string(SubmissionService::kMaxContentLength + 1, 'x'));
    const RunResult r = run(login("student") + "3\n1\n@no_such_file.txt\n.\n3\n1\n@" + big +
                            "\n.\n" + kLogoutAndExit);
    EXPECT_TRUE(has(r.output, "is not a file that can be submitted")) << r.output;
    EXPECT_TRUE(has(r.output, "The file is too large")) << r.output;
    EXPECT_FALSE(school.sys->submissions().findOwn(*school.student, school.assignment));
    std::filesystem::remove(big);
}

TEST(SystemFileDb, ST27_DatabaseFileCannotBeSubmitted) {
    const std::string db = (std::filesystem::temp_directory_path() / "lms_st27.db").string();
    for (const char* suffix : {"", "-wal", "-shm"}) {
        std::filesystem::remove(db + suffix);
    }
    {
        TestSchool school(std::make_unique<PercentPerDayPenaltyPolicy>(), nullptr, db);
        const RunResult r =
            runApp(*school.sys, login("student") + "3\n1\n@" + db + "\n.\n" + kLogoutAndExit);
        EXPECT_TRUE(has(r.output, "[ERROR] That file cannot be submitted.")) << r.output;
    }
    for (const char* suffix : {"", "-wal", "-shm"}) {
        std::filesystem::remove(db + suffix);
    }
}

}  // namespace
