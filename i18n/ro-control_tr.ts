<?xml version="1.0" encoding="utf-8"?>
<TS version="2.1" language="tr" sourcelanguage="en">
<context>
    <name>DriverPage</name>
    <message><source>Driver Management</source><translation>Sürücü Yönetimi</translation></message>
    <message><source>GPU: </source><translation>GPU: </translation></message>
    <message><source>Not detected</source><translation>Tespit edilmedi</translation></message>
    <message><source>Active driver: </source><translation>Aktif sürücü: </translation></message>
    <message><source>Driver version: </source><translation>Sürücü sürümü: </translation></message>
    <message><source>None</source><translation>Yok</translation></message>
    <message><source>Secure Boot: </source><translation>Secure Boot: </translation></message>
    <message><source>Enabled</source><translation>Açık</translation></message>
    <message><source>Disabled / Unknown</source><translation>Kapalı / Bilinmiyor</translation></message>
    <message><source>Session type: </source><translation>Oturum türü: </translation></message>
    <message><source>For Wayland sessions, nvidia-drm.modeset=1 is applied automatically.</source><translation>Wayland oturumlarında nvidia-drm.modeset=1 otomatik uygulanır.</translation></message>
    <message><source>For X11 sessions, the xorg-x11-drv-nvidia package is verified and installed.</source><translation>X11 oturumlarında xorg-x11-drv-nvidia paketi doğrulanır ve kurulur.</translation></message>
    <message><source>I accept the license / agreement terms</source><translation>Lisans / sözleşme koşullarını kabul ediyorum</translation></message>
    <message><source>Install Proprietary Driver</source><translation>Kapalı Kaynak Sürücüyü Kur</translation></message>
    <message><source>Install Open-Source Driver (Nouveau)</source><translation>Açık Kaynak Sürücüyü Kur (Nouveau)</translation></message>
    <message><source>Deep Clean</source><translation>Derin Temizlik</translation></message>
    <message><source>Check for Updates</source><translation>Güncellemeleri Kontrol Et</translation></message>
    <message><source>Apply Update</source><translation>Güncellemeyi Uygula</translation></message>
    <message><source>Latest version: </source><translation>En son sürüm: </translation></message>
    <message><source>Rescan</source><translation>Yeniden Tara</translation></message>
    <message><source>Installed NVIDIA version: </source><translation>Kurulu NVIDIA sürümü: </translation></message>
</context>
<context>
    <name>Main</name>
    <message><source>ro-Control</source><translation>ro-Control</translation></message>
    <message><source>Theme: System (Dark)</source><translation>Tema: Sistem (Koyu)</translation></message>
    <message><source>Theme: System (Light)</source><translation>Tema: Sistem (Açık)</translation></message>
    <message><source>Driver</source><translation>Sürücü</translation></message>
    <message><source>Monitor</source><translation>İzleme</translation></message>
    <message><source>Settings</source><translation>Ayarlar</translation></message>
</context>
<context>
    <name>MonitorPage</name>
    <message><source>System Monitoring</source><translation>Sistem İzleme</translation></message>
    <message><source>CPU</source><translation>CPU</translation></message>
    <message><source>Usage: </source><translation>Kullanım: </translation></message>
    <message><source>CPU data unavailable</source><translation>CPU verisi alınamıyor</translation></message>
    <message><source>Temperature: </source><translation>Sıcaklık: </translation></message>
    <message><source>GPU (NVIDIA)</source><translation>GPU (NVIDIA)</translation></message>
    <message><source>NVIDIA GPU</source><translation>NVIDIA GPU</translation></message>
    <message><source>Failed to read data via nvidia-smi</source><translation>nvidia-smi üzerinden veri okunamadı</translation></message>
    <message><source>Load: </source><translation>Yük: </translation></message>
    <message><source>VRAM: </source><translation>VRAM: </translation></message>
    <message><source>RAM</source><translation>RAM</translation></message>
    <message><source>RAM data unavailable</source><translation>RAM verisi alınamıyor</translation></message>
    <message><source>Refresh</source><translation>Yenile</translation></message>
    <message><source>Refresh interval: </source><translation>Yenileme aralığı: </translation></message>
</context>
<context>
    <name>NvidiaDetector</name>
    <message><source>Proprietary (NVIDIA)</source><translation>Kapalı Kaynak (NVIDIA)</translation></message>
    <message><source>Open Source (Nouveau)</source><translation>Açık Kaynak (Nouveau)</translation></message>
    <message><source>Not Installed / Unknown</source><translation>Kurulu Değil / Bilinmiyor</translation></message>
    <message><source>GPU: %1
Driver Version: %2
Secure Boot: %3
Session: %4
NVIDIA Module: %5
Nouveau: %6</source><translation>GPU: %1
Sürücü Sürümü: %2
Secure Boot: %3
Oturum: %4
NVIDIA Modülü: %5
Nouveau: %6</translation></message>
    <message><source>Unknown</source><translation>Bilinmiyor</translation></message>
    <message><source>Loaded</source><translation>Yüklü</translation></message>
    <message><source>Not loaded</source><translation>Yüklü değil</translation></message>
    <message><source>Active</source><translation>Aktif</translation></message>
    <message><source>Inactive</source><translation>Aktif değil</translation></message>
</context>
<context>
    <name>NvidiaInstaller</name>
    <message><source>You must accept the NVIDIA proprietary driver license terms before installation. Detected license: %1</source><translation>Kurulumdan önce NVIDIA kapalı kaynak sürücü lisans koşullarını kabul etmeniz gerekir. Tespit edilen lisans: %1</translation></message>
    <message><source>License agreement acceptance is required before installation.</source><translation>Kurulumdan önce lisans sözleşmesinin kabul edilmesi gerekir.</translation></message>
    <message><source>Checking RPM Fusion repositories...</source><translation>RPM Fusion depoları kontrol ediliyor...</translation></message>
    <message><source>Platform version could not be detected.</source><translation>Platform sürümü tespit edilemedi.</translation></message>
    <message><source>Failed to enable RPM Fusion repositories: </source><translation>RPM Fusion depoları etkinleştirilemedi: </translation></message>
    <message><source>Installing the proprietary NVIDIA driver (akmod-nvidia)...</source><translation>Kapalı kaynak NVIDIA sürücüsü kuruluyor (akmod-nvidia)...</translation></message>
    <message><source>Installation failed: </source><translation>Kurulum başarısız: </translation></message>
    <message><source>Building the kernel module (akmods --force)...</source><translation>Kernel modülü derleniyor (akmods --force)...</translation></message>
    <message><source>The proprietary NVIDIA driver was installed successfully. Please restart the system.</source><translation>Kapalı kaynak NVIDIA sürücüsü başarıyla kuruldu. Lütfen sistemi yeniden başlatın.</translation></message>
    <message><source>Switching to the open-source driver...</source><translation>Açık kaynak sürücüye geçiliyor...</translation></message>
    <message><source>Failed to remove proprietary packages: </source><translation>Kapalı kaynak paketler kaldırılamadı: </translation></message>
    <message><source>Open-source driver installation failed: </source><translation>Açık kaynak sürücü kurulumu başarısız: </translation></message>
    <message><source>The open-source driver (Nouveau) was installed. Please restart the system.</source><translation>Açık kaynak sürücü (Nouveau) kuruldu. Lütfen sistemi yeniden başlatın.</translation></message>
    <message><source>Removing the NVIDIA driver...</source><translation>NVIDIA sürücüsü kaldırılıyor...</translation></message>
    <message><source>Driver removed successfully.</source><translation>Sürücü başarıyla kaldırıldı.</translation></message>
    <message><source>Removal failed: </source><translation>Kaldırma başarısız: </translation></message>
    <message><source>Cleaning legacy driver leftovers...</source><translation>Eski sürücü kalıntıları temizleniyor...</translation></message>
    <message><source>Deep clean completed.</source><translation>Derin temizlik tamamlandı.</translation></message>
    <message><source>Wayland detected: applying nvidia-drm.modeset=1...</source><translation>Wayland tespit edildi: nvidia-drm.modeset=1 uygulanıyor...</translation></message>
    <message><source>Failed to apply the Wayland kernel parameter: </source><translation>Wayland kernel parametresi uygulanamadı: </translation></message>
    <message><source>X11 detected: checking NVIDIA userspace packages...</source><translation>X11 tespit edildi: NVIDIA userspace paketleri kontrol ediliyor...</translation></message>
    <message><source>Failed to install the X11 NVIDIA package: </source><translation>X11 NVIDIA paketi kurulamadı: </translation></message>
</context>
<context>
    <name>NvidiaUpdater</name>
    <message><source>Updating the NVIDIA driver...</source><translation>NVIDIA sürücüsü güncelleniyor...</translation></message>
    <message><source>Update failed: </source><translation>Güncelleme başarısız: </translation></message>
    <message><source>Rebuilding the kernel module...</source><translation>Kernel modülü yeniden derleniyor...</translation></message>
    <message><source>Wayland detected: refreshing nvidia-drm.modeset=1...</source><translation>Wayland tespit edildi: nvidia-drm.modeset=1 yenileniyor...</translation></message>
    <message><source>Driver updated successfully. Please restart the system.</source><translation>Sürücü başarıyla güncellendi. Lütfen sistemi yeniden başlatın.</translation></message>
</context>
<context>
    <name>SettingsPage</name>
    <message><source>Settings</source><translation>Ayarlar</translation></message>
    <message><source>About</source><translation>Hakkında</translation></message>
    <message><source>Application: </source><translation>Uygulama: </translation></message>
    <message><source>Theme: </source><translation>Tema: </translation></message>
    <message><source>System Dark</source><translation>Sistem Koyu</translation></message>
    <message><source>System Light</source><translation>Sistem Açık</translation></message>
</context>
</TS>
