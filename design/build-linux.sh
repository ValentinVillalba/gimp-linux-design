#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail
source_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
build_dir=${BUILD_DIR:-"$source_root/_build"}
install_prefix=${INSTALL_PREFIX:-"$source_root/_install"}
jobs=${BUILD_JOBS:-2}
export LC_ALL=${BUILD_LOCALE:-en_US.UTF-8}
if ! python3 -c 'import locale; locale.setlocale(locale.LC_ALL, "")'; then
    printf 'Generate en_US.UTF-8 with localedef/locale-gen, or set BUILD_LOCALE to an installed non-C UTF-8 locale.\n' >&2
    exit 1
fi
if [[ ! "$jobs" =~ ^[1-9][0-9]*$ ]]; then
    printf 'BUILD_JOBS must be a positive integer.\n' >&2
    exit 1
fi
if [[ ! -f "$source_root/gimp-data/meson.build" ]]; then
    printf 'Missing gimp-data: run git submodule update --init first.\n' >&2
    exit 1
fi
options=(--prefix="$install_prefix" --buildtype=debugoptimized
         -Dcheck-update=no -Dgimpdir=GimpLinuxDesign
         -Dgi-docgen=disabled -Dwebkit-unmaintained=false
         -Dauto_features=disabled -Djavascript=disabled -Dlua=false
         -Dheadless-tests=enabled)
if [[ -f "$build_dir/build.ninja" ]]; then
    meson setup --reconfigure "$build_dir" "$source_root" "${options[@]}"
else
    meson setup "$build_dir" "$source_root" "${options[@]}"
fi
meson compile -C "$build_dir" -j "$jobs"
meson test -C "$build_dir" --print-errorlogs
meson install -C "$build_dir"
python3 "$source_root/design/retire-online-plugins.py" "$install_prefix"
printf 'Installed into %s\n' "$install_prefix"
