#pragma once

#include <optional>
#include <string>

#include "lms/common/Clock.h"
#include "lms/domain/SubmissionStatus.h"

namespace lms {

struct Department {
    int id = 0;
    std::string name;
};

struct Course {
    int id = 0;
    std::string code;
    std::string title;
    int departmentId = 0;
};

struct Assignment {
    int id = 0;
    int courseId = 0;
    std::string title;
    std::string description;
    Timestamp deadline = 0;
    int maxMark = 0;
    int createdBy = 0;
    Timestamp createdAt = 0;
};

/// Individual deadline extension for one student on one assignment.
struct Extension {
    int id = 0;
    int assignmentId = 0;
    int studentId = 0;
    Timestamp newDeadline = 0;
    std::string reason;
    int grantedBy = 0;
    Timestamp grantedAt = 0;
};

/// Deadline extension for every student enrolled on the assignment's course
/// (release v1.1, change request CR1). At most one per assignment.
struct ClassExtension {
    int assignmentId = 0;
    Timestamp newDeadline = 0;
    std::string reason;
    int grantedBy = 0;
    Timestamp grantedAt = 0;
};

struct Submission {
    int id = 0;
    int assignmentId = 0;
    int studentId = 0;
    std::string content;
    Timestamp submittedAt = 0;
    int version = 1;
    SubmissionStatus status = SubmissionStatus::Submitted;
    std::optional<double> rawMark;
    int daysLate = 0;
    double lateDeduction = 0.0;
    std::optional<double> finalMark;
    std::string feedback;
    std::optional<int> markedBy;
    std::optional<Timestamp> markedAt;
    std::optional<int> moderatedBy;
    std::optional<Timestamp> moderatedAt;
    std::optional<int> releasedBy;
    std::optional<Timestamp> releasedAt;
};

enum class AuditAction { Mark, Remark, ModerationApprove, ModerationAdjust, Release };

std::string auditActionToCode(AuditAction a);
AuditAction auditActionFromCode(const std::string& code);

/// One immutable row of the mark audit trail: who changed a mark, when, from what, to what, why.
struct MarkAuditEntry {
    int id = 0;
    int submissionId = 0;
    int changedBy = 0;
    Timestamp changedAt = 0;
    AuditAction action = AuditAction::Mark;
    std::optional<double> oldMark;
    std::optional<double> newMark;
    std::string reason;
};

struct Notification {
    int id = 0;
    int userId = 0;
    std::string message;
    Timestamp createdAt = 0;
    bool read = false;
};

}  // namespace lms
