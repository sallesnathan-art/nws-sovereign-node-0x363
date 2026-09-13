#!/usr/bin/env python3
"""Classify Linux thermal-zone names exposed by the S25 Ultra.

This does not invent a single 'device temperature'. It preserves every
raw sensor and assigns a functional class from the kernel-reported `type`.
"""

import re
import sys

PATTERNS = {
    "cpu": re.compile(r"^(cpu-|cpuss-)"),
    "gpu": re.compile(r"^gpuss-"),
    "memory": re.compile(r"^ddr$"),
    "battery": re.compile(r"^battery$"),
    "pmic": re.compile(r"^(pm\w+|pmr\w+)"),
    "modem": re.compile(r"^(mdmss-|nsphm)"),
    "radio": re.compile(r"^(mmw|mmw_|sdr)"),
    "camera": re.compile(r"^camera-"),
    "video": re.compile(r"^video$"),
    "always_on": re.compile(r"^aoss-"),
    "system": re.compile(r"^sys-therm-"),
    "power": re.compile(r"^ac$"),
}


def classify(sensor_type: str) -> str:
    for name, pattern in PATTERNS.items():
        if pattern.search(sensor_type):
            return name
    if sensor_type in {"mmw0", "mmw1", "mmw2", "mmw3", "mmw_ific0"}:
        return "radio"
    if sensor_type.startswith("pm8550-bcl-"):
        return "battery_current_limit"
    return "other"


def main() -> int:
    for line in sys.stdin:
        line = line.rstrip("\n")
        if not line.startswith("thermal="):
            print(line)
            continue
        fields = dict(part.split("=", 1) for part in line.split() if "=" in part)
        sensor_type = fields.get("type", "unknown")
        fields["class"] = classify(sensor_type)
        raw = fields.get("raw")
        if raw is not None and raw.startswith("-"):
            fields["valid"] = "false"
        elif raw == "0" and fields.get("class") == "battery_current_limit":
            fields["valid"] = "false"
        else:
            fields["valid"] = "true"
        order = ["thermal", "type", "class", "raw", "celsius", "valid"]
        extras = [k for k in fields if k not in order]
        print(" ".join(f"{k}={fields[k]}" for k in order + extras if k in fields))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
