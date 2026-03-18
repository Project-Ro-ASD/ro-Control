<?xml version="1.0" encoding="utf-8"?>
<TS version="2.1" language="en" sourcelanguage="en">
<context>
    <name>DriverPage</name>
    <message><source>Driver Management</source><translation>Driver Management</translation></message>
    <message><source>GPU: </source><translation>GPU: </translation></message>
    <message><source>Not detected</source><translation>Not detected</translation></message>
    <message><source>Active driver: </source><translation>Active driver: </translation></message>
    <message><source>Driver version: </source><translation>Driver version: </translation></message>
    <message><source>None</source><translation>None</translation></message>
    <message><source>Secure Boot: </source><translation>Secure Boot: </translation></message>
    <message><source>Enabled</source><translation>Enabled</translation></message>
    <message><source>Disabled / Unknown</source><translation>Disabled / Unknown</translation></message>
    <message><source>Session type: </source><translation>Session type: </translation></message>
    <message><source>For Wayland sessions, nvidia-drm.modeset=1 is applied automatically.</source><translation>For Wayland sessions, nvidia-drm.modeset=1 is applied automatically.</translation></message>
    <message><source>For X11 sessions, the xorg-x11-drv-nvidia package is verified and installed.</source><translation>For X11 sessions, the xorg-x11-drv-nvidia package is verified and installed.</translation></message>
    <message><source>I accept the license / agreement terms</source><translation>I accept the license / agreement terms</translation></message>
    <message><source>Install Proprietary Driver</source><translation>Install Proprietary Driver</translation></message>
    <message><source>Install Open-Source Driver (Nouveau)</source><translation>Install Open-Source Driver (Nouveau)</translation></message>
    <message><source>Deep Clean</source><translation>Deep Clean</translation></message>
    <message><source>Check for Updates</source><translation>Check for Updates</translation></message>
    <message><source>Apply Update</source><translation>Apply Update</translation></message>
    <message><source>Latest version: </source><translation>Latest version: </translation></message>
    <message><source>Rescan</source><translation>Rescan</translation></message>
    <message><source>Installed NVIDIA version: </source><translation>Installed NVIDIA version: </translation></message>
</context>
<context>
    <name>Main</name>
    <message><source>ro-Control</source><translation>ro-Control</translation></message>
    <message><source>Theme: System (Dark)</source><translation>Theme: System (Dark)</translation></message>
    <message><source>Theme: System (Light)</source><translation>Theme: System (Light)</translation></message>
    <message><source>Driver</source><translation>Driver</translation></message>
    <message><source>Monitor</source><translation>Monitor</translation></message>
    <message><source>Settings</source><translation>Settings</translation></message>
</context>
<context>
    <name>MonitorPage</name>
    <message><source>System Monitoring</source><translation>System Monitoring</translation></message>
    <message><source>CPU</source><translation>CPU</translation></message>
    <message><source>Usage: </source><translation>Usage: </translation></message>
    <message><source>CPU data unavailable</source><translation>CPU data unavailable</translation></message>
    <message><source>Temperature: </source><translation>Temperature: </translation></message>
    <message><source>GPU (NVIDIA)</source><translation>GPU (NVIDIA)</translation></message>
    <message><source>NVIDIA GPU</source><translation>NVIDIA GPU</translation></message>
    <message><source>Failed to read data via nvidia-smi</source><translation>Failed to read data via nvidia-smi</translation></message>
    <message><source>Load: </source><translation>Load: </translation></message>
    <message><source>VRAM: </source><translation>VRAM: </translation></message>
    <message><source>RAM</source><translation>RAM</translation></message>
    <message><source>RAM data unavailable</source><translation>RAM data unavailable</translation></message>
    <message><source>Refresh</source><translation>Refresh</translation></message>
    <message><source>Refresh interval: </source><translation>Refresh interval: </translation></message>
</context>
<context>
    <name>NvidiaDetector</name>
    <message><source>Proprietary (NVIDIA)</source><translation>Proprietary (NVIDIA)</translation></message>
    <message><source>Open Source (Nouveau)</source><translation>Open Source (Nouveau)</translation></message>
    <message><source>Not Installed / Unknown</source><translation>Not Installed / Unknown</translation></message>
    <message><source>GPU: %1
Driver Version: %2
Secure Boot: %3
Session: %4
NVIDIA Module: %5
Nouveau: %6</source><translation>GPU: %1
Driver Version: %2
Secure Boot: %3
Session: %4
NVIDIA Module: %5
Nouveau: %6</translation></message>
    <message><source>Unknown</source><translation>Unknown</translation></message>
    <message><source>Loaded</source><translation>Loaded</translation></message>
    <message><source>Not loaded</source><translation>Not loaded</translation></message>
    <message><source>Active</source><translation>Active</translation></message>
    <message><source>Inactive</source><translation>Inactive</translation></message>
</context>
<context>
    <name>NvidiaInstaller</name>
    <message><source>You must accept the NVIDIA proprietary driver license terms before installation. Detected license: %1</source><translation>You must accept the NVIDIA proprietary driver license terms before installation. Detected license: %1</translation></message>
    <message><source>License agreement acceptance is required before installation.</source><translation>License agreement acceptance is required before installation.</translation></message>
    <message><source>Checking RPM Fusion repositories...</source><translation>Checking RPM Fusion repositories...</translation></message>
    <message><source>Platform version could not be detected.</source><translation>Platform version could not be detected.</translation></message>
    <message><source>Failed to enable RPM Fusion repositories: </source><translation>Failed to enable RPM Fusion repositories: </translation></message>
    <message><source>Installing the proprietary NVIDIA driver (akmod-nvidia)...</source><translation>Installing the proprietary NVIDIA driver (akmod-nvidia)...</translation></message>
    <message><source>Installation failed: </source><translation>Installation failed: </translation></message>
    <message><source>Building the kernel module (akmods --force)...</source><translation>Building the kernel module (akmods --force)...</translation></message>
    <message><source>The proprietary NVIDIA driver was installed successfully. Please restart the system.</source><translation>The proprietary NVIDIA driver was installed successfully. Please restart the system.</translation></message>
    <message><source>Switching to the open-source driver...</source><translation>Switching to the open-source driver...</translation></message>
    <message><source>Failed to remove proprietary packages: </source><translation>Failed to remove proprietary packages: </translation></message>
    <message><source>Open-source driver installation failed: </source><translation>Open-source driver installation failed: </translation></message>
    <message><source>The open-source driver (Nouveau) was installed. Please restart the system.</source><translation>The open-source driver (Nouveau) was installed. Please restart the system.</translation></message>
    <message><source>Removing the NVIDIA driver...</source><translation>Removing the NVIDIA driver...</translation></message>
    <message><source>Driver removed successfully.</source><translation>Driver removed successfully.</translation></message>
    <message><source>Removal failed: </source><translation>Removal failed: </translation></message>
    <message><source>Cleaning legacy driver leftovers...</source><translation>Cleaning legacy driver leftovers...</translation></message>
    <message><source>Deep clean completed.</source><translation>Deep clean completed.</translation></message>
    <message><source>Wayland detected: applying nvidia-drm.modeset=1...</source><translation>Wayland detected: applying nvidia-drm.modeset=1...</translation></message>
    <message><source>Failed to apply the Wayland kernel parameter: </source><translation>Failed to apply the Wayland kernel parameter: </translation></message>
    <message><source>X11 detected: checking NVIDIA userspace packages...</source><translation>X11 detected: checking NVIDIA userspace packages...</translation></message>
    <message><source>Failed to install the X11 NVIDIA package: </source><translation>Failed to install the X11 NVIDIA package: </translation></message>
</context>
<context>
    <name>NvidiaUpdater</name>
    <message><source>Updating the NVIDIA driver...</source><translation>Updating the NVIDIA driver...</translation></message>
    <message><source>Update failed: </source><translation>Update failed: </translation></message>
    <message><source>Rebuilding the kernel module...</source><translation>Rebuilding the kernel module...</translation></message>
    <message><source>Wayland detected: refreshing nvidia-drm.modeset=1...</source><translation>Wayland detected: refreshing nvidia-drm.modeset=1...</translation></message>
    <message><source>Driver updated successfully. Please restart the system.</source><translation>Driver updated successfully. Please restart the system.</translation></message>
</context>
<context>
    <name>SettingsPage</name>
    <message><source>Settings</source><translation>Settings</translation></message>
    <message><source>About</source><translation>About</translation></message>
    <message><source>Application: </source><translation>Application: </translation></message>
    <message><source>Theme: </source><translation>Theme: </translation></message>
    <message><source>System Dark</source><translation>System Dark</translation></message>
    <message><source>System Light</source><translation>System Light</translation></message>
</context>
</TS>
