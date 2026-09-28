#include <filesystem>
#include <fstream>
#include <sstream>

#include "lms/common/Errors.h"
#include "lms/common/TextUtil.h"
#include "lms/common/TimeUtil.h"
#include "lms/ui/Menus.h"

namespace lms::ui {

namespace {

/// Reads a file the student wants to submit (answer typed as "@path"). The size is checked
/// before reading, and the LMS database itself can never be submitted (review finding R3).
std::string readSubmissionFile(const std::string& path, const std::string& databasePath) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::is_regular_file(path, ec)) {
        throw ValidationError("'" + path + "' is not a file that can be submitted.");
    }
    if (databasePath != ":memory:" && fs::exists(databasePath, ec) &&
        fs::equivalent(path, databasePath, ec)) {
        throw ValidationError("That file cannot be submitted.");
    }
    const auto size = fs::file_size(path, ec);
    if (ec || size > SubmissionService::kMaxContentLength) {
        throw ValidationError("The file is too large (maximum " +
                              std::to_string(SubmissionService::kMaxContentLength / 1000) +
                              " KB).");
    }
    const std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw ValidationError("Cannot open the file '" + path + "'.");
    }
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

}  // namespace

std::vector<MenuItem> StudentMenu::items() {
    return {
        {"My courses", [this] { listCourses(); }},
        {"My assignments and deadlines", [this] { listAssignments(); }},
        {"Submit or resubmit work", [this] { submitWork(); }},
        {"My results and feedback", [this] { viewResults(); }},
    };
}

void StudentMenu::listCourses() {
    std::vector<std::vector<std::string>> rows;
    for (const Course& c : sys_.assignments().coursesOf(user_)) {
        std::string lecturers;
        for (const UserSummary& l : sys_.directory().lecturersOf(c.id)) {
            lecturers += (lecturers.empty() ? "" : ", ") + l.fullName;
        }
        rows.push_back({c.code, c.title, lecturers.empty() ? "-" : lecturers});
    }
    io_.table({"Code", "Course", "Lecturer(s)"}, rows);
}

void StudentMenu::listAssignments() {
    const Timestamp now = sys_.clock().now();
    std::vector<std::vector<std::string>> rows;
    for (const StudentAssignmentView& v : sys_.assignments().listForStudent(user_)) {
        std::string deadline = time_util::formatLocal(v.effectiveDeadline);
        if (v.extended) {
            deadline += " (extended)";
        }
        const std::int64_t remaining = v.effectiveDeadline - now;
        const std::string timing = remaining >= 0
                                       ? time_util::describeDuration(remaining) + " left"
                                       : "deadline passed";
        rows.push_back({std::to_string(v.assignment.id), v.course.code, v.assignment.title,
                        deadline, timing, std::to_string(v.assignment.maxMark),
                        v.status ? statusDisplayName(*v.status) +
                                       (v.version > 1 ? " (v" + std::to_string(v.version) + ")" : "")
                                 : "Not submitted"});
    }
    io_.table({"ID", "Course", "Assignment", "Your deadline", "Time", "Max", "Status"}, rows);
}

void StudentMenu::submitWork() {
    listAssignments();
    const int assignmentId = io_.readInt("Assignment ID", 1, 1000000000);
    io_.info("Type your answer, or type @ followed by a file path (e.g. @C:\\work\\essay.txt).");
    std::string content = io_.readMultiline("Your work");
    const std::string trimmed = text::trim(content);
    if (!trimmed.empty() && trimmed.front() == '@' && trimmed.find('\n') == std::string::npos) {
        content = readSubmissionFile(text::trim(trimmed.substr(1)), sys_.databasePath());
    }
    const SubmitResult r = sys_.submissions().submit(user_, assignmentId, content);
    if (r.resubmission) {
        io_.success("Resubmitted (version " + std::to_string(r.version) +
                    "). Your previous version has been replaced.");
    } else if (r.late) {
        io_.success("Submitted LATE. The deadline was " +
                    time_util::formatLocal(r.effectiveDeadline) +
                    ". Late penalty: " + sys_.marking().policy().description() +
                    ". Your work will still be marked and receive feedback.");
    } else {
        io_.success("Submitted on time. You may resubmit until " +
                    time_util::formatLocal(r.effectiveDeadline) + ".");
    }
}

void StudentMenu::viewResults() {
    std::vector<std::vector<std::string>> rows;
    const auto results = sys_.submissions().resultsFor(user_);
    for (const StudentResultView& r : results) {
        std::string mark = "Not released yet";
        if (r.released && r.finalMark) {
            mark = text::formatMark(*r.finalMark) + "/" + std::to_string(r.assignment.maxMark);
            if (r.daysLate > 0) {
                mark += " (raw " + text::formatMark(r.rawMark.value_or(0.0)) + ", " +
                        std::to_string(r.daysLate) + " day(s) late, -" +
                        text::formatMark(r.lateDeduction) + ")";
            }
        }
        rows.push_back({r.course.code, r.assignment.title,
                        r.released ? "Released" : "Awaiting marking/moderation", mark});
    }
    io_.table({"Course", "Assignment", "Status", "Mark"}, rows);
    for (const StudentResultView& r : results) {
        if (r.released && !r.feedback.empty()) {
            io_.heading("Feedback: " + r.assignment.title);
            io_.println(r.feedback);
        }
    }
}

}  // namespace lms::ui
