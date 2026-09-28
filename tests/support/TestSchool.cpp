#include "TestSchool.h"

namespace lms::test {

TestSchool::TestSchool(std::unique_ptr<ILatePenaltyPolicy> policy,
                       std::unique_ptr<IPasswordHasher> hasher, const std::string& databasePath) {
    auto fixedClock = std::make_unique<FixedClock>(kT0);
    clock = fixedClock.get();
    if (!hasher) {
        hasher = std::make_unique<Argon2idPasswordHasher>(Argon2Params::fastForTesting());
    }
    sys = std::make_unique<LmsSystem>(databasePath, std::move(fixedClock), std::move(hasher),
                                      std::move(policy));

    AdminService& a = sys->admin();
    a.bootstrapAdministrator("admin", "Admin User", kPassword);
    admin = login("admin");

    sciences = a.createDepartment(*admin, "Sciences");
    languages = a.createDepartment(*admin, "Languages");
    a.createUser(*admin, Role::HeadOfDepartment, "hod", "Head Sciences", kPassword, sciences);
    a.createUser(*admin, Role::HeadOfDepartment, "otherhod", "Head Languages", kPassword, languages);
    const int lect = a.createUser(*admin, Role::Lecturer, "lecturer", "Lecturer Sciences",
                                  kPassword, sciences);
    const int otherLect = a.createUser(*admin, Role::Lecturer, "otherlecturer",
                                       "Lecturer Languages", kPassword, languages);
    const int s1 = a.createUser(*admin, Role::Student, "student", "Student One", kPassword,
                                std::nullopt);
    const int s2 = a.createUser(*admin, Role::Student, "student2", "Student Two", kPassword,
                                std::nullopt);
    a.createUser(*admin, Role::Student, "outsider", "Not Enrolled", kPassword, std::nullopt);

    course = a.createCourse(*admin, "MAT101", "Mathematics", sciences);
    otherCourse = a.createCourse(*admin, "ENG101", "English", languages);
    a.assignLecturer(*admin, course, lect);
    a.assignLecturer(*admin, otherCourse, otherLect);
    a.enrolStudent(*admin, course, s1);
    a.enrolStudent(*admin, course, s2);

    hod = login("hod");
    otherHod = login("otherhod");
    lecturer = login("lecturer");
    otherLecturer = login("otherlecturer");
    student = login("student");
    student2 = login("student2");
    outsider = login("outsider");

    deadline = kT0 + 7 * kSecondsPerDay;
    assignment = sys->assignments().createAssignment(*lecturer, course, "Homework 1",
                                                     "Questions 1-5", deadline, 25);
}

std::unique_ptr<User> TestSchool::login(const std::string& username) {
    return sys->auth().login(username, kPassword);
}

void TestSchool::setTimeRelativeToDeadline(std::int64_t offset) {
    clock->set(deadline + offset);
}

int TestSchool::submitAndMark(const User& who, double raw) {
    const SubmitResult r = sys->submissions().submit(who, assignment, "My answers");
    sys->marking().mark(*lecturer, r.submissionId, raw, "Good work");
    return r.submissionId;
}

}  // namespace lms::test
