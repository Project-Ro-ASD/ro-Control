# Release Checklist

Use this checklist for every production release.

## 1. Branch and PR State

- [ ] `dev` and `main` are synchronized.
- [ ] All required PR reviews are approved.
- [ ] No pending change requests remain.

## 2. Quality Gates

- [ ] CI `Build & Test` is green.
- [ ] Formatting check is green.
- [ ] RPM build/lint job is green.
- [ ] Local `ctest --output-on-failure` passes.

## 3. Versioning and Changelog

- [ ] Update version in `CMakeLists.txt` if needed.
- [ ] Update `CHANGELOG.md` with final release notes.
- [ ] Ensure AppStream metadata is up to date.
- [ ] Refresh translation sources and verify `.ts` files are complete.
- [ ] Smoke-test the app in English and Turkish locales.

## 4. Packaging

- [ ] `packaging/rpm/ro-control.spec` release/version fields are correct.
- [ ] Build RPM artifacts successfully.
- [ ] Verify installation and launch on the target desktop environment.
- [ ] Verify `man ro-control` and shell completions install correctly.

## 5. Tag and Publish

- [ ] Create annotated tag: `vX.Y.Z`.
- [ ] Push tag to trigger release workflow.
- [ ] Verify GitHub Release includes source archives.

## 6. Post-release

- [ ] Announce release notes.
- [ ] Open next milestone planning issues.
- [ ] Back-merge `main` into `dev` if release happened from PR flow.
