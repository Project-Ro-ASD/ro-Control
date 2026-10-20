#!/usr/bin/env python3
"""Run the pinned consumer's bundle validation without accepting/publishing it."""
import json
import pathlib
import sys

contract, artifacts, fedora_names = map(pathlib.Path, sys.argv[1:])
sys.path.insert(0, str(contract / "tools"))
import ro_repo  # noqa: E402

manifest_path = artifacts / "component-artifact-manifest-v1.json"
manifest, headers, diagnostics = ro_repo.verify_component(
    manifest_path, artifacts, contract / "config/producers-v2.json", fedora_names
)
report = {
    "scope": "bundle-only; release ID is a placeholder; provenance is not verified",
    "manifest_sha256": ro_repo.digest(manifest_path),
    "headers": headers,
    "diagnostics": diagnostics,
}
(artifacts / "preflight-report.json").write_text(json.dumps(report, indent=2) + "\n")
print(json.dumps(report, indent=2))
