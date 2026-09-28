#pragma once

#include "lms/data/Database.h"
#include "lms/data/Repositories.h"

namespace lms {

class SqliteUserRepository final : public IUserRepository {
public:
    explicit SqliteUserRepository(Database& db) : db_(db) {}
    int add(const UserRecord& user) override;
    void update(const UserRecord& user) override;
    std::optional<UserRecord> findById(int id) const override;
    std::optional<UserRecord> findByUsername(const std::string& username) const override;
    std::vector<UserRecord> listAll() const override;
    std::vector<UserRecord> listByRole(Role role) const override;
    int countByRole(Role role) const override;
    std::optional<UserRecord> findHeadOfDepartment(int departmentId) const override;

private:
    Database& db_;
};

class SqliteDepartmentRepository final : public IDepartmentRepository {
public:
    explicit SqliteDepartmentRepository(Database& db) : db_(db) {}
    int add(const std::string& name) override;
    std::optional<Department> findById(int id) const override;
    std::vector<Department> listAll() const override;

private:
    Database& db_;
};

class SqliteCourseRepository final : public ICourseRepository {
public:
    explicit SqliteCourseRepository(Database& db) : db_(db) {}
    int add(const Course& course) override;
    std::optional<Course> findById(int id) const override;
    std::vector<Course> listAll() const override;
    std::vector<Course> listForLecturer(int lecturerId) const override;
    std::vector<Course> listForStudent(int studentId) const override;
    void assignLecturer(int courseId, int lecturerId) override;
    void enrol(int courseId, int studentId, Timestamp at) override;
    bool isLecturerOf(int courseId, int lecturerId) const override;
    bool isEnrolled(int courseId, int studentId) const override;
    std::vector<int> lecturerIds(int courseId) const override;
    std::vector<int> studentIds(int courseId) const override;

private:
    Database& db_;
};

class SqliteAssignmentRepository final : public IAssignmentRepository {
public:
    explicit SqliteAssignmentRepository(Database& db) : db_(db) {}
    int add(const Assignment& assignment) override;
    std::optional<Assignment> findById(int id) const override;
    std::vector<Assignment> listByCourse(int courseId) const override;
    void saveExtension(const Extension& extension) override;
    std::optional<Extension> findExtension(int assignmentId, int studentId) const override;
    std::vector<Extension> listExtensions(int assignmentId) const override;

private:
    Database& db_;
};

class SqliteSubmissionRepository final : public ISubmissionRepository {
public:
    explicit SqliteSubmissionRepository(Database& db) : db_(db) {}
    int add(const Submission& submission) override;
    void update(const Submission& submission) override;
    std::optional<Submission> findById(int id) const override;
    std::optional<Submission> find(int assignmentId, int studentId) const override;
    std::vector<Submission> listByAssignment(int assignmentId) const override;
    std::vector<Submission> listForStudent(int studentId) const override;
    std::vector<Submission> listByDepartmentAndStatus(int departmentId,
                                                      SubmissionStatus status) const override;
    void addVersion(int submissionId, int version, const std::string& content,
                    Timestamp submittedAt) override;
    int countVersions(int submissionId) const override;

private:
    Database& db_;
};

class SqliteAuditRepository final : public IAuditRepository {
public:
    explicit SqliteAuditRepository(Database& db) : db_(db) {}
    int add(const MarkAuditEntry& entry) override;
    std::vector<MarkAuditEntry> listForSubmission(int submissionId) const override;
    std::vector<MarkAuditEntry> listForDepartment(int departmentId) const override;
    std::vector<MarkAuditEntry> listAll() const override;

private:
    Database& db_;
};

class SqliteNotificationRepository final : public INotificationRepository {
public:
    explicit SqliteNotificationRepository(Database& db) : db_(db) {}
    int add(const Notification& notification) override;
    std::vector<Notification> listForUser(int userId, bool unreadOnly) const override;
    void markAllRead(int userId) override;

private:
    Database& db_;
};

/// Owns one instance of every SQLite repository for a Database connection.
class SqliteStore {
public:
    explicit SqliteStore(Database& db);
    Repositories repositories();
    ITransactionRunner& transactions() { return tx_; }

private:
    SqliteUserRepository users_;
    SqliteDepartmentRepository departments_;
    SqliteCourseRepository courses_;
    SqliteAssignmentRepository assignments_;
    SqliteSubmissionRepository submissions_;
    SqliteAuditRepository audit_;
    SqliteNotificationRepository notifications_;
    SqliteTransactionRunner tx_;
};

}  // namespace lms
