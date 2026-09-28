# Lecturer Manual

Log in as described in the Student manual (option `1`, username, password; the password is not shown). You then see new notifications, such as "Neo Mohapi submitted 'Algebra homework 1' (MAT101) LATE.", and the **Lecturer menu**:

```
1. My courses and assignments
2. Create an assignment
3. View submissions for an assignment
4. Mark or re-mark a submission
5. Grant an individual extension
6. View extensions for an assignment
7. View all my notifications
8. Change my password
0. Log out
```

You only see and work with courses the administrator has assigned to you.

## 2 - Create an assignment
1. Choose the course ID from the list.
2. Type the title and an optional one-line description.
3. Type the deadline as `YYYY-MM-DD HH:MM`, e.g. `2026-10-15 17:00`. It must be in the future.
4. Type the maximum mark (1-1000).

## 3 - View submissions
Choose the assignment ID. The table shows the submission ID, student, submission time, **Timeliness** ("on time" or "n day(s) late", already taking extensions into account), version, status and mark. Type a submission ID to read the work, or press Enter to skip.

## 4 - Mark or re-mark a submission
1. Type the **submission ID**.
2. Type the **raw mark**, the mark the work deserves before any late penalty (0 up to the maximum; decimals allowed, e.g. 17.5).
3. Type your **written feedback**. You can use several lines; finish with a line containing only `.`. Feedback is required.
4. **Reason**: leave empty when marking for the first time. When you change a mark you already entered, you must give a reason (e.g. "Missed question 4 on first pass").

The system calculates the late penalty (10 % of the maximum per started day late, zero after 3 days) and shows, for example: `Marked. Raw 20, final 17.5 (1 day late - 10% of 25 = 2.5 deducted).`

The mark is then sent to your Head of Department for moderation. **Students cannot see it until the HoD releases it.** Once the HoD has moderated a mark you can no longer change it; ask the HoD.

Every mark and change is recorded in the audit log with your name, the time and the reason.

## 5 - Grant an individual extension
1. Choose the assignment ID and type the student's **username**.
2. Type the new deadline (`YYYY-MM-DD HH:MM`); it must be later than the original deadline.
3. Type the reason (e.g. "Medical certificate 12 Oct").

The student is notified and sees "(extended)" next to their deadline. Granting an extension again for the same student replaces the earlier one. If the student had already submitted late but within the new deadline, the work is automatically treated as on time. You cannot grant an extension once the student's mark has been moderated.

## 6 - View extensions
Lists every extension for an assignment: student, new deadline, reason, who granted it.
