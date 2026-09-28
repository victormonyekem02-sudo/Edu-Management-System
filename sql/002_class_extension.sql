-- =============================================================================
-- LMS - schema version 2 (release v1.1, change request CR1)
-- Adds class-wide deadline extensions. Additive only: v1.0 data is unchanged.
-- =============================================================================

-- At most one class extension per assignment; granting again replaces it.
CREATE TABLE class_extension (
    assignment_id  INTEGER PRIMARY KEY REFERENCES assignment (assignment_id) ON DELETE CASCADE,
    new_deadline   INTEGER NOT NULL,
    reason         TEXT    NOT NULL CHECK (length(trim(reason)) > 0),
    granted_by     INTEGER NOT NULL REFERENCES app_user (user_id),
    granted_at     INTEGER NOT NULL
);
