#include "lms/common/Errors.h"
#include "lms/common/TextUtil.h"
#include "lms/common/TimeUtil.h"
#include "lms/ui/Menus.h"

namespace lms::ui {

std::vector<MenuItem> LecturerMenu::items() {
    return {
        {"My courses and assignments", [this] { listCoursesAndAssignments(); }},
        {"Create an assignment", [this] { createAssignment(); }},
        {"View submissions for an assignment", [this] { viewSubmissions(); }},
        {"Mark or re-mark a submission", [this] { markSubmission(); }},
        {"Grant an individual extension", [this] { grantExtension(); }},
        {"View extensions for an assignment", [this] { viewExtensions(); }},
    };
}

void LecturerMenu::listCoursesAndAssignments() {
    std::vector<std::vector<std::string>> rows;
    for (const Course& c : sys_.assignments().coursesTaughtBy(user_)) {
        const auto assignments = sys_.assignments().listForCourse(user_, c.id);
        if (assignments.empty()) {
            rows.push_back({c.code, "-", "(no assignments yet)", "", ""});
        }
        for (const Assignment& a : assignments) {
            rows.push_back({c.code, std::to_string(a.id), a.title,
                            time_util::formatLocal(a.deadline), std::to_string(a.maxMark)});
        }
    }
    io_.table({"Course", "Assign. ID", "Title", "Deadline", "Max mark"}, rows);
}

int LecturerMenu::chooseOwnCourse() {
    std::vector<std::vector<std::string>> rows;
    const auto courses = sys_.assignments().coursesTaughtBy(user_);
    if (courses.empty()) {
        throw BusinessRuleError("You are not assigned to any course yet. Ask the administrator.");
    }
    for (const Course& c : courses) {
        rows.push_back({std::to_string(c.id), c.code, c.title});
    }
    io_.table({"ID", "Code", "Course"}, rows);
    return io_.readInt("Course ID", 1, 1000000);
}

void LecturerMenu::createAssignment() {
    const int course = chooseOwnCourse();
    const std::string title = io_.readRequired("Title");
    const std::string description = io_.readLine("Description (optional)");
    const Timestamp deadline = io_.readDateTime("Deadline");
    const int maxMark = io_.readInt("Maximum mark", 1, 1000);
    const int id =
        sys_.assignments().createAssignment(user_, course, title, description, deadline, maxMark);
    io_.success("Assignment created with ID " + std::to_string(id) + ".");
}

void LecturerMenu::viewSubmissions() {
    listCoursesAndAssignments();
    const int assignmentId = io_.readInt("Assignment ID", 1, 1000000000);
    const auto items = sys_.marking().listForAssignment(user_, assignmentId);
    std::vector<std::vector<std::string>> rows;
    for (const MarkingItem& item : items) {
        const Submission& s = item.submission;
        const int daysLate = LatenessCalculator::daysLate(s.submittedAt, item.effectiveDeadline);
        rows.push_back({std::to_string(s.id), item.studentName, time_util::formatLocal(s.submittedAt),
                        daysLate > 0 ? std::to_string(daysLate) + " day(s) late" : "on time",
                        "v" + std::to_string(s.version), statusDisplayName(s.status),
                        s.finalMark ? text::formatMark(*s.finalMark) : "-"});
    }
    io_.table({"Subm. ID", "Student", "Submitted", "Timeliness", "Ver.", "Status", "Mark"}, rows);
    const auto view = io_.readOptionalInt("Submission ID to read (blank to skip)", 1, 1000000000);
    if (view) {
        for (const MarkingItem& item : items) {
            if (item.submission.id == *view) {
                io_.heading("Submission " + std::to_string(*view) + " by " + item.studentName);
                io_.println(item.submission.content);
                if (!item.submission.feedback.empty()) {
                    io_.println("--- Feedback ---");
                    io_.println(item.submission.feedback);
                }
                return;
            }
        }
        io_.error("That submission is not in the list.");
    }
}

void LecturerMenu::markSubmission() {
    const int submissionId = io_.readInt("Submission ID", 1, 1000000000);
    const double raw = io_.readDouble("Raw mark (before any late penalty)", 0.0, 1000.0);
    const std::string feedback = io_.readMultiline("Written feedback for the student");
    const std::string reason =
        io_.readLine("Reason for change (required only when re-marking; blank otherwise)");
    const PenaltyResult r = sys_.marking().mark(user_, submissionId, raw, feedback, reason);
    io_.success("Marked. Raw " + text::formatMark(r.rawMark) + ", final " +
                text::formatMark(r.finalMark) + " (" + r.explanation + ").");
    io_.info("The mark is hidden from the student until the Head of Department releases it.");
}

void LecturerMenu::grantExtension() {
    listCoursesAndAssignments();
    const int assignmentId = io_.readInt("Assignment ID", 1, 1000000000);
    const std::string username = io_.readRequired("Student username");
    const auto student = sys_.directory().findUserByUsername(username);
    if (!student) {
        throw NotFoundError("No user called '" + username + "'.");
    }
    const Timestamp deadline = io_.readDateTime("New deadline for this student");
    const std::string reason = io_.readRequired("Reason");
    sys_.assignments().grantExtension(user_, assignmentId, student->id, deadline, reason);
    io_.success("Extension granted to " + student->fullName + " until " +
                time_util::formatLocal(deadline) + ".");
}

void LecturerMenu::viewExtensions() {
    listCoursesAndAssignments();
    const int assignmentId = io_.readInt("Assignment ID", 1, 1000000000);
    std::vector<std::vector<std::string>> rows;
    for (const Extension& e : sys_.assignments().listExtensions(user_, assignmentId)) {
        rows.push_back({userName(e.studentId), time_util::formatLocal(e.newDeadline), e.reason,
                        userName(e.grantedBy)});
    }
    io_.table({"Student", "New deadline", "Reason", "Granted by"}, rows);
}

}  // namespace lms::ui
