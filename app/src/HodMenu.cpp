#include "lms/common/TextUtil.h"
#include "lms/common/TimeUtil.h"
#include "lms/ui/Menus.h"

namespace lms::ui {

std::vector<MenuItem> HodMenu::items() {
    return {
        {"Marks awaiting moderation", [this] { showQueue(true); }},
        {"Approve a mark", [this] { approve(); }},
        {"Adjust a mark", [this] { adjust(); }},
        {"Moderated marks ready for release", [this] { showQueue(false); }},
        {"Release moderated marks for an assignment", [this] { release(); }},
        {"Audit trail of one submission", [this] { submissionAudit(); }},
        {"Department mark audit log", [this] { departmentAudit(); }},
    };
}

void HodMenu::showQueue(bool pending) {
    const auto items = pending ? sys_.moderation().pendingModeration(user_)
                               : sys_.moderation().readyForRelease(user_);
    std::vector<std::vector<std::string>> rows;
    for (const ModerationItem& i : items) {
        const Submission& s = i.submission;
        rows.push_back({std::to_string(s.id), i.course.code,
                        std::to_string(i.assignment.id) + " " + i.assignment.title, i.studentName,
                        s.rawMark ? text::formatMark(*s.rawMark) : "-",
                        s.daysLate > 0 ? "-" + text::formatMark(s.lateDeduction) : "0",
                        s.finalMark ? text::formatMark(*s.finalMark) + "/" +
                                          std::to_string(i.assignment.maxMark)
                                    : "-",
                        i.markerName});
    }
    io_.table({"Subm. ID", "Course", "Assignment", "Student", "Raw", "Late", "Final", "Marker"},
              rows);
}

void HodMenu::approve() {
    showQueue(true);
    const int id = io_.readInt("Submission ID to approve", 1, 1000000000);
    const std::string note = io_.readLine("Moderation note (optional)");
    sys_.moderation().approve(user_, id, note);
    io_.success("Mark approved. Release it when the whole assignment is moderated.");
}

void HodMenu::adjust() {
    showQueue(true);
    const int id = io_.readInt("Submission ID to adjust", 1, 1000000000);
    const double mark = io_.readDouble("New final mark", 0.0, 1000.0);
    const std::string reason = io_.readRequired("Reason for the adjustment (recorded in the audit log)");
    sys_.moderation().adjust(user_, id, mark, reason);
    io_.success("Mark adjusted and recorded in the audit log.");
}

void HodMenu::release() {
    showQueue(false);
    const int assignmentId = io_.readInt("Assignment ID to release", 1, 1000000000);
    if (!io_.confirm("Release all moderated marks for this assignment to students?")) {
        io_.info("Nothing released.");
        return;
    }
    const ReleaseSummary r = sys_.moderation().releaseAssignment(user_, assignmentId);
    io_.success(std::to_string(r.released) + " mark(s) released.");
    if (r.awaitingModeration > 0 || r.notYetMarked > 0) {
        io_.info(std::to_string(r.awaitingModeration) + " still await moderation and " +
                 std::to_string(r.notYetMarked) +
                 " are not marked yet; they remain hidden from students.");
    }
}

void HodMenu::submissionAudit() {
    const int id = io_.readInt("Submission ID", 1, 1000000000);
    printAudit(sys_.moderation().auditTrail(user_, id));
}

void HodMenu::departmentAudit() {
    printAudit(sys_.moderation().auditLog(user_));
}

}  // namespace lms::ui
