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
example 1.3.4), update CMake/spec/AppStream/changelog version metadata together,
merge the reviewed packaging PR and obtain Emir's explicit approval for the tag
and release. For the authorized 1.3.4 publication, AppStream is marked stable and the changelog release date is 2026-10-21. Fedora 45 remains a prerelease compatibility target; the public V2 release contains Fedora 44 artifacts only.

On an authorized exact version tag, the workflow requires all build, clean-install
and contract jobs to pass. The release operator checks native GitHub release immutability is enabled
with `bash packaging/scripts/check-release-settings.sh` before tagging. This
settings endpoint requires administrator read access and cannot be called with
the workflow token. CI uses supported contents/attestation permissions and refuses to replace any existing release, including drafts. It creates a new
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

## Fedora 45 compatibility

The same release preflight now builds Fedora 44 **and Fedora 45** on native
x86_64/aarch64 runners. CI compiles and runs the Qt tests and desktop/AppStream
validation against both distributions. Fedora 45 runtime jobs test fresh
installation, installed files, dependencies, linking, CLI and replacement of the
same-source Fedora 44 package with the Fedora 45 RPM. This exercises the package
upgrade transaction under Fedora 45 userspace; it is not a full OS upgrade test.

Fedora 45 RPMs use `1.fc45` and have their own matching SRPM. Download
`ro-control-fc45-compatibility-<run-id>` for the two RPMs, SRPM, `SHA256SUMS` and
`compatibility-report.json`. The report intentionally is not named or presented
as `component-artifact-manifest-v1.json`: the pinned Ro-Repo V2 schema and
producer registry accept **only Fedora 44**. The publication job downloads only
`ro-control-fc44-*`; Fedora 45 RPMs must never be mixed into that release set.
Fedora 45 compatibility checks must pass before the Fedora 44 publication job
can proceed on a future authorized tag.

Fedora 45 container results do not prove physical NVIDIA driver installation,
Secure Boot enrollment, akmods kernel-module builds, reboot or graphical login.
Those require booted Fedora 44/45 systems with supported NVIDIA hardware and
separate test evidence. Container packaging success must not close the reported
NVIDIA installer bug. Fedora 45 distribution-channel publication needs an
explicitly reviewed downstream contract and separate authorization.


The original v1.3.2 tag is preserved as an unsuccessful publication attempt;
no GitHub Release was created for it. The workflow permissions fix was prepared in
v1.3.3; v1.3.4 includes it together with the Ro-Theme and single-instance changes.
No tag is moved or reused to hide that history.
