#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Prepare and run a private development profile without replacing user settings."""

import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[1]
MARKER = ".gimp-linux-design-profile"
FILES = {
    "gimprc": ROOT / "design/profile/gimprc",
    "toolrc": ROOT / "etc/toolrc",
    "sessionrc": ROOT / "etc/sessionrc",
    "shortcutsrc": ROOT / "etc/shortcutsrc",
}


def prepare(profile):
    # Only operate on a directory created by this launcher or an empty one.
    if profile.is_symlink():
        raise ValueError("The profile cannot be a symbolic link.")
    profile = profile.resolve()
    if profile.exists() and not profile.is_dir():
        raise ValueError("The profile must be a directory.")
    marker = profile / MARKER
    if profile.exists() and any(profile.iterdir()) and not marker.is_file():
        raise ValueError("Refusing to modify an existing non-design profile.")
    if marker.is_symlink() or (marker.exists() and marker.read_text() != "1\n"):
        raise ValueError("Invalid design profile marker.")
    # Validate all destinations before copying anything.
    for name in FILES:
        target = profile / name
        if target.is_symlink() or (target.exists() and not target.is_file()):
            raise ValueError(f"Unsafe profile destination: {name}")
    profile.mkdir(parents=True, exist_ok=True)
    marker.write_text("1\n")
    for name, source in FILES.items():
        target = profile / name
        if not target.exists():
            shutil.copyfile(source, target)
    return profile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--profile", type=Path, default=ROOT / "_design-profile")
    parser.add_argument("--executable", type=Path,
                        help="Explicit GIMP 3.2 executable (fork or reference build)")
    parser.add_argument("--prepare-only", action="store_true")
    parser.add_argument("files", nargs="*", help="Local image files to open")
    args = parser.parse_args()
    try:
        if args.prepare_only:
            print(prepare(args.profile))
            return 0
        if not args.executable:
            parser.error("--executable is required unless --prepare-only is used")
        executable = args.executable.resolve(strict=True)
        if not executable.is_file():
            raise ValueError("The executable must be a file.")
        files = [str(Path(name).resolve(strict=True)) for name in args.files]
        if any(not Path(name).is_file() for name in files):
            raise ValueError("Only local image files are accepted.")
        environment = os.environ.copy()
        version = subprocess.run([str(executable), "--version"], env=environment,
                                 capture_output=True, text=True, timeout=20, check=True)
        if "3.2." not in version.stdout:
            raise ValueError("This initial profile requires GIMP 3.2.x.")
        profile = prepare(args.profile)
        environment["GIMP3_DIRECTORY"] = str(profile)
        command = [str(executable), "--new-instance", "--verbose", "--no-splash", *files]
        logs = ROOT / "_design-logs"
        logs.mkdir(exist_ok=True)
        stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S%fZ")
        log = logs / f"gimp-{stamp}.log"
        print(f"Profile: {profile}\nLog: {log}", flush=True)
        with log.open("w", encoding="utf-8") as stream:
            stream.write(json.dumps({"command": command, "profile": str(profile),
                                     "version": version.stdout.strip()}) + "\n")
            stream.flush()
            return subprocess.run(command, env=environment, stdout=stream,
                                  stderr=subprocess.STDOUT, check=False).returncode
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(f"GIMP Linux Design: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
