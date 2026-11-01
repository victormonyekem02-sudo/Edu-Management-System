# Administrator Manual

The administrator prepares the school in the system at the start of each term and helps users with their accounts. The first administrator account is created the first time the program starts (see the installation guide).

```
1. List user accounts
2. Create user account
3. Activate / deactivate an account
4. Reset a user's password
5. List departments
6. Create department
7. List courses
8. Create course
9. Assign lecturer to course
10. Enrol student on course
11. View mark audit log (whole school)
12. View all my notifications
13. Change my password
0. Log out
```

## Setting up a new term (in this order)
1. **Create departments** (6), e.g. "Sciences", "Languages". Names must be unique.
2. **Create user accounts** (2):
   - Choose the role: 1 Administrator, 2 Lecturer, 3 Head of Department, 4 Student.
   - Username: 3-32 characters, letters, digits, `.`, `_`, `-` (e.g. `lmokoena`, `s2026-001`). Must be unique.
   - Lecturers and Heads of Department **must** belong to a department; students may (press Enter for none).
   - Each department can have only **one** active Head of Department.
   - Initial password: at least 8 characters with a letter and a digit. Give it to the user privately and ask them to change it (their menu option "Change my password").
3. **Create courses** (8) with a unique code (e.g. `MAT101`), a title and the department.
4. **Assign lecturers** (9). A lecturer can only teach courses of their own department.
5. **Enrol students** (10) by course ID and student username.

## Helping users
- **Forgotten password or locked account:** option 4, "Reset a user's password". This also removes the 15-minute lock.
- **Someone leaves the school:** option 3, answer `n` to deactivate. The account can no longer log in, but all their submissions, marks and audit records are kept. You cannot deactivate your own account.
- **New Head of Department:** deactivate the old HoD first, then create or activate the new one.

## Audit log (11)
Shows every mark change in the whole school. Use it when the principal needs to investigate a complaint.

## Good practice
- Create one administrator account per person; do not share accounts.
- Make sure the technician takes the daily backup of `lms.db`.
- At the end of the year, deactivate accounts of students who have left and archive their records, following the school's data retention rules (see the professional issues section of the project report).
