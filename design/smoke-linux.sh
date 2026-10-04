#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Start GTK under a virtual display, parse the profile, then exit normally.
set -euo pipefail
source_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
install_prefix=${INSTALL_PREFIX:-"$source_root/_install"}
libdir=${GIMP_LIBDIR:-"$install_prefix/lib/x86_64-linux-gnu"}
profile=${GIMP_SMOKE_PROFILE:-"$source_root/_design-profile-linux-smoke"}
log="$source_root/_design-logs/linux-smoke.log"
mkdir -p "$source_root/_design-logs"
python3 "$source_root/design/launch.py" --prepare-only --profile "$profile"
# GIMP resolves a relative GIMP3_DIRECTORY against the user's home directory.
# Use the same absolute directory that the launcher prepared.
profile=$(cd -- "$profile" && pwd)
export GIMP3_DIRECTORY="$profile"
export GDK_BACKEND=x11
export LD_LIBRARY_PATH="$libdir:$install_prefix/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export GI_TYPELIB_PATH="$libdir/girepository-1.0${GI_TYPELIB_PATH:+:$GI_TYPELIB_PATH}"
export BABL_PATH=${BABL_PATH:-"$(pkg-config --variable=libdir babl-0.1)/babl-0.1"}
export GEGL_PATH=${GEGL_PATH:-"$(pkg-config --variable=libdir gegl-0.4)/gegl-0.4"}
timeout 60s xvfb-run -a "$install_prefix/bin/gimp-3.2" \
    --new-instance --verbose --no-splash \
    --batch-interpreter=plug-in-script-fu-eval --batch='(gimp-quit 0)' > "$log" 2>&1
if python3 - "$log" "$profile" <<'PY'
import sys
from pathlib import Path
text = Path(sys.argv[1]).read_text(errors="replace")
profile = Path(sys.argv[2])
for name in ("gimprc", "sessionrc", "toolrc", "shortcutsrc"):
    if f"Parsing '{profile / name}'" not in text:
        print(f"Expected profile file was not parsed: {profile / name}", file=sys.stderr)
        sys.exit(1)
errors = ("duplicate accelerator", "not existing action", "invalid accelerator",
          "Failed reading", "Error parsing", "Error while parsing",
          "missing tools in toolrc", "CRITICAL")
found = [error for error in errors if error.lower() in text.lower()]
if found:
    print("Profile errors: " + ", ".join(found), file=sys.stderr)
    sys.exit(1)
PY
then
    printf 'GTK startup/profile smoke passed. Log: %s\n' "$log"
else
    exit 1
fi
