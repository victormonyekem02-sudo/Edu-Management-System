#pragma once

#include <optional>
#include <set>
#include <string>

#include "lms/domain/Role.h"

namespace lms {

/// Abstract base of the four user roles. Behaviour that differs by role is expressed
/// through virtual functions rather than `if (role == ...)` chains (Open/Closed principle):
/// adding a role means adding a subclass, not editing every service.
class User {
public:
    virtual ~User() = default;
    User(const User&) = delete;
    User& operator=(const User&) = delete;

    int id() const noexcept { return id_; }
    const std::string& username() const noexcept { return username_; }
    const std::string& fullName() const noexcept { return fullName_; }
    std::optional<int> departmentId() const noexcept { return departmentId_; }

    virtual Role role() const noexcept = 0;
    virtual std::string roleTitle() const = 0;

    /// True if the role grants the permission.
    bool can(Permission p) const { return permissions().count(p) > 0; }

    /// Throws AuthorizationError if the role does not grant the permission.
    void require(Permission p) const;

    /// True if this user may see department-level data (moderation queue, audit log)
    /// for the given department. Administrators see all departments, a Head of
    /// Department sees only their own, other roles none.
    virtual bool hasDepartmentScope(int departmentId) const = 0;

protected:
    User(int id, std::string username, std::string fullName, std::optional<int> departmentId);
    virtual const std::set<Permission>& permissions() const noexcept = 0;

private:
    int id_;
    std::string username_;
    std::string fullName_;
    std::optional<int> departmentId_;
};

class Administrator final : public User {
public:
    Administrator(int id, std::string username, std::string fullName);
    Role role() const noexcept override { return Role::Administrator; }
    std::string roleTitle() const override { return "Administrator"; }
    bool hasDepartmentScope(int) const override { return true; }

protected:
    const std::set<Permission>& permissions() const noexcept override;
};

class Lecturer final : public User {
public:
    Lecturer(int id, std::string username, std::string fullName, int departmentId);
    Role role() const noexcept override { return Role::Lecturer; }
    std::string roleTitle() const override { return "Lecturer"; }
    bool hasDepartmentScope(int) const override { return false; }

protected:
    const std::set<Permission>& permissions() const noexcept override;
};

class HeadOfDepartment final : public User {
public:
    HeadOfDepartment(int id, std::string username, std::string fullName, int departmentId);
    Role role() const noexcept override { return Role::HeadOfDepartment; }
    std::string roleTitle() const override { return "Head of Department"; }
    bool hasDepartmentScope(int departmentId) const override;

protected:
    const std::set<Permission>& permissions() const noexcept override;
};

class Student final : public User {
public:
    Student(int id, std::string username, std::string fullName, std::optional<int> departmentId);
    Role role() const noexcept override { return Role::Student; }
    std::string roleTitle() const override { return "Student"; }
    bool hasDepartmentScope(int) const override { return false; }

protected:
    const std::set<Permission>& permissions() const noexcept override;
};

}  // namespace lms
