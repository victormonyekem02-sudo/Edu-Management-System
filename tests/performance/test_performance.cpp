// PT: performance tests for the measurable non-functional requirements, with all 800
// students stored and production Argon2id parameters. Run in a Release build.
#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <iostream>

#include "TestSchool.h"

using namespace lms;
using namespace lms::test;

namespace {

constexpr int kStudents = 800;
constexpr int kAssignments = 20;

double secondsSince(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
}

class PerformanceTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        path_ = (std::filesystem::temp_directory_path() / "lms_perf_test.db").string();
        for (const char* suffix : {"", "-wal", "-shm"}) {
            std::filesystem::remove(path_ + suffix);
        }
        school_ = new TestSchool(std::make_unique<PercentPerDayPenaltyPolicy>(),
                                 std::make_unique<Argon2idPasswordHasher>(), path_);
        Repositories repos = school_->sys->repositories();

        // One real production hash is reused for all generated students: hashing 800
        // passwords individually would only slow down the fixture, not the measured code.
        const std::string hash = Argon2idPasswordHasher().hash(kPassword);
        school_->sys->database().inTransaction([&] {
            for (int i = 1; i <= kStudents; ++i) {
                UserRecord r;
                r.username = "stu" + std::to_string(i);
                r.passwordHash = hash;
                r.fullName = "Student " + std::to_string(i);
                r.role = Role::Student;
                r.createdAt = kT0;
                const int id = repos.users.add(r);
                repos.courses.enrol(school_->course, id, kT0);
            }
            for (int a = 2; a <= kAssignments; ++a) {
                school_->sys->assignments().createAssignment(
                    *school_->lecturer, school_->course, "Task " + std::to_string(a), "",
                    school_->deadline + a * kSecondsPerDay, 20);
            }
        });
        // Every student submits assignment 1 and the lecturer marks all of them.
        school_->sys->database().inTransaction([&] {
            for (int i = 1; i <= kStudents; ++i) {
                auto rec = repos.users.findByUsername("stu" + std::to_string(i));
                auto student = UserFactory::create(*rec);
                const int sid = school_->sys->submissions()
                                    .submit(*student, school_->assignment, "answer")
                                    .submissionId;
                school_->sys->marking().mark(*school_->lecturer, sid, 15, "ok");
            }
        });
    }

    static void TearDownTestSuite() {
        delete school_;
        school_ = nullptr;
        for (const char* suffix : {"", "-wal", "-shm"}) {
            std::filesystem::remove(path_ + suffix);
        }
    }

    static void report(const char* nfr, double seconds, double limit) {
        std::cout << "[PERF] " << nfr << ": " << seconds * 1000.0 << " ms (limit "
                  << limit * 1000.0 << " ms)\n";
        ::testing::Test::RecordProperty(nfr, std::to_string(seconds * 1000.0) + " ms");
    }

    static TestSchool* school_;
    static std::string path_;
};

TestSchool* PerformanceTest::school_ = nullptr;
std::string PerformanceTest::path_;

TEST_F(PerformanceTest, NFR01_LoginCompletesWithinTwoSeconds) {
    EXPECT_GE(school_->sys->repositories().users.countByRole(Role::Student), kStudents);
    const auto start = std::chrono::steady_clock::now();
    auto user = school_->sys->auth().login("stu800", kPassword);
    const double elapsed = secondsSince(start);
    report("NFR-01 login", elapsed, 2.0);
    EXPECT_EQ(user->role(), Role::Student);
    EXPECT_LT(elapsed, 2.0);
}

TEST_F(PerformanceTest, NFR02_AssignmentListWithinOneSecond) {
    auto rec = school_->sys->repositories().users.findByUsername("stu400");
    auto student = UserFactory::create(*rec);
    const auto start = std::chrono::steady_clock::now();
    const auto list = school_->sys->assignments().listForStudent(*student);
    const double elapsed = secondsSince(start);
    report("NFR-02 assignment list", elapsed, 1.0);
    EXPECT_EQ(list.size(), static_cast<std::size_t>(kAssignments));
    EXPECT_LT(elapsed, 1.0);
}

TEST_F(PerformanceTest, NFR02_SubmissionWithinOneSecond) {
    auto rec = school_->sys->repositories().users.findByUsername("stu401");
    auto student = UserFactory::create(*rec);
    const auto start = std::chrono::steady_clock::now();
    school_->sys->submissions().submit(*student, school_->assignment + 1, "answer");
    const double elapsed = secondsSince(start);
    report("NFR-02 submit", elapsed, 1.0);
    EXPECT_LT(elapsed, 1.0);
}

TEST_F(PerformanceTest, NFR03_ModerationQueueOf800WithinTwoSeconds) {
    const auto start = std::chrono::steady_clock::now();
    const auto queue = school_->sys->moderation().pendingModeration(*school_->hod);
    const double elapsed = secondsSince(start);
    report("NFR-03 moderation queue (800 items)", elapsed, 2.0);
    EXPECT_EQ(queue.size(), static_cast<std::size_t>(kStudents));
    EXPECT_LT(elapsed, 2.0);
}

TEST_F(PerformanceTest, NFR03_ReleaseOf800MarksWithinFiveSeconds) {
    TestSchool& s = *school_;
    for (const ModerationItem& item : s.sys->moderation().pendingModeration(*s.hod)) {
        s.sys->moderation().approve(*s.hod, item.submission.id);
    }
    const auto start = std::chrono::steady_clock::now();
    const ReleaseSummary r = s.sys->moderation().releaseAssignment(*s.hod, s.assignment);
    const double elapsed = secondsSince(start);
    report("NFR-03 release 800 marks", elapsed, 5.0);
    EXPECT_EQ(r.released, kStudents);
    EXPECT_LT(elapsed, 5.0);
}

}  // namespace
