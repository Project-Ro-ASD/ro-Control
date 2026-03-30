# ro-Control

<div align="center">

![ro-Control Logo](data/icons/hicolor/scalable/apps/ro-control.svg)

**Linux için Akıllı NVIDIA Sürücü Yöneticisi & Sistem Monitörü**

[![Lisans: GPL-3.0](https://img.shields.io/badge/lisans-GPL--3.0-blue?style=flat-square)](LICENSE)
[![Qt6 ile yapıldı](https://img.shields.io/badge/Qt6%20%2B%20QML-41CD52?style=flat-square)](https://www.qt.io/)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square)](https://isocpp.org/)

[Özellikler](#özellikler) • [Kurulum](#kurulum) • [Derleme](#kaynaktan-derleme) • [Katkı](#katkıda-bulunma) • [Lisans](#lisans)

[![README in English](https://img.shields.io/badge/README-English-blue?style=flat-square)](README.md)

</div>

---

ro-Control, **C++20** ve **Qt6/QML** ile geliştirilmiş, Linux üzerinde NVIDIA GPU sürücü yönetimini ve sistem izlemeyi kolaylaştıran native bir KDE Plasma masaüstü uygulamasıdır. Sürücülerin kurulumu, güncellenmesi ve izlenmesi için modern, Plasma'ya uyumlu bir arayüz sunar; güvenli yetki yükseltme için PolicyKit entegrasyonu içerir.

## Proje Durumu

ro-Control, Fedora odaklı NVIDIA sürücü iş akışları için aktif geliştirilen bir masaüstü yardımcı uygulamasıdır ve birincil hedef ortam Fedora KDE Desktop'tır.
Mevcut kod tabanı özellikle şu alanlara odaklanır:

- Script sarmalayıcıları yerine yerel Qt/QML masaüstü deneyimi
- PolicyKit ve DNF üzerinden güvenli sürücü yaşam döngüsü işlemleri
- GPU, CPU ve RAM telemetrisi için pratik tanılama araçları
- İngilizce kaynak metinler ve tam Türkçe çalışma zamanı yerelleştirmesi
- Kalıcı arayüz tercihleri ve açık `Sistem / Açık / Koyu` tema seçimi

Şu anda **hibrit grafik geçişi**, fan kontrolü veya overclock özellikleri sunmaz.

## Bu Depo Neden Var

ro-Control, daha geniş **Project Ro ASD / ro-ASD OS** ekosistemi içinde NVIDIA işlemleri ve tanılama yüzeyi olarak konumlanır. Depo yapısı şu amaçlara uygundur:

- Organizasyon profilinde öne çıkan bir masaüstü aracı olarak sergilenmek
- İşletim sistemi imajından bağımsız şekilde derlenip paketlenebilmek
- Hem GUI hem de CLI üzerinden kullanılabilmek
- Backend, frontend, paketleme ve çeviri katmanlarında temiz şekilde genişletilebilmek

## Özellikler

### 🚀 Sürücü Yönetimi
- **Tek tıkla kurulum** — RPM Fusion üzerinden NVIDIA sürücü kurulumu (`akmod-nvidia`)
- **Sürücü güncelleme** — Yeni sürücü versiyonlarını tespit et ve uygula
- **Temiz kaldırma** — Çakışmaları önlemek için eski sürücü dosyalarını temizle
- **Güvenli Önyükleme** — İmzasız kernel modülleri için tespit ve uyarı

### 📊 Canlı Sistem Monitörü
- `nvidia-smi` erişilebildiğinde gerçek zamanlı GPU sıcaklığı, yük ve VRAM kullanımı
- sysfs, hwmon ve `sensors` üzerinden CPU yükü ve sıcaklık takibi
- `/proc/meminfo` ve gerektiğinde `free` fallback'i ile RAM kullanım izleme
- Renk kodlu ilerleme göstergeleri

### 🖥 Ekran & Sistem
- **Wayland desteği** — Otomatik `nvidia-drm.modeset=1` GRUB yapılandırması
- **PolicyKit entegrasyonu** — Root olarak çalıştırmadan güvenli yetki yükseltme
- **Kalıcı kabuk tercihleri** — Kayıtlı tema modu, yoğunluk ve tanılama görünürlüğü

### 🌍 Çok Dil Desteği
- Qt çeviri sistemi (`.ts` / `.qm`) ile çalışma zamanı yerelleştirme
- Dağıtıma giren çalışma zamanı dilleri: İngilizce ve Türkçe
- Yeni diller için genişletilebilir iş akışı

### 🧰 CLI Desteği
- `ro-control help` kullanım bilgisini gösterir
- `ro-control version` uygulama sürümünü gösterir
- `ro-control status` kısa sistem ve sürücü durumunu gösterir
- `ro-control diagnostics --json` makine tarafından işlenebilir tanı çıktısı üretir
- `ro-control driver install|remove|update|deep-clean` scriptlenebilir sürücü yönetimi sunar
- Kurulumla birlikte `man ro-control` sayfası ve Bash/Zsh/Fish completion dosyaları gelir

### ✅ Test Kapsamı
- Detector, updater, monitor, preferences, CLI ve sistem entegrasyonu için backend testleri
- `DriverPage` durum senkronizasyonu için QML entegrasyon testi
- Dağıtıma giren diller için translation release target doğrulaması

## Mevcut Kapsam

Bugün iyi desteklenenler:
- Fedora odaklı NVIDIA sürücü kurulum, güncelleme ve temizlik iş akışları
- Sürücü durumu denetimi ve tanılama çıktıları
- CPU/GPU/RAM canlı durumunu gösteren monitor paneli
- Paketleme metadatası, shell completion dosyaları ve man page desteği

Şimdilik kapsam dışı olanlar:
- Windows desteği
- Qt dışı frontendler
- İleri seviye GPU tuning veya oyun içi overlay özellikleri

## Ekran Görüntüleri

Önizleme görselleri [`docs/screenshots/`](docs/screenshots/) altında bulunur.
Daha geniş mağaza / distro dağıtımı öncesinde PNG ekran görüntüleri eklenmelidir.

## Kurulum

### RPM Paketi

[Releases](https://github.com/Project-Ro-ASD/ro-Control/releases) sayfasından sistem mimarinize uygun en güncel Fedora `.rpm` paketini indirin (`x86_64` = 64-bit x86 sistemler, `aarch64` = ARM64 sistemler). Ortak dosyalar eşlik eden `noarch` RPM içinde gelir:

```bash
sudo dnf install ./ro-control-*.<arch>.rpm ./ro-control-common-*.noarch.rpm
```

### Kaynaktan Derleme

Tam talimatlar için [docs/BUILDING.md](docs/BUILDING.md) dosyasına bakın.

Fedora hızlı kurulum:

```bash
./scripts/fedora-bootstrap.sh
```

GitHub Releases yalnızca `x86_64`, `aarch64`, `noarch` ve `src` RPM çıktıları yayınlar.

Fedora çalışma notları için [docs/FEDORA.md](docs/FEDORA.md) dosyasına bakın.

### CLI Hızlı Örnekler

```bash
ro-control help
ro-control version
ro-control status
ro-control diagnostics --json
ro-control driver install --proprietary --accept-license
ro-control driver update
```

**Hızlı başlangıç:**

```bash
# Bağımlılıkları kur
sudo dnf install cmake extra-cmake-modules gcc-c++ \
  qt6-qtbase-devel \
  qt6-qtdeclarative-devel \
  qt6-qttools-devel \
  qt6-qtwayland-devel \
  kf6-qqc2-desktop-style \
  polkit-devel

# Klonla ve derle
git clone https://github.com/Project-Ro-ASD/ro-Control.git
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
├── packaging/rpm/        # RPM paketleme
├── docs/                 # Mimari ve derleme dökümanları
├── tests/                # Birim testleri
└── CMakeLists.txt
```

## Katkıda Bulunma

Katkılarınızı bekliyoruz! Pull request göndermeden önce lütfen [CONTRIBUTING.md](CONTRIBUTING.md) dosyasını okuyun.
Sürüm akış detayları için [docs/RELEASE.md](docs/RELEASE.md) dosyasına bakın.
Yerelleştirme altyapısı için [i18n/README.md](i18n/README.md) dosyasını inceleyin.
Mimari detaylar için [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) dosyasına bakın.
Kullanım desteği ve issue yönlendirmesi için [SUPPORT.md](SUPPORT.md) dosyasına bakın.

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
| Qt      | 6.6+            |
| CMake   | 3.22+           |
| GCC     | 13+ (C++20)     |
| GPU     | NVIDIA (herhangi) |

## Lisans

Bu proje [GNU Genel Kamu Lisansı v3.0](LICENSE) ile lisanslanmıştır.
