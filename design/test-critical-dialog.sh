#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail
source_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
build_dir=${BUILD_DIR:-"$source_root/_build"}
output="$build_dir/design-test-critical-dialog"
# Compile the actual widget. Only the version-report string is a test fixture.
read -ra flags <<< "$(pkg-config --cflags --libs gtk+-3.0 gegl-0.4)"
cc -Wno-deprecated-declarations -I"$build_dir" -I"$source_root" -I"$source_root/app" \
    "$source_root/design/test-critical-dialog.c" \
    "$source_root/app/widgets/gimpcriticaldialog.c" "${flags[@]}" -o "$output"
GDK_BACKEND=x11 timeout 30s xvfb-run -a "$output"
