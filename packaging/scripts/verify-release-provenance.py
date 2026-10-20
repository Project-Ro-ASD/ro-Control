#!/usr/bin/env python3
"""Verify all five subjects using the consumer's exact run/attempt verifier."""
import json
import os
import pathlib
import sys

contract, assets = map(pathlib.Path, sys.argv[1:])
sys.path.insert(0, str(contract / "tools"))
import ro_repo  # noqa: E402

path = assets / "component-artifact-manifest-v1.json"
manifest = json.loads(path.read_text())
assert manifest["source_commit"] == os.environ["GITHUB_SHA"]
assert manifest["release_tag"] == os.environ["GITHUB_REF_NAME"]
assert manifest["workflow_run"] == int(os.environ["GITHUB_RUN_ID"])
workflow = "Project-Ro-ASD/ro-Control/.github/workflows/release.yml"
verified = ro_repo.verify_attestations(manifest, assets, path, workflow)
assert len(verified) == 5
assert {entry["workflow_attempt"] for entry in verified.values()} == {
    int(os.environ["GITHUB_RUN_ATTEMPT"])
}
pathlib.Path("verified-release-provenance.json").write_text(
    json.dumps(verified, indent=2) + "\n"
)
print("Verified all RPMs, SHA256SUMS and manifest against exact source, tag, workflow, run and attempt")
