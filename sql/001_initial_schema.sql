-- =============================================================================
-- Moderated LMS - schema version 1 (release v1.0)
-- Normalised to 3NF. All timestamps are INTEGER seconds since the Unix epoch (UTC).
-- This file is embedded into the executable at build time (lib/CMakeLists.txt)
-- and applied by SchemaMigrator; it is the single source of truth for the schema.
-- =============================================================================

CREATE TABLE department (
    department_id  INTEGER PRIMARY KEY,
    name           TEXT    NOT NULL UNIQUE COLLATE NOCASE
                           CHECK (length(trim(name)) BETWEEN 1 AND 100)
);

CREATE TABLE app_user (
    user_id                INTEGER PRIMARY KEY,
    username               TEXT    NOT NULL UNIQUE COLLATE NOCASE
                                   CHECK (length(username) BETWEEN 3 AND 32),
    password_hash          TEXT    NOT NULL CHECK (password_hash LIKE '$argon2id$%'),
    full_name              TEXT    NOT NULL CHECK (length(trim(full_name)) BETWEEN 1 AND 100),
    role                   TEXT    NOT NULL CHECK (role IN ('ADMIN', 'LECTURER', 'HOD', 'STUDENT')),
    department_id          INTEGER REFERENCES department (department_id) ON DELETE RESTRICT,
    is_active              INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0, 1)),
    failed_login_attempts  INTEGER NOT NULL DEFAULT 0 CHECK (failed_login_attempts >= 0),
    locked_until           INTEGER NOT NULL DEFAULT 0,
    created_at             INTEGER NOT NULL,
    CHECK (role NOT IN ('HOD', 'LECTURER') OR department_id IS NOT NULL)
);
-- BR-12: a department has at most one active Head of Department.
CREATE UNIQUE INDEX ux_app_user_one_hod_per_department
    ON app_user (department_id) WHERE role = 'HOD' AND is_active = 1;
CREATE INDEX ix_app_user_role ON app_user (role);

CREATE TABLE course (
    course_id      INTEGER PRIMARY KEY,
    code           TEXT    NOT NULL UNIQUE COLLATE NOCASE CHECK (length(trim(code)) BETWEEN 1 AND 20),
    title          TEXT    NOT NULL CHECK (length(trim(title)) BETWEEN 1 AND 100),
    department_id  INTEGER NOT NULL REFERENCES department (department_id) ON DELETE RESTRICT
);
CREATE INDEX ix_course_department ON course (department_id);

CREATE TABLE course_lecturer (
    course_id    INTEGER NOT NULL REFERENCES course (course_id) ON DELETE CASCADE,
    lecturer_id  INTEGER NOT NULL REFERENCES app_user (user_id) ON DELETE RESTRICT,
    PRIMARY KEY (course_id, lecturer_id)
);
CREATE INDEX ix_course_lecturer_lecturer ON course_lecturer (lecturer_id);

CREATE TABLE enrolment (
    course_id    INTEGER NOT NULL REFERENCES course (course_id) ON DELETE CASCADE,
    student_id   INTEGER NOT NULL REFERENCES app_user (user_id) ON DELETE RESTRICT,
    enrolled_at  INTEGER NOT NULL,
    PRIMARY KEY (course_id, student_id)
);
CREATE INDEX ix_enrolment_student ON enrolment (student_id);

CREATE TABLE assignment (
    assignment_id  INTEGER PRIMARY KEY,
    course_id      INTEGER NOT NULL REFERENCES course (course_id) ON DELETE RESTRICT,
    title          TEXT    NOT NULL CHECK (length(trim(title)) BETWEEN 1 AND 200),
    description    TEXT    NOT NULL DEFAULT '',
    deadline       INTEGER NOT NULL,
    max_mark       INTEGER NOT NULL CHECK (max_mark BETWEEN 1 AND 1000),
    created_by     INTEGER NOT NULL REFERENCES app_user (user_id),
    created_at     INTEGER NOT NULL
);
CREATE INDEX ix_assignment_course ON assignment (course_id);

-- Individual extension granted by a lecturer to one student (BR-03).
CREATE TABLE extension (
    extension_id   INTEGER PRIMARY KEY,
    assignment_id  INTEGER NOT NULL REFERENCES assignment (assignment_id) ON DELETE CASCADE,
    student_id     INTEGER NOT NULL REFERENCES app_user (user_id),
    new_deadline   INTEGER NOT NULL,
    reason         TEXT    NOT NULL CHECK (length(trim(reason)) > 0),
    granted_by     INTEGER NOT NULL REFERENCES app_user (user_id),
    granted_at     INTEGER NOT NULL,
    UNIQUE (assignment_id, student_id)
);

-- Current (latest) submission of a student for an assignment.
CREATE TABLE submission (
    submission_id   INTEGER PRIMARY KEY,
    assignment_id   INTEGER NOT NULL REFERENCES assignment (assignment_id) ON DELETE RESTRICT,
    student_id      INTEGER NOT NULL REFERENCES app_user (user_id),
    content         TEXT    NOT NULL,
    submitted_at    INTEGER NOT NULL,
    version         INTEGER NOT NULL DEFAULT 1 CHECK (version >= 1),
    status          TEXT    NOT NULL
                    CHECK (status IN ('SUBMITTED', 'LATE', 'MARKED', 'MODERATED', 'RELEASED')),
    raw_mark        REAL    CHECK (raw_mark IS NULL OR raw_mark >= 0),
    days_late       INTEGER NOT NULL DEFAULT 0 CHECK (days_late >= 0),
    late_deduction  REAL    NOT NULL DEFAULT 0 CHECK (late_deduction >= 0),
    final_mark      REAL    CHECK (final_mark IS NULL OR final_mark >= 0),
    feedback        TEXT,
    marked_by       INTEGER REFERENCES app_user (user_id),
    marked_at       INTEGER,
    moderated_by    INTEGER REFERENCES app_user (user_id),
    moderated_at    INTEGER,
    released_by     INTEGER REFERENCES app_user (user_id),
    released_at     INTEGER,
    UNIQUE (assignment_id, student_id),
    CHECK (status IN ('SUBMITTED', 'LATE') OR (raw_mark IS NOT NULL AND final_mark IS NOT NULL))
);
CREATE INDEX ix_submission_student ON submission (student_id);
CREATE INDEX ix_submission_status ON submission (status);

-- Every version a student has submitted (resubmission history, BR-05).
CREATE TABLE submission_version (
    submission_id  INTEGER NOT NULL REFERENCES submission (submission_id) ON DELETE CASCADE,
    version        INTEGER NOT NULL CHECK (version >= 1),
    content        TEXT    NOT NULL,
    submitted_at   INTEGER NOT NULL,
    PRIMARY KEY (submission_id, version)
);

-- Append-only audit trail of every mark change (FR-15, NFR-07).
CREATE TABLE mark_audit (
    audit_id       INTEGER PRIMARY KEY,
    submission_id  INTEGER NOT NULL REFERENCES submission (submission_id) ON DELETE RESTRICT,
    changed_by     INTEGER NOT NULL REFERENCES app_user (user_id),
    changed_at     INTEGER NOT NULL,
    action         TEXT    NOT NULL CHECK (action IN
                   ('MARK', 'REMARK', 'MODERATION_APPROVE', 'MODERATION_ADJUST', 'RELEASE')),
    old_mark       REAL,
    new_mark       REAL,
    reason         TEXT    NOT NULL CHECK (length(trim(reason)) > 0)
);
CREATE INDEX ix_mark_audit_submission ON mark_audit (submission_id);

CREATE TRIGGER trg_mark_audit_no_update BEFORE UPDATE ON mark_audit
BEGIN
    SELECT RAISE(ABORT, 'mark_audit is append-only');
END;

CREATE TRIGGER trg_mark_audit_no_delete BEFORE DELETE ON mark_audit
BEGIN
    SELECT RAISE(ABORT, 'mark_audit is append-only');
END;

CREATE TABLE notification (
    notification_id  INTEGER PRIMARY KEY,
    user_id          INTEGER NOT NULL REFERENCES app_user (user_id) ON DELETE CASCADE,
    message          TEXT    NOT NULL,
    created_at       INTEGER NOT NULL,
    is_read          INTEGER NOT NULL DEFAULT 0 CHECK (is_read IN (0, 1))
);
CREATE INDEX ix_notification_user ON notification (user_id, is_read);
