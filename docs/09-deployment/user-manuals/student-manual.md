# Student Manual

## Logging in
1. Double-click **School LMS** on the desktop.
2. Type `1` and press Enter to log in.
3. Type your username and your password. Your password is **not shown** while you type; this is normal.
4. If you type the wrong password 5 times, your account is locked for 15 minutes. Ask the administrator if you forgot your password.

After logging in you see any **new notifications** (for example "You have been granted an extension" or "Your mark ... has been released"), then the **Student menu**. Choose an option by typing its number and pressing Enter.

```
1. My courses
2. My assignments and deadlines
3. Submit or resubmit work
4. My results and feedback
5. View all my notifications
6. Change my password
0. Log out
```

**Always log out (0) when you finish**, so the next person cannot use your account.

## 2 - My assignments and deadlines
Shows every assignment of your courses, sorted by deadline:

| Column | Meaning |
|--------|---------|
| ID | Number you type when submitting |
| Your deadline | Your personal deadline. "(extended)" means your lecturer gave you more time |
| Time | Time left, or "deadline passed" |
| Status | Not submitted, Submitted, Submitted late, Marked (awaiting moderation), Moderated (awaiting release), Released. "(v2)" means you have resubmitted once |

## 3 - Submit or resubmit work
1. Type the assignment **ID**.
2. Type your answer. You can use several lines. When you finish, type a line with only a full stop `.` and press Enter.
   - To submit a file instead, type `@` followed by the file's full path, e.g. `@E:\Maths\homework1.txt`, then a line with `.`. Files may be up to 100 KB.
3. The system tells you the result:
   - **"Submitted on time"** - you may still change your work until the deadline by submitting again.
   - **"Resubmitted (version 2)"** - your new work replaces the old one (the old one is kept on record).
   - **"Submitted LATE"** - your work is accepted but a late penalty will apply.

### Late penalty rule
You lose **10 % of the maximum mark for each started day** after your deadline:

| Submitted | Days late | Example: maximum 25, you earned 20 |
|-----------|-----------|------------------------------------|
| before or exactly at the deadline | 0 | 20 |
| 1 second to 24 hours late | 1 | 20 - 2.5 = 17.5 |
| 24 h + 1 s to 48 hours late | 2 | 15 |
| 48 h + 1 s to 72 hours late | 3 | 12.5 |
| more than 72 hours late | 4+ | 0 (you still receive written feedback) |

After the deadline you **cannot** change work you already submitted. If you have a good reason for needing more time (illness, family emergency), speak to your lecturer **before** the deadline if possible. An extension moves your deadline and removes the penalty.

## 4 - My results and feedback
You see your mark and your lecturer's written feedback only **after the Head of Department has checked and released the marks**. Until then the status is "Awaiting marking/moderation". Late deductions are shown next to the mark, e.g. `17.5/25 (raw 20, 1 day(s) late, -2.5)`.

## 6 - Change my password
Type your current password, then the new one twice. Passwords need at least 8 characters with letters and at least one digit, and must not be the same as your username.

## Accessibility
The program uses plain text only, with no colours or pictures, so it works with screen readers (e.g. NVDA or Windows Narrator) and with the Windows magnifier. You can make the text bigger: right-click the window's title bar > *Properties* > *Font*.
