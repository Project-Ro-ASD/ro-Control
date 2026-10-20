#!/usr/bin/env python3
"""Record Fedora 45 evidence without claiming the Fedora 44 Ro-Repo contract."""
import importlib.util
import json
import os
import pathlib
import sys

module_path = pathlib.Path(__file__).with_name("build-ro-repo-manifest.py")
spec = importlib.util.spec_from_file_location("manifest", module_path)
manifest = importlib.util.module_from_spec(spec)
spec.loader.exec_module(manifest)
assets = pathlib.Path(sys.argv[1])
artifacts = [manifest.rpm_header(path) for path in sorted(assets.glob("*.rpm"))]
if sorted(item["architecture"] for item in artifacts) != ["aarch64", "src", "x86_64"]:
    raise SystemExit("expected exactly x86_64, aarch64 and shared SRPM")
if any(item["name"] != "ro-control" for item in artifacts):
    raise SystemExit("unexpected package name")
manifest.validate_identity(
    artifacts, os.environ["GITHUB_REPOSITORY"], os.environ["GITHUB_SHA"],
    "v" + os.environ["VERSION"], "1", os.environ["GITHUB_RUN_ID"], fedora_release=45,
)
report = {
    "kind": "fedora-compatibility-evidence",
    "fedora_release": 45,
    "source_commit": os.environ["GITHUB_SHA"],
    "workflow_run": int(os.environ["GITHUB_RUN_ID"]),
    "artifacts": artifacts,
    "scope": "native container build/install/upgrade evidence; not a release or Ro-Repo acceptance manifest",
}
(assets / "compatibility-report.json").write_text(json.dumps(report, indent=2) + "\n")
(assets / "SHA256SUMS").write_text("".join(
    f"{item['producer_artifact_sha256']}  {item['filename']}\n" for item in artifacts
))
print(json.dumps(report, indent=2))
