#pragma once

#include <optional>
#include <string>
#include <vector>

#include "lms/domain/Entities.h"
#include "lms/domain/Role.h"
#include "lms/domain/UserFactory.h"

namespace lms {

// Repository pattern: the service layer depends only on these interfaces
// (Dependency Inversion). The SQLite implementations live in SqliteRepositories.h and
// are the only code in the system that contains SQL.

class IUserRepository {
public:
    virtual ~IUserRepository() = default;
    virtual int add(const UserRecord& user) = 0;
    virtual void update(const UserRecord& user) = 0;
    virtual std::optional<UserRecord> findById(int id) const = 0;
    virtual std::optional<UserRecord> findByUsername(const std::string& username) const = 0;
    virtual std::vector<UserRecord> listAll() const = 0;
    virtual std::vector<UserRecord> listByRole(Role role) const = 0;
    virtual int countByRole(Role role) const = 0;
    virtual std::optional<UserRecord> findHeadOfDepartment(int departmentId) const = 0;
};

class IDepartmentRepository {
public:
    virtual ~IDepartmentRepository() = default;
    virtual int add(const std::string& name) = 0;
    virtual std::optional<Department> findById(int id) const = 0;
    virtual std::vector<Department> listAll() const = 0;
};

class ICourseRepository {
public:
    virtual ~ICourseRepository() = default;
    virtual int add(const Course& course) = 0;
    virtual std::optional<Course> findById(int id) const = 0;
    virtual std::vector<Course> listAll() const = 0;
    virtual std::vector<Course> listForLecturer(int lecturerId) const = 0;
    virtual std::vector<Course> listForStudent(int studentId) const = 0;
    virtual void assignLecturer(int courseId, int lecturerId) = 0;
    virtual void enrol(int courseId, int studentId, Timestamp at) = 0;
    virtual bool isLecturerOf(int courseId, int lecturerId) const = 0;
    virtual bool isEnrolled(int courseId, int studentId) const = 0;
    virtual std::vector<int> lecturerIds(int courseId) const = 0;
    virtual std::vector<int> studentIds(int courseId) const = 0;
};

class IAssignmentRepository {
public:
    virtual ~IAssignmentRepository() = default;
    virtual int add(const Assignment& assignment) = 0;
    virtual std::optional<Assignment> findById(int id) const = 0;
    virtual std::vector<Assignment> listByCourse(int courseId) const = 0;
    /// Inserts or replaces the extension for (assignmentId, studentId).
    virtual void saveExtension(const Extension& extension) = 0;
    virtual std::optional<Extension> findExtension(int assignmentId, int studentId) const = 0;
    virtual std::vector<Extension> listExtensions(int assignmentId) const = 0;
};

class ISubmissionRepository {
public:
    virtual ~ISubmissionRepository() = default;
    virtual int add(const Submission& submission) = 0;
    virtual void update(const Submission& submission) = 0;
    virtual std::optional<Submission> findById(int id) const = 0;
    virtual std::optional<Submission> find(int assignmentId, int studentId) const = 0;
    virtual std::vector<Submission> listByAssignment(int assignmentId) const = 0;
    virtual std::vector<Submission> listForStudent(int studentId) const = 0;
    virtual std::vector<Submission> listByDepartmentAndStatus(int departmentId,
                                                              SubmissionStatus status) const = 0;
    virtual void addVersion(int submissionId, int version, const std::string& content,
                            Timestamp submittedAt) = 0;
    virtual int countVersions(int submissionId) const = 0;
};

class IAuditRepository {
public:
    virtual ~IAuditRepository() = default;
    virtual int add(const MarkAuditEntry& entry) = 0;
    virtual std::vector<MarkAuditEntry> listForSubmission(int submissionId) const = 0;
    virtual std::vector<MarkAuditEntry> listForDepartment(int departmentId) const = 0;
    virtual std::vector<MarkAuditEntry> listAll() const = 0;
};

class INotificationRepository {
public:
    virtual ~INotificationRepository() = default;
    virtual int add(const Notification& notification) = 0;
    virtual std::vector<Notification> listForUser(int userId, bool unreadOnly) const = 0;
    virtual void markAllRead(int userId) = 0;
};

/// Groups the repositories that make up one data store so they can be passed as a unit.
struct Repositories {
    IUserRepository& users;
    IDepartmentRepository& departments;
    ICourseRepository& courses;
    IAssignmentRepository& assignments;
    ISubmissionRepository& submissions;
    IAuditRepository& audit;
    INotificationRepository& notifications;
};

}  // namespace lms
