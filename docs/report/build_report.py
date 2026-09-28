"""Builds docs/report/final-report.pdf from final-report.md plus the appendices.

usage: python docs/report/build_report.py [path-to-msedge.exe]
Requires: pip install markdown. The PDF is printed with a headless Chromium browser
(Microsoft Edge or Google Chrome).
"""
import io
import os
import re
import subprocess
import sys

import markdown

HERE = os.path.dirname(os.path.abspath(__file__))
DOCS = os.path.dirname(HERE)


def read(rel):
    return io.open(os.path.join(DOCS, rel), encoding="utf-8").read()


def demote(md, levels):
    """Shift every Markdown heading down by `levels` (max h6)."""
    def repl(m):
        return "#" * min(6, len(m.group(1)) + levels) + " "
    return re.sub(r"^(#{1,6}) ", repl, md, flags=re.M)


def section(md, start, end=None):
    i = md.index(start)
    j = md.index(end, i + 1) if end else len(md)
    return md[i:j]


def count_body_words(md):
    """Words of sections 2-12, excluding tables, figures, captions, code blocks and headings."""
    body = md[md.index("## 2. Introduction"):md.index("## 13. References")]
    body = re.sub(r"```.*?```", "", body, flags=re.S)
    words = 0
    for line in body.splitlines():
        s = line.strip()
        if not s or s.startswith(("|", "![", "#", "*Table", "*Figure", "<")):
            continue
        words += len(re.findall(r"[A-Za-z0-9][A-Za-z0-9'%.,/-]*", s))
    return words


def appendices():
    uc = read("04-requirements/use-cases.md")
    parts = [
        ("A", "Requirements traceability matrix", read("appendices/traceability-matrix.md")),
        ("B", "Test results: test report v1.0, defect log, CR1 regression report",
         read("07-testing/test-report.md") + "\n\n" + read("07-testing/defect-log.md") + "\n\n"
         + read("10-maintenance/regression-test-report.md")),
        ("C", "User manuals",
         "\n\n".join(read("09-deployment/user-manuals/%s-manual.md" % r)
                     for r in ("admin", "lecturer", "hod", "student"))),
        ("D", "Updated risk register", read("03-project-management/risk-register.md")),
        ("E", "Fully dressed use cases UC-02 and UC-03",
         section(uc, "### UC-02 Mark submission", "## 4. Brief descriptions")),
    ]
    out = ["\n<div class=\"page-break\"></div>\n\n## 14. Appendices\n"]
    for letter, title, md in parts:
        # drop the first document's H1 (the appendix heading replaces it); the H1 titles of
        # further concatenated documents become sub-headings below the table-of-contents depth
        md = re.sub(r"^# .*\n", "", md, count=1, flags=re.M)
        md = re.sub(r"^# ", "## ", md, flags=re.M)
        out.append("\n<div class=\"page-break\"></div>\n\n### Appendix %s - %s\n\n%s"
                   % (letter, title, demote(md, 2)))
    return "\n".join(out)


CSS = """
@page { size: A4; margin: 22mm 20mm 22mm 20mm; }
body { font-family: 'Calibri', 'Segoe UI', Arial, sans-serif; font-size: 11pt; line-height: 1.45;
       color: #111; }
h1 { font-size: 26pt; color: #1F3864; margin-top: 60mm; text-align: center; }
h2 { font-size: 16pt; color: #1F3864; border-bottom: 1px solid #1F3864; margin-top: 18pt;
     page-break-after: avoid; }
h3 { font-size: 13pt; color: #1F3864; page-break-after: avoid; }
h4, h5, h6 { font-size: 11pt; color: #1F3864; page-break-after: avoid; }
.title-page { text-align: center; page-break-after: always; }
.title-page h2 { border: none; text-align: center; font-size: 16pt; }
table { border-collapse: collapse; width: 100%; font-size: 8.5pt; margin: 6pt 0;
        page-break-inside: auto; }
th, td { border: 1px solid #999; padding: 3pt 4pt; vertical-align: top; }
th { background: #1F3864; color: #fff; }
tr { page-break-inside: avoid; }
img { max-width: 100%; max-height: 225mm; display: block; margin: 8pt auto 2pt auto; }
p img + em, .caption { font-size: 9pt; }
pre, code { font-family: Consolas, monospace; font-size: 8.5pt; }
pre { background: #f4f6fa; padding: 6pt; white-space: pre-wrap; }
.page-break { page-break-before: always; }
.toc { page-break-after: always; }
.toc ul { list-style: none; padding-left: 14pt; }
.toc > ul { padding-left: 0; }
p { text-align: justify; }
"""


def main():
    md = io.open(os.path.join(HERE, "final-report.md"), encoding="utf-8").read()
    words = count_body_words(md)
    print("Body word count (sections 2-12, excluding tables, figures, code): %d" % words)
    full = md + appendices()
    converter = markdown.Markdown(extensions=["tables", "fenced_code", "toc", "md_in_html"],
                                  extension_configs={"toc": {"toc_depth": "2-3"}})
    html_body = converter.convert(full)
    toc = "<div class=\"toc\"><h2>Table of contents</h2>%s</div>" % converter.toc
    html_body = html_body.replace("<div class=\"toc-placeholder\"></div>", toc)
    # images: captions come from the alt text
    html_body = re.sub(r'<p><img alt="([^"]*)" src="([^"]*)" /></p>',
                       r'<p><img alt="\1" src="\2" /><em class="caption">\1</em></p>', html_body)
    html = ("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>LMS - Final "
            "Report</title><style>%s</style></head><body>%s</body></html>" % (CSS, html_body))
    html_path = os.path.join(HERE, "final-report.html")
    io.open(html_path, "w", encoding="utf-8").write(html)
    browser = sys.argv[1] if len(sys.argv) > 1 else \
        r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"
    pdf_path = os.path.join(HERE, "final-report.pdf")
    subprocess.run([browser, "--headless", "--disable-gpu", "--no-pdf-header-footer",
                    "--print-to-pdf=" + pdf_path, "file:///" + html_path.replace("\\", "/")],
                   check=True, timeout=180)
    print("Wrote", pdf_path)


if __name__ == "__main__":
    main()
