"""Acceptance tests AT-01..AT-05: drive the real lms executable with scripted keyboard input
against a fresh demonstration database and save each transcript as evidence.

usage: python tests/acceptance/run_acceptance.py <path-to-lms-executable> <output-dir>
The script exits with status 1 if any expected text is missing from a transcript.
"""
import datetime
import os
import subprocess
import sys
import tempfile

PW = "Passw0rd2026"


def login(user, password=PW):
    return ["1", user, password]


def logout():
    return ["0"]


def run(exe, db, lines, seed=False):
    args = [exe, "--db", db] + (["--seed-demo"] if seed else [])
    result = subprocess.run(args, input="\n".join(lines) + "\n", capture_output=True,
                            text=True, timeout=120)
    return result.returncode, result.stdout + result.stderr


def scenarios():
    in3days = (datetime.datetime.now() + datetime.timedelta(days=3)).strftime("%Y-%m-%d %H:%M")
    return [
        ("AT-01", "IT officer: first run on an empty database, offline, creates administrator",
         False,
         ["root", "School Admin", "RootPass2026", "RootPass2026"] + login("root", "RootPass2026")
         + ["1"] + logout() + ["0"],
         ["First run", "[OK] Administrator created.", "Administrator menu", "Goodbye."]),
        ("AT-02/03", "Principal + HoD: late work is penalised and no mark is visible before release",
         True,
         # student submits the past-deadline assignment (late)
         login("stud1") + ["3", "1", "x = 4, y = 7", "."] + ["4"] + logout()
         # lecturer marks it
         + login("lect.sci") + ["4", "1", "20", "Correct method; show your working.", ".", ""]
         + logout()
         # student still sees nothing
         + login("stud1") + ["4"] + logout()
         # HoD approves and releases, then inspects the audit trail
         + login("hod.sci") + ["1", "2", "1", "Checked against memo", "5", "1", "y", "6", "1"]
         + logout()
         # student now sees the moderated mark and feedback
         + login("stud1") + ["4"] + logout() + ["0"],
         ["[OK] Submitted LATE.", "Not released yet", "awaits moderation", "[OK] Mark approved.",
          "[OK] 1 mark(s) released.", "MODERATION_APPROVE", "RELEASE",
          "have been released", "12.5/25", "show your working"]),
        ("AT-04", "Student representative: resubmit before deadline, late work accepted",
         True,
         login("stud2") + ["3", "2", "first draft", ".", "3", "2", "corrected answer", ".",
                           "3", "1", "late but done", ".", "2"] + logout() + ["0"],
         ["Submitted on time", "[OK] Resubmitted (version 2)", "[OK] Submitted LATE.",
          "Submitted (v2)"]),
        ("AT-05", "Lecturer: individual extension removes the late status for that student only",
         True,
         login("lect.sci") + ["5", "1", "stud3", in3days, "Hospital letter"] + logout()
         + login("stud3") + ["2", "3", "1", "my answers", "."] + logout()
         + login("stud4") + ["3", "1", "my answers", "."] + logout() + ["0"],
         ["[OK] Extension granted to Kamohelo Tau", "granted an extension", "individual",
          "Submitted on time", "[OK] Submitted LATE."]),
        ("AT-07", "CR1: lecturer extends the deadline for the whole class",
         True,
         login("lect.sci") + ["6", "1", in3days, "Sports day clash", "7", "1"] + logout()
         + login("stud1") + ["2", "3", "1", "on time now", "."] + logout()
         + login("stud2") + ["2"] + logout() + ["0"],
         ["[OK] Deadline extended for the whole class", "Class extension: until",
          "extended for the whole class", "class", "Submitted on time"]),
    ]


def main():
    exe, out_dir = sys.argv[1], sys.argv[2]
    os.makedirs(out_dir, exist_ok=True)
    failures = 0
    summary = []
    for at_id, title, seed, lines, expected in scenarios():
        with tempfile.TemporaryDirectory() as tmp:
            db = os.path.join(tmp, "acceptance.db")
            code, output = run(exe, db, lines, seed)
        missing = [e for e in expected if e not in output]
        verdict = "PASS" if code == 0 and not missing else "FAIL"
        failures += verdict == "FAIL"
        name = at_id.replace("/", "_")
        with open(os.path.join(out_dir, name + ".txt"), "w", encoding="utf-8") as f:
            f.write("%s - %s\nResult: %s\nExit code: %d\nMissing expectations: %s\n\n"
                    "----- transcript -----\n%s" % (at_id, title, verdict, code, missing, output))
        summary.append("%-9s %-4s %s" % (at_id, verdict, title))
    with open(os.path.join(out_dir, "summary.txt"), "w", encoding="utf-8") as f:
        f.write("\n".join(summary) + "\n")
    print("\n".join(summary))
    sys.exit(1 if failures else 0)


if __name__ == "__main__":
    main()
