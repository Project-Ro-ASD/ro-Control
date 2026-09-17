# Configuration Reference

**ro-Control** persists user preferences and hardware tuning state with the Qt
`QSettings` native backend. On Ro-ASD / Fedora these settings are stored in a
single INI-style file:

```
~/.config/Project-Ro-ASD/ro-control.conf
```

You can inspect the current values with `ro-control` running or, while it is
not running, by reading the file directly. Removing the file (or the individual
keys) resets the affected subsystem to its factory defaults.

> Values in the tables below are the built-in defaults used when a key is
> absent. Ranges listed under "Constraints" are enforced when the value is
> set through the application.

---

## 1. Thermal Safety (HealthGuard)

Stored at the top level of the settings file.

| Key | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `NotificationsEnabled` | bool | `true` | Enables desktop notifications for thermal warnings |
| `GpuWarningThreshold` | int | `82` | GPU warning temperature (°C). Constraint: 40–110 |
| `GpuCriticalThreshold` | int | `88` | GPU critical temperature (°C). Constraint: 45–115 |
| `CpuWarningThreshold` | int | `85` | CPU warning temperature (°C). Constraint: 40–115 |
| `CpuCriticalThreshold` | int | `95` | CPU critical temperature (°C). Constraint: 50–125 |

Crossing the warning threshold emits a warning notification; crossing the
critical threshold triggers an emergency 100% cooling override.

---

## 2. GPU Power (PowerController)

Stored at the top level of the settings file.

| Key | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `Preset` | string | `balanced` | Active power preset (`eco`, `balanced`, `performance`, `custom`) |
| `PersistenceMode` | bool | `false` | NVIDIA driver persistence daemon enabled |
| `LastTargetWatts` | real | — | Last applied GPU power limit in Watts |
| `CoreClockOffset` | int | `0` | Core clock offset in MHz |
| `MemoryClockOffset` | int | `0` | Memory clock offset in MHz |

---

## 3. Diagnostic Report

Stored at the top level of the settings file.

| Key | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `format` | string | `markdown` | Report output format (`markdown`) |
| `destination` | string | `preview` | Report destination (`preview`) |

---

## 4. UI Preferences & Language

Stored under the `ui` group.

| Key | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `ui/showAdvancedInfo` | bool | `true` | Shows advanced diagnostics information |
| `ui/language` | string | `en` | Interface language code (e.g. `en`, `tr`, `de`, `es`); empty means system locale |

---

## 5. Fan Control

Fan settings are stored per hardware domain under dedicated groups.

### GPU Fan — `FanControl`

| Key | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `mode` | string | `auto` | Cooling mode (`auto`, `silent`, `balanced`, `performance`, `manual`, `custom`) |
| `manualSpeed` | int | `50` | Fixed manual speed (%) in `manual` mode. Constraint: 0–100 |
| `thermalThreshold` | int | `85` | Fan thermal threshold (°C) |
| `smoothingEnabled` | bool | `false` | Fan ramp smoothing enabled |
| `batteryProfileSyncEnabled` | bool | `false` | Syncs fan profile with battery power source |
| `rampUpRate` | int | `20` | Ramp-up rate (%/s). Constraint: 1–100 |
| `rampDownRate` | int | `5` | Ramp-down rate (%/s). Constraint: 1–100 |
| `hysteresisTempC` | int | `2` | Directional hysteresis (°C). Constraint: 0–15 |
| `displayName` | string | — | Custom fan display label |
| `curveCount` | int | `0` | Number of custom curve points |
| `curveTemp_<N>` | int | — | Temperature (°C) of curve point N |
| `curveSpeed_<N>` | int | — | Fan speed (%) of curve point N |

### CPU Fan — `FanControl_cpu`

| Key | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `mode` | string | `auto` | Cooling mode (`auto`, `silent`, `balanced`, `performance`, `manual`, `custom`) |
| `manualSpeed` | int | `50` | Fixed manual speed (%). Constraint: 0–100 |
| `thermalThreshold` | int | `90` | Fan thermal threshold (°C). Constraint: 60–105 |
| `displayName` | string | — | Custom fan display label |
| `curveCount` | int | `0` | Number of custom curve points |
| `curveTemp_<N>` | int | — | Temperature (°C) of curve point N |
| `curveSpeed_<N>` | int | — | Fan speed (%) of curve point N |

### Chassis Fan — `FanControl_sys`

| Key | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `mode` | string | `auto` | Cooling mode (`auto`, `silent`, `balanced`, `performance`, `manual`, `custom`) |
| `manualSpeed` | int | `35` | Fixed manual speed (%). Constraint: 0–100 |
| `thermalThreshold` | int | `75` | Fan thermal threshold (°C). Constraint: 50–95 |
| `displayName` | string | — | Custom fan display label |
| `curveCount` | int | `0` | Number of custom curve points |
| `curveTemp_<N>` | int | — | Temperature (°C) of curve point N |
| `curveSpeed_<N>` | int | — | Fan speed (%) of curve point N |

---

## 6. Manual Override Examples

Reset the complete configuration to defaults:

```bash
rm ~/.config/Project-Ro-ASD/ro-control.conf
```

Persistently lower the GPU critical threshold:

```ini
[General]
GpuCriticalThreshold=82
```

These keys are managed through the desktop UI and CLI and should rarely need
manual editing. If you edit the file, do so while `ro-control` is not running.