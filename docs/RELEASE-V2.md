# Fedora 44 / Ro-Repo V2 release verification

The release workflow builds native x86_64 and aarch64 packages and one shared
SRPM from the exact checked-out commit. The spec owns `Release: 1%{?dist}`;
all Fedora 44 workflows use `1.fc44`. Both binaries must name the published
SRPM and share its epoch, version and release.

## Review without publishing

Pull requests and manual `Release` workflow runs build the same RPMs as tags,
run the spec's CTest suite, then install them in fresh Fedora 44 containers on
both architectures. The runtime jobs resolve dependencies with weak dependencies
disabled, test the RPM transaction for conflicts, verify installed files and
linking, run CLI help/version, reinstall, and upgrade from the unchanged v1.3.1
release. No `--allowerasing` is permitted.

The bundle preflight invokes `verify_component` from Ro-Repo commit
`9fb1deb363ab6f43b595bf526bba966ec0587c27`, the consumer behind
`accept-component.yml`. It checks actual RPM headers, architecture/SRPM parity,
checksums, schema, producer policy, Fedora package name collisions and payload
conflicts. Its `preflight-report.json` includes advisory rpmlint diagnostics.
The consumer's rpmlint policy is advisory; it is not silently presented as a
passing lint gate.

Download `ro-control-v2-preflight-<run-id>` from the workflow run for both binary
RPMs, SRPM, SHA256SUMS, manifest and preflight report. The preflight manifest uses
release ID **1 as a placeholder**. It is not an acceptance-ready release and has
no tag-bound provenance. Do not submit it to Ro-Repo.

## Authorized release procedure

Do not reuse or modify v1.3.1. Before a new release, select a new version (for
example 1.3.2), update CMake/spec/AppStream/changelog version metadata together,
merge the reviewed packaging PR and obtain Emir's explicit approval for the tag
and release. This PR intentionally creates no new version or tag.

On an authorized exact version tag, the workflow requires all build, clean-install
and contract jobs to pass. It checks native GitHub release immutability is enabled
and refuses to replace any existing release, including drafts. It creates a new
draft, uses its real numeric release ID in the final manifest, and attests all
three RPMs, SHA256SUMS and the manifest. The pinned consumer's verifier must verify
repository, source digest, tag ref, signer workflow and the exact run/attempt for
all five subjects before upload/publication. A final API check confirms the
published release is immutable and retains the expected ID and tag.

A failed draft is preserved for investigation. Do not delete/recreate it casually;
its ID is part of the manifest identity. Recovery requires an explicit reviewed
release decision, not a workflow rerun that silently replaces the draft.

After the release run completes, independently download its assets and verify:

```bash
sha256sum --check SHA256SUMS
for subject in *.rpm SHA256SUMS component-artifact-manifest-v1.json; do
  gh attestation verify "$subject" --repo Project-Ro-ASD/ro-Control \
    --source-digest "$SOURCE_COMMIT" --source-ref "refs/tags/$RELEASE_TAG" \
    --signer-workflow Project-Ro-ASD/ro-Control/.github/workflows/release.yml \
    --deny-self-hosted-runners
 done
```

Then compare the manifest IDs with the release API, tag commit and successful
workflow run attempt. Only after separate approval may Ro-Repo's
`accept-component` be dispatched with the exact repository, tag, 40-character
commit and numeric release ID. That workflow also signs accepted packages;
it is deliberately not dispatched by this producer PR. No Beta publication is
performed here. Bundle preflight success alone does not prove live acceptance.

Containers verify Fedora 44 userspace and native RPM transactions; they do not
prove booted Fedora/systemd service behavior or physical GPU operations.
