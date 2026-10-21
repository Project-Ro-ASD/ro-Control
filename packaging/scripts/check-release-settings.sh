#!/usr/bin/env bash
# Operator-side read-only preflight. Administration: read is required by GitHub.
# Never copy an administrator token into the publication workflow to run this.
set -euo pipefail
release_repository="${GITHUB_REPOSITORY:-Project-Ro-ASD/ro-Control}"
[[ "$(gh api "repos/${release_repository}/immutable-releases" --jq .enabled)" == true ]]
echo "Native release immutability is enabled for ${release_repository}"
