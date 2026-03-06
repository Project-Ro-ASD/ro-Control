#pragma once

#include <QObject>
#include <QString>

// NvidiaDetector: Sistemdeki NVIDIA GPU ve sürücü durumunu tespit eder.
// Hiçbir kurulum yapmaz — sadece okur ve raporlar.
class NvidiaDetector : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool gpuFound READ gpuFound NOTIFY infoChanged)
  Q_PROPERTY(QString gpuName READ gpuName NOTIFY infoChanged)
  Q_PROPERTY(QString driverVersion READ driverVersion NOTIFY infoChanged)
  Q_PROPERTY(bool driverLoaded READ driverLoaded NOTIFY infoChanged)
  Q_PROPERTY(bool nouveauActive READ nouveauActive NOTIFY infoChanged)
  Q_PROPERTY(bool secureBootEnabled READ secureBootEnabled NOTIFY infoChanged)
  Q_PROPERTY(QString verificationReport READ verificationReport NOTIFY infoChanged)

public:
  struct GpuInfo {
    bool found = false;    // NVIDIA GPU var mı?
    QString name;          // GPU adı (ör: "NVIDIA GeForce RTX 3060")
    QString driverVersion; // Kurulu sürücü versiyonu (ör: "535.154.05")
    QString vbiosVersion;  // VBIOS versiyonu
    bool driverLoaded = false;     // nvidia.ko kernel modülü yüklü mü?
    bool nouveauActive = false;    // Nouveau (açık kaynak) sürücü aktif mi?
    bool secureBootEnabled = false; // UEFI Secure Boot açık mı?
  };

  explicit NvidiaDetector(QObject *parent = nullptr);

  bool gpuFound() const { return m_info.found; }
  QString gpuName() const { return m_info.name; }
  QString driverVersion() const { return m_info.driverVersion; }
  bool driverLoaded() const { return m_info.driverLoaded; }
  bool nouveauActive() const { return m_info.nouveauActive; }
  bool secureBootEnabled() const { return m_info.secureBootEnabled; }
  QString verificationReport() const;

  Q_INVOKABLE void refresh();

  // Tüm GPU bilgisini toplar ve döner
  GpuInfo detect() const;

  // Hızlı kontroller
  bool hasNvidiaGpu() const;
  bool isDriverInstalled() const;
  QString installedDriverVersion() const;

signals:
  void infoChanged();

private:
  // lspci çıktısından GPU adını çıkar
  QString detectGpuName() const;

  // nvidia-smi'den sürücü versiyonunu çıkar
  QString detectDriverVersion() const;

  // /proc/modules'dan modül durumunu kontrol et
  bool isModuleLoaded(const QString &moduleName) const;

  // mokutil ile Secure Boot durumunu kontrol et
  bool detectSecureBoot() const;

  GpuInfo m_info;
};
