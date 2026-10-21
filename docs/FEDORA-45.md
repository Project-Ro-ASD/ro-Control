# Fedora 45 readiness and host qualification

Version 1.3.4 builds Fedora 44/45 on native x86_64 and aarch64.
`release.yml` validates the complete release candidate; `rpm-artifacts.yml`
also produces both distributions on main/manual runs. Fedora-specific binary
RPMs and SRPMs must remain in separate sets. Ro-Repo V2 currently accepts only
Fedora 44; Fedora 45 distribution-channel support requires downstream review.
No tag, release, repository submission or Beta promotion is performed by this guide.

## Automated gates

- C++/Qt tests, packaging/helper boundary tests, desktop/AppStream validation.
- Exact RPM/SRPM identity and checksums, no Qt private ABI requirement.
- Fresh install without weak dependencies or `--allowerasing`, RPM transaction
  test, installed-file verification, linking, reinstall and package upgrade.
- Installed CLI status/diagnostics JSON and helper's exact locale-pinned DNF query.
- Installed Polkit/helper path and system/user unit executable paths.
- Private session bus: start the installed daemon and retrieve real JSON telemetry.
- Load the full installed QML app with offscreen/software rendering, failing on
  startup exit, missing QML modules or component-loading errors.

Fedora 45 tests replace the same-source Fedora 44 package under Fedora 45
userspace. They do not perform a full operating-system upgrade.

## Booted Fedora 45 acceptance

Use a disposable VM for OS upgrade/login/service tests and a supported NVIDIA
machine for GPU tests. Record architecture, kernel, GPU PCI ID, driver flavor,
ro-Control NEVRA, source commit and Secure Boot state. Preserve before/after
results for a real Fedora 44 → 45 OS upgrade; do not substitute the container
package replacement test for this evidence.

1. Verify the candidate RPM SHA256 against its CI bundle and install the package
   for the host's exact Fedora version and architecture. Follow `INSTALL.md`.
2. Launch the desktop application in a normal Wayland desktop session. Verify
   monitor/driver pages, no-GPU state where applicable, keyboard focus and
   resize behavior. Check installed daemon telemetry from the user session.
3. Exercise Polkit authorization and cancellation. The installer requires a
   detected Wayland session and compatible GPU. Save the first failing command
   and complete progress/error output if installation fails; track this in #57.
4. For each supported/available driver flavor on the chosen GPU, verify package
   installation, matching running-kernel build, Secure Boot enrollment/signature
   requirements when enabled, and correct reboot-required reporting. An Open
   Kernel Modules test is applicable only where GPU/package support exists.
5. After the required reboot, verify module loading, `nvidia-smi`, graphical
   login, application startup and telemetry. Record failures as failures, even
   when package installation itself succeeded. Do not disable Secure Boot or
   use Deep Clean as a substitute for diagnosing an installation failure.
6. Collect read-only baseline probes from the actual user session:

   ```bash
   python3 packaging/scripts/collect-fedora45-evidence.py --output fedora45-host-evidence.json
   ```

   The collector never installs drivers, changes boot options, enables services,
   reboots or publishes data. It refuses to overwrite existing evidence. A zero
   exit code establishes only a Fedora 45 systemd host with a responding NVIDIA
   GPU; `release_qualified` remains false because manual workflow evidence is
   still required. An inactive optional service is recorded, not automatically
   enabled. Attach reviewed evidence and installer logs to #57/#58.

## Release handoff

1. Complete or explicitly record the remaining booted/hardware qualifications in
   #57/#58. The confirmed helper command rejection is fixed and tested separately;
   it does not establish the cause of every reported NVIDIA installation failure.
2. Review version changes; set the actual stable release date/type in
   AppStream and changelog and obtain Emir's approval for any new tag/release.
3. Follow `RELEASE-V2.md` for immutable Fedora 44 publication/attestation checks.
   Fedora 45 compatibility RPMs are not a V2 acceptance candidate. Arrange a
   separately reviewed Fedora 45 downstream contract before channel distribution.
