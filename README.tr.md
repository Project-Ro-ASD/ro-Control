# ro-Control

<div align="center">

![ro-Control Logo](data/icons/hicolor/scalable/apps/ro-control.svg)

**Linux için Akıllı NVIDIA Sürücü Yöneticisi & Sistem Monitörü**

[![Lisans: GPL-3.0](https://img.shields.io/badge/lisans-GPL--3.0-blue?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Fedora%2040%2B-51A2DA?style=flat-square)](https://getfedora.org/)
[![Qt6 ile yapıldı](https://img.shields.io/badge/Qt6%20%2B%20QML-41CD52?style=flat-square)](https://www.qt.io/)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square)](https://isocpp.org/)

[Özellikler](#özellikler) • [Kurulum](#kurulum) • [Derleme](#kaynaktan-derleme) • [Katkı](#katkıda-bulunma) • [Lisans](#lisans)

[![README in English](https://img.shields.io/badge/README-English-blue?style=flat-square)](README.md)

</div>

---

ro-Control, **C++20** ve **Qt6/QML** ile geliştirilmiş, Fedora Linux üzerinde NVIDIA GPU sürücü yönetimini ve sistem izlemeyi kolaylaştıran native bir KDE Plasma masaüstü uygulamasıdır. Sürücülerin kurulumu, güncellenmesi ve izlenmesi için modern, Plasma'ya uyumlu bir arayüz sunar; güvenli yetki yükseltme için PolicyKit entegrasyonu içerir.

## Özellikler

### 🚀 Sürücü Yönetimi
- **Tek tıkla kurulum** — RPM Fusion üzerinden NVIDIA sürücü kurulumu (`akmod-nvidia`)
- **Sürücü güncelleme** — Yeni sürücü versiyonlarını tespit et ve uygula
- **Temiz kaldırma** — Çakışmaları önlemek için eski sürücü dosyalarını temizle
- **Güvenli Önyükleme** — İmzasız kernel modülleri için tespit ve uyarı

### 📊 Canlı Sistem Monitörü
- Gerçek zamanlı GPU sıcaklığı, yük ve VRAM kullanımı
- CPU yükü ve sıcaklık takibi
- RAM kullanım izleme
- Renk kodlu ilerleme göstergeleri

### 🖥 Ekran & Sistem
- **Wayland desteği** — Otomatik `nvidia-drm.modeset=1` GRUB yapılandırması
- **Hibrit grafik** — NVIDIA, Intel ve On-Demand modları arasında geçiş
- **PolicyKit entegrasyonu** — Root olarak çalıştırmadan güvenli yetki yükseltme

### 🌍 Çok Dil Desteği
- Türkçe ve İngilizce arayüz
- Genişletilebilir çeviri sistemi

## Ekran Görüntüleri

> Ekran görüntüleri ilk UI milestone'ından sonra eklenecektir.

## Kurulum

### Fedora (RPM) — Önerilen

[Releases](https://github.com/Acik-Kaynak-Gelistirme-Toplulugu/ro-Control/releases) sayfasından en son `.rpm` dosyasını indirin:

```bash
sudo dnf install ./ro-control-*.rpm
```

### Kaynaktan Derleme

Tam talimatlar için [docs/BUILDING.md](docs/BUILDING.md) dosyasına bakın.

**Hızlı başlangıç:**

```bash
# Bağımlılıkları kur (Fedora 40+)
sudo dnf install cmake extra-cmake-modules gcc-c++ \
  qt6-qtbase-devel \
  qt6-qtdeclarative-devel \
  qt6-qtwayland-devel \
  kf6-qqc2-desktop-style

# Klonla ve derle
git clone https://github.com/Acik-Kaynak-Gelistirme-Toplulugu/ro-Control.git
cd ro-Control
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Yükle
sudo make install
```

## Proje Yapısı

```
ro-Control/
├── src/
│   ├── backend/          # C++ iş mantığı
│   │   ├── nvidia/       # Sürücü tespiti, kurulum, güncelleme
│   │   ├── monitor/      # GPU/CPU/RAM istatistikleri
│   │   └── system/       # Polkit, DNF, komut çalıştırıcı
│   ├── qml/              # Qt Quick arayüzü
│   │   ├── pages/        # Ana uygulama sayfaları
│   │   └── components/   # Tekrar kullanılabilir UI bileşenleri
│   └── main.cpp
├── data/                 # İkonlar, .desktop, PolicyKit, AppStream
├── packaging/rpm/        # Fedora RPM spec
├── docs/                 # Mimari ve derleme dökümanları
├── tests/                # Birim testleri
└── CMakeLists.txt
```

## Katkıda Bulunma

Katkılarınızı bekliyoruz! Pull request göndermeden önce lütfen [CONTRIBUTING.md](CONTRIBUTING.md) dosyasını okuyun.
Surum akis detaylari icin [docs/RELEASE.md](docs/RELEASE.md) dosyasina bakin.
Yerellestirme altyapisi icin [i18n/README.md](i18n/README.md) dosyasini inceleyin.

Hızlı katkı akışı:

```bash
git checkout dev
git checkout -b feature/ozellik-adi
# ... değişikliklerinizi yapın ...
git commit -m "feat: değişikliğinizi açıklayın"
git push origin feature/ozellik-adi
# Pull Request açın → dev branch'ine
```

## Gereksinimler

| Bileşen | Minimum Versiyon |
|---------|-----------------|
| Fedora  | 40+             |
| Qt      | 6.6+            |
| CMake   | 3.22+           |
| GCC     | 13+ (C++20)     |
| GPU     | NVIDIA (herhangi) |

## Lisans

Bu proje [GNU Genel Kamu Lisansı v3.0](LICENSE) ile lisanslanmıştır.