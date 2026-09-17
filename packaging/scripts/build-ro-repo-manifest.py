#!/usr/bin/env python3
"""Build component-artifact-manifest-v1.json from an exact RPM release set."""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import subprocess


def sha256(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def rpm_header(path: pathlib.Path) -> dict[str, object]:
    query = "%{NAME}\t%{EPOCHNUM}\t%{VERSION}\t%{RELEASE}\t%{ARCH}\t%{SOURCERPM}\t%|SOURCEPACKAGE?{true}:{false}|"
    output = subprocess.check_output(
        ["rpm", "-qp", "--qf", query, str(path)], text=True
    )
    values = output.split("\t")
    if len(values) != 7:
        raise SystemExit(f"unexpected RPM header for {path.name}: {output!r}")

    name, epoch, version, release, architecture, source_rpm, is_source = values
    if is_source == "true":
        architecture = "src"
        source_rpm_value = None
    else:
        source_rpm_value = source_rpm

    return {
        "filename": path.name,
        "name": name,
        "epoch": int(epoch or 0),
        "version": version,
        "release": release,
        "architecture": architecture,
        "source_rpm": source_rpm_value,
        "producer_artifact_sha256": sha256(path),
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--artifacts-dir", required=True, type=pathlib.Path)
    parser.add_argument("--output", required=True, type=pathlib.Path)
    parser.add_argument("--repository", required=True)
    parser.add_argument("--commit", required=True)
    parser.add_argument("--tag", required=True)
    parser.add_argument("--release-id", required=True)
    parser.add_argument("--workflow-run", required=True)
    args = parser.parse_args()

    rpms = sorted(args.artifacts_dir.glob("*.rpm"))
    if len(rpms) != 3:
        raise SystemExit(
            f"ro-control release must contain exactly 3 RPMs (x86_64, aarch64, src); found {[p.name for p in rpms]}"
        )

    artifacts = [rpm_header(path) for path in rpms]
    architectures = sorted(str(item["architecture"]) for item in artifacts)
    if architectures != ["aarch64", "src", "x86_64"]:
        raise SystemExit(f"unexpected RPM architecture set: {architectures}")

    for item in artifacts:
        if item["name"] != "ro-control":
            raise SystemExit(f"unexpected package name in {item['filename']}: {item['name']}")
        if not str(item["release"]).endswith(".fc44"):
            raise SystemExit(f"RPM is not a Fedora 44 build: {item['filename']}")

    source_names = {
        str(item["filename"])
        for item in artifacts
        if item["architecture"] == "src"
    }
    for item in artifacts:
        if item["architecture"] != "src" and item["source_rpm"] not in source_names:
            raise SystemExit(
                f"binary RPM {item['filename']} does not point at the published SRPM {item['source_rpm']}"
            )

    manifest = {
        "schema_version": 1,
        "component": "ro-control",
        "source_repository": args.repository,
        "source_commit": args.commit,
        "release_tag": args.tag,
        "release_id": int(args.release_id),
        "workflow_run": int(args.workflow_run),
        "fedora_release": 44,
        "artifacts": artifacts,
        "provenance": {
            "provider": "github-actions",
            "subject_digest": f"git:{args.commit}",
        },
        "attestation": {
            "provider": "github-artifact-attestations",
            "verification": "gh attestation verify with exact repository, commit and signer workflow",
        },
        "sbom": None,
    }

    args.output.write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )


if __name__ == "__main__":
    main()
