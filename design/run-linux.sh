#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail
source_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
install_prefix=${INSTALL_PREFIX:-"$source_root/_install"}
libdir=${GIMP_LIBDIR:-"$install_prefix/lib/x86_64-linux-gnu"}
export LD_LIBRARY_PATH="$libdir:$install_prefix/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export GI_TYPELIB_PATH="$libdir/girepository-1.0${GI_TYPELIB_PATH:+:$GI_TYPELIB_PATH}"
export BABL_PATH=${BABL_PATH:-"$(pkg-config --variable=libdir babl-0.1)/babl-0.1"}
export GEGL_PATH=${GEGL_PATH:-"$(pkg-config --variable=libdir gegl-0.4)/gegl-0.4"}
exec python3 "$source_root/design/launch.py" --executable "$install_prefix/bin/gimp-3.2" "$@"
