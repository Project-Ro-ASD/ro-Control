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
# Paste relevant output from:
# journalctl --user -u ro-control --no-pager -n 50
```

## Additional Context

Any other context, screenshots, or information that might help.
