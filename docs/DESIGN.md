# Design

This document captures practical UI/UX decisions for ro-Control so contributors can keep the interface consistent.

## Product Goals

- Keep NVIDIA driver actions understandable for non-technical users.
- Surface risky states (for example Secure Boot) before running privileged actions.
- Show system metrics clearly without overwhelming the user.

## Information Architecture

- `Surucu` tab:
	- NVIDIA detection status
	- Active driver and installed version
	- Install/update/deep-clean actions
	- Session and Secure Boot warnings
- `Izleme` tab:
	- CPU usage and temperature
	- GPU usage, temperature, and VRAM
	- RAM usage
- `Ayarlar` tab:
	- General app-level settings and about information

## Interaction Rules

- Privileged actions must be explicit and user-initiated by button click.
- Destructive actions (`Deep Clean`) should always log progress messages.
- UI should refresh detector and updater state after install/remove/update events.
- Progress and result messages should be visible in the same screen where action is triggered.

## Visual Language

- Use readable spacing and grouped cards for each subsystem.
- Status colors:
	- Warning: amber (`#8a6500`)
	- Error/risky security state: red (`#c43a3a`)
	- Normal/healthy state: green (`#2b8a3e`)
- Prefer explicit labels over icon-only controls.

## Accessibility and Localization

- Keep labels short and descriptive for Turkish and English translations.
- Avoid color-only communication; pair color with text.
- Ensure controls remain usable on 980x640 and larger windows.

## Future Design Backlog

- Add confirmation dialog before deep clean.
- Add lightweight charts for metric history (CPU/GPU/RAM).
- Add screenshot set for README and AppStream metadata.