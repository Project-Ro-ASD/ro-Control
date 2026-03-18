---
name: Bug Report
about: Report a bug or unexpected behavior
title: "[BUG] "
labels: bug
assignees: ''
---

## Description

A clear and concise description of the bug.

## Steps to Reproduce

1. Go to '...'
2. Click on '...'
3. See error

## Expected Behavior

What you expected to happen.

## Actual Behavior

What actually happened.

## Environment

| Field | Value |
|-------|-------|
| Fedora version | `cat /etc/fedora-release` |
| GPU model | `lspci \| grep -i nvidia` |
| Driver version | `nvidia-smi \| head -1` |
| ro-Control version | (from About page or package info) |
| Desktop session | Wayland / X11 |

## Logs

```
# Paste any relevant output from one or more of these:
# ro-control started from a terminal
# coredumpctl info ro-control
# journalctl --since "10 minutes ago" --no-pager
# dnf history info
```

## Additional Context

Any other context, screenshots, or information that might help.
