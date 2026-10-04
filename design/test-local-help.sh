#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail
source_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
build_dir=${BUILD_DIR:-"$source_root/_build"}
read -ra flags <<< "$(pkg-config --cflags --libs gio-2.0)"
cc -DDISABLE_NLS -I"$build_dir" -I"$source_root" -I"$source_root/plug-ins/help" \
  "$source_root/design/test-local-help.c" \
  "$source_root/plug-ins/help/gimphelplocale.c" \
  "$source_root/plug-ins/help/gimphelpitem.c" \
  "$source_root/plug-ins/help/gimphelpprogress.c" \
  "${flags[@]}" -o "$build_dir/design-test-local-help"
timeout 15s "$build_dir/design-test-local-help"
