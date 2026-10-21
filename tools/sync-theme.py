#!/usr/bin/env python3
"""Generate the QML palette from the pinned upstream snapshots (no network)."""
import argparse
import json
import hashlib
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--check', action='store_true')
args = parser.parse_args()
provenance = json.loads((ROOT / 'data/theme/upstream.json').read_text())
for path, expected in provenance['files'].items():
    actual = hashlib.sha256((ROOT / path).read_bytes()).hexdigest()
    if actual != expected:
        raise SystemExit(f'Upstream snapshot changed: {path}; review and update provenance')
for path in (ROOT / 'src/qml').rglob('*.qml'):
    if 'style' not in path.parts:
        source = path.read_text()
        if re.search(r'#[0-9a-fA-F]{6}|\bgradient\s*:', source):
            raise SystemExit(f'Use shared theme roles instead of literal colors/gradients: {path}')
output = '// Generated from data/theme/colors.*.json by tools/sync-theme.py.\n.pragma library\n'
for mode in ('light', 'dark'):
    palette = json.loads((ROOT / f'data/theme/colors.{mode}.json').read_text())
    output += f'var {mode} = ' + json.dumps(palette, ensure_ascii=False, indent=2) + ';\n'
target = ROOT / 'src/qml/style/Palettes.js'
if args.check:
    if target.read_text() != output:
        raise SystemExit('QML palette is stale; run python3 tools/sync-theme.py')
else:
    target.write_text(output)
