#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Move obsolete bundled network plug-ins outside an isolated install tree."""
import argparse
from pathlib import Path


def retire(prefix):
    prefix = Path(prefix).resolve(strict=True)
    if prefix == Path(prefix.anchor) or prefix in (Path('/usr'), Path('/usr/local')):
        raise ValueError('Use an isolated installation prefix')
    if not (prefix / 'bin/gimp-3.2').is_file():
        raise ValueError('The prefix does not contain bin/gimp-3.2')
    targets = []
    for pattern in ('lib*/gimp/3.0/plug-ins', 'lib*/*/gimp/3.0/plug-ins'):
        for directory in prefix.glob(pattern):
            for name in ('mail', 'web-browser'):
                target = directory / name
                if target.exists() or target.is_symlink():
                    resolved = target.resolve(strict=True)
                    if not resolved.is_relative_to(prefix) or target.is_symlink():
                        raise ValueError(f'Unsafe plug-in path: {target}')
                    targets.append(target)
    # Validate every target before moving anything. Preserve binaries for recovery.
    planned = []
    for target in targets:
        relative = target.relative_to(prefix)
        destination = prefix / '_retired-plugins' / relative
        if not destination.resolve().is_relative_to(prefix):
            raise ValueError('Unsafe retirement directory')
        number = 0
        original = destination
        while destination.exists():
            number += 1
            destination = original.with_name(f'{original.name}.{number}')
        planned.append((target, destination))
    moved = []
    for target, destination in planned:
        destination.parent.mkdir(parents=True, exist_ok=True)
        target.rename(destination)
        moved.append((target, destination))
    return moved


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('prefix', type=Path)
    args = parser.parse_args()
    for source, destination in retire(args.prefix):
        print(f'Retired {source} -> {destination}')
