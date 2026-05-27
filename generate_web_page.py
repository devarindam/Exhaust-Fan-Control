from pathlib import Path


ROOT = Path(__file__).resolve().parent


def resolve_input() -> Path:
    nested = ROOT / "preview" / "index.html"
    return nested if nested.exists() else ROOT / "index.html"


def resolve_output() -> Path:
    nested_parent = ROOT / "firmware" / "fan_control"
    return nested_parent / "web_page.h" if nested_parent.exists() else ROOT / "web_page.h"


def strip_simulation_block(lines: list[str]) -> list[str]:
    sim_start = -1
    sim_end = -1

    for i, line in enumerate(lines):
        if "Interactive Simulation Mode" in line:
            sim_start = i
        if sim_start >= 0 and line.strip() == "}, 2000);":
            sim_end = i
            break

    if sim_start >= 0 and sim_end >= 0:
        return lines[:sim_start] + lines[sim_end + 1 :]

    print("WARNING: Could not find simulation block boundaries. Using full file.")
    return lines


def main() -> None:
    input_file = resolve_input()
    output_file = resolve_output()

    lines = input_file.read_text(encoding="utf-8").splitlines()
    html_lines = strip_simulation_block(lines)
    html_content = "\n".join(html_lines)

    output = f"""#ifndef WEB_PAGE_H
#define WEB_PAGE_H

const char DASHBOARD_HTML[] PROGMEM = R\"rawliteral(
{html_content}
)rawliteral\";

#endif
"""
    output_file.write_text(output, encoding="utf-8")
    print(f"Successfully synced {output_file.name} ({len(html_lines)} lines of HTML)")


if __name__ == "__main__":
    main()
