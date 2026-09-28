#pragma once

#include <memory>
#include <string>

#include "lms/services/LmsSystem.h"

namespace lms::test {

inline constexpr const char* kPassword = "Passw0rd2026";

/// Fixed "now" at the start of every test: 2026-09-22 (a Tuesday), well before any deadline.
inline constexpr Timestamp kT0 = 1790000000;

/// In-memory school used by the integration and system tests.
///
///   Departments: Sciences (hod, lecturer, MAT101), Languages (otherHod, otherLecturer, ENG101)
///   Students:    student, student2 enrolled on MAT101; outsider enrolled on nothing
///   Assignment:  "Homework 1" on MAT101, deadline = kT0 + 7 days, max mark 25
class TestSchool {
public:
    explicit TestSchool(std::unique_ptr<ILatePenaltyPolicy> policy =
                            std::make_unique<PercentPerDayPenaltyPolicy>(10.0, 3),
                        std::unique_ptr<IPasswordHasher> hasher = nullptr,
                        const std::string& databasePath = ":memory:");

    std::unique_ptr<User> login(const std::string& username);

    /// Moves the clock to `offset` seconds after the assignment deadline (negative = before).
    void setTimeRelativeToDeadline(std::int64_t offset);

    /// Submits as `who`, marks as lecturer with `raw`, returns submission id.
    int submitAndMark(const User& who, double raw);

    std::unique_ptr<LmsSystem> sys;
    FixedClock* clock = nullptr;

    std::unique_ptr<User> admin;
    std::unique_ptr<User> hod;
    std::unique_ptr<User> otherHod;
    std::unique_ptr<User> lecturer;
    std::unique_ptr<User> otherLecturer;
    std::unique_ptr<User> student;
    std::unique_ptr<User> student2;
    std::unique_ptr<User> outsider;

    int sciences = 0;
    int languages = 0;
    int course = 0;
    int otherCourse = 0;
    int assignment = 0;
    Timestamp deadline = 0;
};

}  // namespace lms::test
