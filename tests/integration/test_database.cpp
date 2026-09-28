// IT-DB: SQLite data-access layer - schema, constraints, transactions, injection safety,
// persistence (NFR-06, NFR-07, NFR-09).
#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "TestSchool.h"
#include "lms/common/Errors.h"
#include "lms/data/Database.h"
#include "lms/data/SqliteRepositories.h"

using namespace lms;
using namespace lms::test;

namespace {

TEST(IT_DB, MigrationCreatesLatestSchemaAndIsIdempotent) {
    Database db(":memory:");
    SchemaMigrator::migrate(db);
    EXPECT_EQ(db.userVersion(), SchemaMigrator::latestVersion());
    EXPECT_NO_THROW(SchemaMigrator::migrate(db));
    EXPECT_EQ(db.userVersion(), SchemaMigrator::latestVersion());
    for (const char* table : {"department", "app_user", "course", "course_lecturer", "enrolment",
                              "assignment", "extension", "submission", "submission_version",
                              "mark_audit", "notification", "class_extension"}) {
        Statement st = db.prepare("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name=?");
        st.bind(1, table);
        ASSERT_TRUE(st.step());
        EXPECT_EQ(st.columnInt(0), 1) << table;
    }
}

TEST(IT_DB, RefusesDatabaseFromNewerRelease) {
    Database db(":memory:");
    db.setUserVersion(SchemaMigrator::latestVersion() + 1);
    EXPECT_THROW(SchemaMigrator::migrate(db), DatabaseError);
}

TEST(IT_DB, ForeignKeysAreEnforced) {
    Database db(":memory:");
    SchemaMigrator::migrate(db);
    EXPECT_THROW(db.execute("INSERT INTO course (code, title, department_id) VALUES ('X','Y',999)"),
                 DatabaseError);
}

TEST(IT_DB, InvalidSqlRaisesDatabaseError) {
    Database db(":memory:");
    EXPECT_THROW(db.prepare("SELEC nonsense"), DatabaseError);
    EXPECT_THROW(db.execute("DROP TABLE does_not_exist"), DatabaseError);
}

TEST(IT_DB, AuditLogIsAppendOnly) {
    TestSchool school;
    const int sid = school.submitAndMark(*school.student, 20);
    ASSERT_EQ(school.sys->repositories().audit.listForSubmission(sid).size(), 1u);
    Database& db = school.sys->database();
    EXPECT_THROW(db.execute("UPDATE mark_audit SET new_mark = 25"), DatabaseError);
    EXPECT_THROW(db.execute("DELETE FROM mark_audit"), DatabaseError);
    EXPECT_EQ(school.sys->repositories().audit.listForSubmission(sid).size(), 1u);
}

TEST(IT_DB, SchemaRejectsPlainTextPasswords) {
    TestSchool school;
    EXPECT_THROW(school.sys->database().execute(
                     "UPDATE app_user SET password_hash = 'hunter2' WHERE username = 'student'"),
                 DatabaseError);
}

TEST(IT_DB, SchemaAllowsOnlyOneActiveHeadPerDepartment) {
    TestSchool school;
    UserRecord r;
    r.username = "secondhod";
    r.passwordHash = "$argon2id$placeholder";
    r.fullName = "Second Head";
    r.role = Role::HeadOfDepartment;
    r.departmentId = school.sciences;
    EXPECT_THROW(school.sys->repositories().users.add(r), DatabaseError);
}

TEST(IT_DB, ParameterisedQueriesNeutraliseSqlInjection) {
    TestSchool school;
    // Classic injection payloads used as a username must simply not match.
    for (const char* payload : {"' OR '1'='1", "admin'--", "x'; DROP TABLE app_user; --"}) {
        EXPECT_THROW(school.sys->auth().login(payload, "anything1"), AuthenticationError);
    }
    // The table is intact and text containing quotes round-trips exactly.
    EXPECT_TRUE(school.sys->repositories().users.findByUsername("admin").has_value());
    const int dept = school.sys->admin().createDepartment(*school.admin, "O'Brien's \"Dept\"; --");
    EXPECT_EQ(school.sys->repositories().departments.findById(dept)->name,
              "O'Brien's \"Dept\"; --");
}

TEST(IT_DB, TransactionRollsBackOnException) {
    TestSchool school;
    const auto before = school.sys->repositories().departments.listAll().size();
    EXPECT_THROW(school.sys->database().inTransaction([&] {
        school.sys->repositories().departments.add("Temporary");
        throw ValidationError("abort");
    }),
                 ValidationError);
    EXPECT_EQ(school.sys->repositories().departments.listAll().size(), before);
}

TEST(IT_DB, NestedTransactionsRollBackOnlyInnerWork) {
    TestSchool school;
    Database& db = school.sys->database();
    auto& departments = school.sys->repositories().departments;
    const auto before = departments.listAll().size();
    db.inTransaction([&] {
        departments.add("Outer");
        try {
            db.inTransaction([&] {
                departments.add("Inner");
                throw ValidationError("inner fails");
            });
        } catch (const ValidationError&) {
        }
    });
    EXPECT_EQ(departments.listAll().size(), before + 1);
}

TEST(IT_DB, DataPersistsAcrossRestarts) {
    const std::string path =
        (std::filesystem::temp_directory_path() / "lms_persistence_test.db").string();
    std::filesystem::remove(path);
    std::filesystem::remove(path + "-wal");
    std::filesystem::remove(path + "-shm");
    {
        TestSchool school(std::make_unique<PercentPerDayPenaltyPolicy>(), nullptr, path);
        school.submitAndMark(*school.student, 21);
    }
    {
        auto system = std::make_unique<LmsSystem>(
            path, std::make_unique<FixedClock>(kT0),
            std::make_unique<Argon2idPasswordHasher>(Argon2Params::fastForTesting()),
            std::make_unique<PercentPerDayPenaltyPolicy>());
        auto student = system->auth().login("student", kPassword);
        auto subs = system->repositories().submissions.listForStudent(student->id());
        ASSERT_EQ(subs.size(), 1u);
        EXPECT_DOUBLE_EQ(subs[0].finalMark.value_or(-1), 21.0);
        EXPECT_EQ(system->repositories().audit.listAll().size(), 1u);
    }
    std::filesystem::remove(path);
    std::filesystem::remove(path + "-wal");
    std::filesystem::remove(path + "-shm");
}

// CR1 adaptive part: a database created by release v1.0 (schema 1, with data) must be
// upgraded in place by v1.1 without losing anything.
TEST(IT_DB, UpgradesVersion1DatabaseWithoutDataLoss) {
    const std::string path =
        (std::filesystem::temp_directory_path() / "lms_upgrade_test.db").string();
    for (const char* suffix : {"", "-wal", "-shm"}) {
        std::filesystem::remove(path + suffix);
    }
    {
        Database v1(path);
        std::ifstream file(std::string(LMS_SOURCE_DIR) + "/sql/001_initial_schema.sql");
        ASSERT_TRUE(file.good());
        std::stringstream sql;
        sql << file.rdbuf();
        v1.execute(sql.str());
        v1.setUserVersion(1);
        v1.execute("INSERT INTO department (name) VALUES ('Sciences')");
    }
    {
        LmsSystem upgraded(path, std::make_unique<FixedClock>(kT0),
                           std::make_unique<Argon2idPasswordHasher>(Argon2Params::fastForTesting()),
                           std::make_unique<PercentPerDayPenaltyPolicy>());
        EXPECT_EQ(upgraded.database().userVersion(), 2);
        const auto departments = upgraded.repositories().departments.listAll();
        ASSERT_EQ(departments.size(), 1u);
        EXPECT_EQ(departments[0].name, "Sciences");
        EXPECT_FALSE(upgraded.repositories().assignments.findClassExtension(1).has_value());
    }
    for (const char* suffix : {"", "-wal", "-shm"}) {
        std::filesystem::remove(path + suffix);
    }
}

TEST(IT_DB, OpeningAnInvalidPathFails) {
    EXPECT_THROW(Database("/definitely/not/a/dir/x/y/z/lms.db"), DatabaseError);
}

TEST(IT_DB, RepositoriesReturnEmptyForUnknownIds) {
    TestSchool school;
    Repositories r = school.sys->repositories();
    EXPECT_FALSE(r.users.findById(9999));
    EXPECT_FALSE(r.departments.findById(9999));
    EXPECT_FALSE(r.courses.findById(9999));
    EXPECT_FALSE(r.assignments.findById(9999));
    EXPECT_FALSE(r.submissions.findById(9999));
    EXPECT_FALSE(r.assignments.findExtension(school.assignment, 9999));
    UserRecord ghost;
    ghost.id = 9999;
    ghost.username = "ghost";
    ghost.passwordHash = "$argon2id$x";
    ghost.fullName = "Ghost";
    EXPECT_THROW(r.users.update(ghost), NotFoundError);
    Submission s;
    s.id = 9999;
    EXPECT_THROW(r.submissions.update(s), NotFoundError);
}

}  // namespace
