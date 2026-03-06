#pragma once

#include <QObject>
#include <QString>

// NvidiaDetector: Sistemdeki NVIDIA GPU ve sürücü durumunu tespit eder.
// Hiçbir kurulum yapmaz — sadece okur ve raporlar.
class NvidiaDetector : public QObject {
  Q_OBJECT

public:
  struct GpuInfo {
    bool found;            // NVIDIA GPU var mı?
    QString name;          // GPU adı (ör: "NVIDIA GeForce RTX 3060")
    QString driverVersion; // Kurulu sürücü versiyonu (ör: "535.154.05")
    QString vbiosVersion;  // VBIOS versiyonu
    bool driverLoaded;     // nvidia.ko kernel modülü yüklü mü?
    bool nouveauActive;    // Nouveau (açık kaynak) sürücü aktif mi?
  };

  explicit NvidiaDetector(QObject *parent = nullptr);

  // Tüm GPU bilgisini toplar ve döner
  GpuInfo detect() const;

  // Hızlı kontroller
  bool hasNvidiaGpu() const;
  bool isDriverInstalled() const;
  QString installedDriverVersion() const;

private:
  // lspci çıktısından GPU adını çıkar
  QString detectGpuName() const;

  // nvidia-smi'den sürücü versiyonunu çıkar
  QString detectDriverVersion() const;

  // /proc/modules'dan modül durumunu kontrol et
  bool isModuleLoaded(const QString &moduleName) const;
};
