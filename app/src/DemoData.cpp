#include <array>
#include <utility>

#include "lms/ui/Application.h"

namespace lms::ui {

bool seedDemoData(LmsSystem& system) {
    Repositories repos = system.repositories();
    if (!repos.users.listAll().empty()) {
        return false;
    }
    AdminService& admin = system.admin();
    const Timestamp now = system.clock().now();

    admin.bootstrapAdministrator("admin", "System Administrator", kDemoPassword);
    std::unique_ptr<User> adminUser = system.auth().login("admin", kDemoPassword);

    system.database().inTransaction([&] {
        const int sciences = admin.createDepartment(*adminUser, "Sciences");
        const int languages = admin.createDepartment(*adminUser, "Languages");

        admin.createUser(*adminUser, Role::HeadOfDepartment, "hod.sci", "Palesa Mokoena",
                         kDemoPassword, sciences);
        admin.createUser(*adminUser, Role::HeadOfDepartment, "hod.lang", "Thabo Letsie",
                         kDemoPassword, languages);
        const int lectSci = admin.createUser(*adminUser, Role::Lecturer, "lect.sci",
                                             "Lineo Ramatla", kDemoPassword, sciences);
        const int lectLang = admin.createUser(*adminUser, Role::Lecturer, "lect.lang",
                                              "Mpho Sello", kDemoPassword, languages);

        const int maths = admin.createCourse(*adminUser, "MAT101", "Mathematics Form A", sciences);
        const int physics = admin.createCourse(*adminUser, "PHY101", "Physical Science", sciences);
        const int english = admin.createCourse(*adminUser, "ENG101", "English Language", languages);
        admin.assignLecturer(*adminUser, maths, lectSci);
        admin.assignLecturer(*adminUser, physics, lectSci);
        admin.assignLecturer(*adminUser, english, lectLang);

        const std::array<std::pair<const char*, const char*>, 5> students{{
            {"stud1", "Neo Mohapi"},
            {"stud2", "Lerato Nthane"},
            {"stud3", "Kamohelo Tau"},
            {"stud4", "Rethabile Phiri"},
            {"stud5", "Tumelo Khabo"},
        }};
        for (const auto& [username, fullName] : students) {
            const int id = admin.createUser(*adminUser, Role::Student, username, fullName,
                                            kDemoPassword, std::nullopt);
            admin.enrolStudent(*adminUser, maths, id);
            admin.enrolStudent(*adminUser, english, id);
        }

        // Assignments are inserted through the repository so that one of them can have a
        // deadline in the past (the service rightly refuses that), which lets the late-penalty
        // rules be demonstrated immediately.
        const auto addAssignment = [&](int course, int lecturer, const char* title,
                                       const char* description, Timestamp deadline, int maxMark) {
            Assignment a;
            a.courseId = course;
            a.title = title;
            a.description = description;
            a.deadline = deadline;
            a.maxMark = maxMark;
            a.createdBy = lecturer;
            a.createdAt = now;
            return repos.assignments.add(a);
        };
        addAssignment(maths, lectSci, "Algebra homework 1", "Exercises 1-10, page 42.",
                      now - (2 * kSecondsPerDay), 25);
        addAssignment(maths, lectSci, "Geometry project", "Construct and measure triangles.",
                      now + (7 * kSecondsPerDay), 50);
        addAssignment(english, lectLang, "Essay: My community", "300 to 400 words.",
                      now + (3 * kSecondsPerDay), 40);
    });
    return true;
}

}  // namespace lms::ui
