#include "lms/data/SqliteRepositories.h"

#include "lms/common/Errors.h"

namespace lms {

namespace {

int lastId(const Database& db) {
    return static_cast<int>(db.lastInsertId());
}

// ---- row mappers -------------------------------------------------------------

const char* const kUserColumns =
    "user_id, username, password_hash, full_name, role, department_id, is_active, "
    "failed_login_attempts, locked_until, created_at";

UserRecord readUser(const Statement& st) {
    UserRecord u;
    u.id = st.columnInt(0);
    u.username = st.columnText(1);
    u.passwordHash = st.columnText(2);
    u.fullName = st.columnText(3);
    u.role = roleFromCode(st.columnText(4));
    u.departmentId = st.columnOptionalInt(5);
    u.active = st.columnBool(6);
    u.failedLoginAttempts = st.columnInt(7);
    u.lockedUntil = st.columnInt64(8);
    u.createdAt = st.columnInt64(9);
    return u;
}

Course readCourse(const Statement& st) {
    return Course{st.columnInt(0), st.columnText(1), st.columnText(2), st.columnInt(3)};
}

const char* const kAssignmentColumns =
    "a.assignment_id, a.course_id, a.title, a.description, a.deadline, a.max_mark, "
    "a.created_by, a.created_at";

Assignment readAssignment(const Statement& st) {
    Assignment a;
    a.id = st.columnInt(0);
    a.courseId = st.columnInt(1);
    a.title = st.columnText(2);
    a.description = st.columnText(3);
    a.deadline = st.columnInt64(4);
    a.maxMark = st.columnInt(5);
    a.createdBy = st.columnInt(6);
    a.createdAt = st.columnInt64(7);
    return a;
}

Extension readExtension(const Statement& st) {
    Extension e;
    e.id = st.columnInt(0);
    e.assignmentId = st.columnInt(1);
    e.studentId = st.columnInt(2);
    e.newDeadline = st.columnInt64(3);
    e.reason = st.columnText(4);
    e.grantedBy = st.columnInt(5);
    e.grantedAt = st.columnInt64(6);
    return e;
}

const char* const kSubmissionColumns =
    "s.submission_id, s.assignment_id, s.student_id, s.content, s.submitted_at, s.version, "
    "s.status, s.raw_mark, s.days_late, s.late_deduction, s.final_mark, s.feedback, "
    "s.marked_by, s.marked_at, s.moderated_by, s.moderated_at, s.released_by, s.released_at";

Submission readSubmission(const Statement& st) {
    Submission s;
    s.id = st.columnInt(0);
    s.assignmentId = st.columnInt(1);
    s.studentId = st.columnInt(2);
    s.content = st.columnText(3);
    s.submittedAt = st.columnInt64(4);
    s.version = st.columnInt(5);
    s.status = statusFromCode(st.columnText(6));
    s.rawMark = st.columnOptionalDouble(7);
    s.daysLate = st.columnInt(8);
    s.lateDeduction = st.columnDouble(9);
    s.finalMark = st.columnOptionalDouble(10);
    s.feedback = st.columnText(11);
    s.markedBy = st.columnOptionalInt(12);
    s.markedAt = st.columnOptionalInt64(13);
    s.moderatedBy = st.columnOptionalInt(14);
    s.moderatedAt = st.columnOptionalInt64(15);
    s.releasedBy = st.columnOptionalInt(16);
    s.releasedAt = st.columnOptionalInt64(17);
    return s;
}

const char* const kAuditColumns =
    "m.audit_id, m.submission_id, m.changed_by, m.changed_at, m.action, m.old_mark, "
    "m.new_mark, m.reason";

MarkAuditEntry readAudit(const Statement& st) {
    MarkAuditEntry e;
    e.id = st.columnInt(0);
    e.submissionId = st.columnInt(1);
    e.changedBy = st.columnInt(2);
    e.changedAt = st.columnInt64(3);
    e.action = auditActionFromCode(st.columnText(4));
    e.oldMark = st.columnOptionalDouble(5);
    e.newMark = st.columnOptionalDouble(6);
    e.reason = st.columnText(7);
    return e;
}

template <typename T, typename Mapper>
std::vector<T> readAll(Statement& st, Mapper map) {
    std::vector<T> rows;
    while (st.step()) {
        rows.push_back(map(st));
    }
    return rows;
}

std::vector<int> readIds(Statement& st) {
    std::vector<int> ids;
    while (st.step()) {
        ids.push_back(st.columnInt(0));
    }
    return ids;
}

}  // namespace

// ---- Users -------------------------------------------------------------------

int SqliteUserRepository::add(const UserRecord& u) {
    db_.prepare(
           "INSERT INTO app_user (username, password_hash, full_name, role, department_id, "
           "is_active, failed_login_attempts, locked_until, created_at) "
           "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)")
        .bind(1, u.username)
        .bind(2, u.passwordHash)
        .bind(3, u.fullName)
        .bind(4, roleToCode(u.role))
        .bind(5, u.departmentId)
        .bind(6, u.active)
        .bind(7, u.failedLoginAttempts)
        .bind(8, u.lockedUntil)
        .bind(9, u.createdAt)
        .run();
    return lastId(db_);
}

void SqliteUserRepository::update(const UserRecord& u) {
    db_.prepare(
           "UPDATE app_user SET username = ?, password_hash = ?, full_name = ?, role = ?, "
           "department_id = ?, is_active = ?, failed_login_attempts = ?, locked_until = ? "
           "WHERE user_id = ?")
        .bind(1, u.username)
        .bind(2, u.passwordHash)
        .bind(3, u.fullName)
        .bind(4, roleToCode(u.role))
        .bind(5, u.departmentId)
        .bind(6, u.active)
        .bind(7, u.failedLoginAttempts)
        .bind(8, u.lockedUntil)
        .bind(9, u.id)
        .run();
    if (db_.changes() == 0) {
        throw NotFoundError("User " + std::to_string(u.id) + " does not exist.");
    }
}

std::optional<UserRecord> SqliteUserRepository::findById(int id) const {
    Statement st =
        db_.prepare(std::string("SELECT ") + kUserColumns + " FROM app_user WHERE user_id = ?");
    st.bind(1, id);
    if (st.step()) {
        return readUser(st);
    }
    return std::nullopt;
}

std::optional<UserRecord> SqliteUserRepository::findByUsername(const std::string& username) const {
    Statement st =
        db_.prepare(std::string("SELECT ") + kUserColumns + " FROM app_user WHERE username = ?");
    st.bind(1, username);
    if (st.step()) {
        return readUser(st);
    }
    return std::nullopt;
}

std::vector<UserRecord> SqliteUserRepository::listAll() const {
    Statement st = db_.prepare(std::string("SELECT ") + kUserColumns +
                               " FROM app_user ORDER BY role, username");
    return readAll<UserRecord>(st, readUser);
}

std::vector<UserRecord> SqliteUserRepository::listByRole(Role role) const {
    Statement st = db_.prepare(std::string("SELECT ") + kUserColumns +
                               " FROM app_user WHERE role = ? ORDER BY username");
    st.bind(1, roleToCode(role));
    return readAll<UserRecord>(st, readUser);
}

int SqliteUserRepository::countByRole(Role role) const {
    Statement st =
        db_.prepare("SELECT COUNT(*) FROM app_user WHERE role = ? AND is_active = 1");
    st.bind(1, roleToCode(role));
    return st.step() ? st.columnInt(0) : 0;
}

std::optional<UserRecord> SqliteUserRepository::findHeadOfDepartment(int departmentId) const {
    Statement st = db_.prepare(std::string("SELECT ") + kUserColumns +
                               " FROM app_user WHERE role = 'HOD' AND is_active = 1 "
                               "AND department_id = ?");
    st.bind(1, departmentId);
    if (st.step()) {
        return readUser(st);
    }
    return std::nullopt;
}

// ---- Departments -------------------------------------------------------------

int SqliteDepartmentRepository::add(const std::string& name) {
    db_.prepare("INSERT INTO department (name) VALUES (?)").bind(1, name).run();
    return lastId(db_);
}

std::optional<Department> SqliteDepartmentRepository::findById(int id) const {
    Statement st = db_.prepare("SELECT department_id, name FROM department WHERE department_id = ?");
    st.bind(1, id);
    if (st.step()) {
        return Department{st.columnInt(0), st.columnText(1)};
    }
    return std::nullopt;
}

std::vector<Department> SqliteDepartmentRepository::listAll() const {
    Statement st = db_.prepare("SELECT department_id, name FROM department ORDER BY name");
    return readAll<Department>(st, [](const Statement& s) {
        return Department{s.columnInt(0), s.columnText(1)};
    });
}

// ---- Courses -----------------------------------------------------------------

int SqliteCourseRepository::add(const Course& c) {
    db_.prepare("INSERT INTO course (code, title, department_id) VALUES (?, ?, ?)")
        .bind(1, c.code)
        .bind(2, c.title)
        .bind(3, c.departmentId)
        .run();
    return lastId(db_);
}

std::optional<Course> SqliteCourseRepository::findById(int id) const {
    Statement st = db_.prepare(
        "SELECT course_id, code, title, department_id FROM course WHERE course_id = ?");
    st.bind(1, id);
    if (st.step()) {
        return readCourse(st);
    }
    return std::nullopt;
}

std::vector<Course> SqliteCourseRepository::listAll() const {
    Statement st =
        db_.prepare("SELECT course_id, code, title, department_id FROM course ORDER BY code");
    return readAll<Course>(st, readCourse);
}

std::vector<Course> SqliteCourseRepository::listForLecturer(int lecturerId) const {
    Statement st = db_.prepare(
        "SELECT c.course_id, c.code, c.title, c.department_id FROM course c "
        "JOIN course_lecturer cl ON cl.course_id = c.course_id "
        "WHERE cl.lecturer_id = ? ORDER BY c.code");
    st.bind(1, lecturerId);
    return readAll<Course>(st, readCourse);
}

std::vector<Course> SqliteCourseRepository::listForStudent(int studentId) const {
    Statement st = db_.prepare(
        "SELECT c.course_id, c.code, c.title, c.department_id FROM course c "
        "JOIN enrolment e ON e.course_id = c.course_id "
        "WHERE e.student_id = ? ORDER BY c.code");
    st.bind(1, studentId);
    return readAll<Course>(st, readCourse);
}

void SqliteCourseRepository::assignLecturer(int courseId, int lecturerId) {
    db_.prepare("INSERT OR IGNORE INTO course_lecturer (course_id, lecturer_id) VALUES (?, ?)")
        .bind(1, courseId)
        .bind(2, lecturerId)
        .run();
}

void SqliteCourseRepository::enrol(int courseId, int studentId, Timestamp at) {
    db_.prepare(
           "INSERT OR IGNORE INTO enrolment (course_id, student_id, enrolled_at) VALUES (?, ?, ?)")
        .bind(1, courseId)
        .bind(2, studentId)
        .bind(3, at)
        .run();
}

bool SqliteCourseRepository::isLecturerOf(int courseId, int lecturerId) const {
    Statement st = db_.prepare(
        "SELECT 1 FROM course_lecturer WHERE course_id = ? AND lecturer_id = ?");
    st.bind(1, courseId).bind(2, lecturerId);
    return st.step();
}

bool SqliteCourseRepository::isEnrolled(int courseId, int studentId) const {
    Statement st =
        db_.prepare("SELECT 1 FROM enrolment WHERE course_id = ? AND student_id = ?");
    st.bind(1, courseId).bind(2, studentId);
    return st.step();
}

std::vector<int> SqliteCourseRepository::lecturerIds(int courseId) const {
    Statement st = db_.prepare("SELECT lecturer_id FROM course_lecturer WHERE course_id = ?");
    st.bind(1, courseId);
    return readIds(st);
}

std::vector<int> SqliteCourseRepository::studentIds(int courseId) const {
    Statement st = db_.prepare(
        "SELECT e.student_id FROM enrolment e JOIN app_user u ON u.user_id = e.student_id "
        "WHERE e.course_id = ? ORDER BY u.username");
    st.bind(1, courseId);
    return readIds(st);
}

// ---- Assignments and extensions ----------------------------------------------

int SqliteAssignmentRepository::add(const Assignment& a) {
    db_.prepare(
           "INSERT INTO assignment (course_id, title, description, deadline, max_mark, "
           "created_by, created_at) VALUES (?, ?, ?, ?, ?, ?, ?)")
        .bind(1, a.courseId)
        .bind(2, a.title)
        .bind(3, a.description)
        .bind(4, a.deadline)
        .bind(5, a.maxMark)
        .bind(6, a.createdBy)
        .bind(7, a.createdAt)
        .run();
    return lastId(db_);
}

std::optional<Assignment> SqliteAssignmentRepository::findById(int id) const {
    Statement st = db_.prepare(std::string("SELECT ") + kAssignmentColumns +
                               " FROM assignment a WHERE a.assignment_id = ?");
    st.bind(1, id);
    if (st.step()) {
        return readAssignment(st);
    }
    return std::nullopt;
}

std::vector<Assignment> SqliteAssignmentRepository::listByCourse(int courseId) const {
    Statement st = db_.prepare(std::string("SELECT ") + kAssignmentColumns +
                               " FROM assignment a WHERE a.course_id = ? ORDER BY a.deadline");
    st.bind(1, courseId);
    return readAll<Assignment>(st, readAssignment);
}

void SqliteAssignmentRepository::saveExtension(const Extension& e) {
    db_.prepare(
           "INSERT INTO extension (assignment_id, student_id, new_deadline, reason, granted_by, "
           "granted_at) VALUES (?, ?, ?, ?, ?, ?) "
           "ON CONFLICT (assignment_id, student_id) DO UPDATE SET "
           "new_deadline = excluded.new_deadline, reason = excluded.reason, "
           "granted_by = excluded.granted_by, granted_at = excluded.granted_at")
        .bind(1, e.assignmentId)
        .bind(2, e.studentId)
        .bind(3, e.newDeadline)
        .bind(4, e.reason)
        .bind(5, e.grantedBy)
        .bind(6, e.grantedAt)
        .run();
}

std::optional<Extension> SqliteAssignmentRepository::findExtension(int assignmentId,
                                                                   int studentId) const {
    Statement st = db_.prepare(
        "SELECT extension_id, assignment_id, student_id, new_deadline, reason, granted_by, "
        "granted_at FROM extension WHERE assignment_id = ? AND student_id = ?");
    st.bind(1, assignmentId).bind(2, studentId);
    if (st.step()) {
        return readExtension(st);
    }
    return std::nullopt;
}

std::vector<Extension> SqliteAssignmentRepository::listExtensions(int assignmentId) const {
    Statement st = db_.prepare(
        "SELECT extension_id, assignment_id, student_id, new_deadline, reason, granted_by, "
        "granted_at FROM extension WHERE assignment_id = ? ORDER BY student_id");
    st.bind(1, assignmentId);
    return readAll<Extension>(st, readExtension);
}

void SqliteAssignmentRepository::saveClassExtension(const ClassExtension& e) {
    db_.prepare(
           "INSERT INTO class_extension (assignment_id, new_deadline, reason, granted_by, "
           "granted_at) VALUES (?, ?, ?, ?, ?) "
           "ON CONFLICT (assignment_id) DO UPDATE SET new_deadline = excluded.new_deadline, "
           "reason = excluded.reason, granted_by = excluded.granted_by, "
           "granted_at = excluded.granted_at")
        .bind(1, e.assignmentId)
        .bind(2, e.newDeadline)
        .bind(3, e.reason)
        .bind(4, e.grantedBy)
        .bind(5, e.grantedAt)
        .run();
}

std::optional<ClassExtension> SqliteAssignmentRepository::findClassExtension(
    int assignmentId) const {
    Statement st = db_.prepare(
        "SELECT assignment_id, new_deadline, reason, granted_by, granted_at "
        "FROM class_extension WHERE assignment_id = ?");
    st.bind(1, assignmentId);
    if (st.step()) {
        return ClassExtension{st.columnInt(0), st.columnInt64(1), st.columnText(2),
                              st.columnInt(3), st.columnInt64(4)};
    }
    return std::nullopt;
}

// ---- Submissions -------------------------------------------------------------

int SqliteSubmissionRepository::add(const Submission& s) {
    db_.prepare(
           "INSERT INTO submission (assignment_id, student_id, content, submitted_at, version, "
           "status) VALUES (?, ?, ?, ?, ?, ?)")
        .bind(1, s.assignmentId)
        .bind(2, s.studentId)
        .bind(3, s.content)
        .bind(4, s.submittedAt)
        .bind(5, s.version)
        .bind(6, statusToCode(s.status))
        .run();
    return lastId(db_);
}

void SqliteSubmissionRepository::update(const Submission& s) {
    db_.prepare(
           "UPDATE submission SET content = ?, submitted_at = ?, version = ?, status = ?, "
           "raw_mark = ?, days_late = ?, late_deduction = ?, final_mark = ?, feedback = ?, "
           "marked_by = ?, marked_at = ?, moderated_by = ?, moderated_at = ?, released_by = ?, "
           "released_at = ? WHERE submission_id = ?")
        .bind(1, s.content)
        .bind(2, s.submittedAt)
        .bind(3, s.version)
        .bind(4, statusToCode(s.status))
        .bind(5, s.rawMark)
        .bind(6, s.daysLate)
        .bind(7, s.lateDeduction)
        .bind(8, s.finalMark)
        .bind(9, s.feedback)
        .bind(10, s.markedBy)
        .bind(11, s.markedAt)
        .bind(12, s.moderatedBy)
        .bind(13, s.moderatedAt)
        .bind(14, s.releasedBy)
        .bind(15, s.releasedAt)
        .bind(16, s.id)
        .run();
    if (db_.changes() == 0) {
        throw NotFoundError("Submission " + std::to_string(s.id) + " does not exist.");
    }
}

std::optional<Submission> SqliteSubmissionRepository::findById(int id) const {
    Statement st = db_.prepare(std::string("SELECT ") + kSubmissionColumns +
                               " FROM submission s WHERE s.submission_id = ?");
    st.bind(1, id);
    if (st.step()) {
        return readSubmission(st);
    }
    return std::nullopt;
}

std::optional<Submission> SqliteSubmissionRepository::find(int assignmentId, int studentId) const {
    Statement st = db_.prepare(std::string("SELECT ") + kSubmissionColumns +
                               " FROM submission s WHERE s.assignment_id = ? AND s.student_id = ?");
    st.bind(1, assignmentId).bind(2, studentId);
    if (st.step()) {
        return readSubmission(st);
    }
    return std::nullopt;
}

std::vector<Submission> SqliteSubmissionRepository::listByAssignment(int assignmentId) const {
    Statement st = db_.prepare(std::string("SELECT ") + kSubmissionColumns +
                               " FROM submission s WHERE s.assignment_id = ? "
                               "ORDER BY s.submitted_at");
    st.bind(1, assignmentId);
    return readAll<Submission>(st, readSubmission);
}

std::vector<Submission> SqliteSubmissionRepository::listForStudent(int studentId) const {
    Statement st = db_.prepare(std::string("SELECT ") + kSubmissionColumns +
                               " FROM submission s WHERE s.student_id = ? "
                               "ORDER BY s.submitted_at");
    st.bind(1, studentId);
    return readAll<Submission>(st, readSubmission);
}

std::vector<Submission> SqliteSubmissionRepository::listByDepartmentAndStatus(
    int departmentId, SubmissionStatus status) const {
    Statement st = db_.prepare(std::string("SELECT ") + kSubmissionColumns +
                               " FROM submission s "
                               "JOIN assignment a ON a.assignment_id = s.assignment_id "
                               "JOIN course c ON c.course_id = a.course_id "
                               "WHERE c.department_id = ? AND s.status = ? "
                               "ORDER BY s.marked_at, s.submission_id");
    st.bind(1, departmentId).bind(2, statusToCode(status));
    return readAll<Submission>(st, readSubmission);
}

void SqliteSubmissionRepository::addVersion(int submissionId, int version,
                                            const std::string& content, Timestamp submittedAt) {
    db_.prepare(
           "INSERT INTO submission_version (submission_id, version, content, submitted_at) "
           "VALUES (?, ?, ?, ?)")
        .bind(1, submissionId)
        .bind(2, version)
        .bind(3, content)
        .bind(4, submittedAt)
        .run();
}

int SqliteSubmissionRepository::countVersions(int submissionId) const {
    Statement st =
        db_.prepare("SELECT COUNT(*) FROM submission_version WHERE submission_id = ?");
    st.bind(1, submissionId);
    return st.step() ? st.columnInt(0) : 0;
}

// ---- Audit -------------------------------------------------------------------

int SqliteAuditRepository::add(const MarkAuditEntry& e) {
    db_.prepare(
           "INSERT INTO mark_audit (submission_id, changed_by, changed_at, action, old_mark, "
           "new_mark, reason) VALUES (?, ?, ?, ?, ?, ?, ?)")
        .bind(1, e.submissionId)
        .bind(2, e.changedBy)
        .bind(3, e.changedAt)
        .bind(4, auditActionToCode(e.action))
        .bind(5, e.oldMark)
        .bind(6, e.newMark)
        .bind(7, e.reason)
        .run();
    return lastId(db_);
}

std::vector<MarkAuditEntry> SqliteAuditRepository::listForSubmission(int submissionId) const {
    Statement st = db_.prepare(std::string("SELECT ") + kAuditColumns +
                               " FROM mark_audit m WHERE m.submission_id = ? "
                               "ORDER BY m.changed_at, m.audit_id");
    st.bind(1, submissionId);
    return readAll<MarkAuditEntry>(st, readAudit);
}

std::vector<MarkAuditEntry> SqliteAuditRepository::listForDepartment(int departmentId) const {
    Statement st = db_.prepare(std::string("SELECT ") + kAuditColumns +
                               " FROM mark_audit m "
                               "JOIN submission s ON s.submission_id = m.submission_id "
                               "JOIN assignment a ON a.assignment_id = s.assignment_id "
                               "JOIN course c ON c.course_id = a.course_id "
                               "WHERE c.department_id = ? ORDER BY m.changed_at, m.audit_id");
    st.bind(1, departmentId);
    return readAll<MarkAuditEntry>(st, readAudit);
}

std::vector<MarkAuditEntry> SqliteAuditRepository::listAll() const {
    Statement st = db_.prepare(std::string("SELECT ") + kAuditColumns +
                               " FROM mark_audit m ORDER BY m.changed_at, m.audit_id");
    return readAll<MarkAuditEntry>(st, readAudit);
}

// ---- Notifications -----------------------------------------------------------

int SqliteNotificationRepository::add(const Notification& n) {
    db_.prepare("INSERT INTO notification (user_id, message, created_at, is_read) "
                "VALUES (?, ?, ?, ?)")
        .bind(1, n.userId)
        .bind(2, n.message)
        .bind(3, n.createdAt)
        .bind(4, n.read)
        .run();
    return lastId(db_);
}

std::vector<Notification> SqliteNotificationRepository::listForUser(int userId,
                                                                    bool unreadOnly) const {
    Statement st = db_.prepare(
        "SELECT notification_id, user_id, message, created_at, is_read FROM notification "
        "WHERE user_id = ? AND (? = 0 OR is_read = 0) ORDER BY created_at, notification_id");
    st.bind(1, userId).bind(2, unreadOnly);
    return readAll<Notification>(st, [](const Statement& s) {
        return Notification{s.columnInt(0), s.columnInt(1), s.columnText(2), s.columnInt64(3),
                            s.columnBool(4)};
    });
}

void SqliteNotificationRepository::markAllRead(int userId) {
    db_.prepare("UPDATE notification SET is_read = 1 WHERE user_id = ? AND is_read = 0")
        .bind(1, userId)
        .run();
}

// ---- Store -------------------------------------------------------------------

SqliteStore::SqliteStore(Database& db)
    : users_(db),
      departments_(db),
      courses_(db),
      assignments_(db),
      submissions_(db),
      audit_(db),
      notifications_(db),
      tx_(db) {}

Repositories SqliteStore::repositories() {
    return Repositories{users_, departments_, courses_, assignments_,
                        submissions_, audit_, notifications_};
}

}  // namespace lms
