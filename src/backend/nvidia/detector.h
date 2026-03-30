#pragma once

#include <QObject>
#include <QString>

// NvidiaDetector: Sistemdeki NVIDIA GPU ve surucu durumunu tespit eder.
class NvidiaDetector : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool gpuFound READ gpuFound NOTIFY infoChanged)
  Q_PROPERTY(QString gpuName READ gpuName NOTIFY infoChanged)
  Q_PROPERTY(QString displayAdapterName READ displayAdapterName NOTIFY
                 infoChanged)
  Q_PROPERTY(QString driverVersion READ driverVersion NOTIFY infoChanged)
  Q_PROPERTY(bool driverLoaded READ driverLoaded NOTIFY infoChanged)
  Q_PROPERTY(bool nouveauActive READ nouveauActive NOTIFY infoChanged)
  Q_PROPERTY(bool secureBootEnabled READ secureBootEnabled NOTIFY infoChanged)
  Q_PROPERTY(bool secureBootKnown READ secureBootKnown NOTIFY infoChanged)
  Q_PROPERTY(QString sessionType READ sessionType NOTIFY infoChanged)
  Q_PROPERTY(bool waylandSession READ waylandSession NOTIFY infoChanged)
  Q_PROPERTY(QString activeDriver READ activeDriver NOTIFY infoChanged)
  Q_PROPERTY(
      QString verificationReport READ verificationReport NOTIFY infoChanged)

public:
  struct GpuInfo {
    bool found = false;
    QString name;
    QString displayAdapterName;
    QString driverVersion;
    QString vbiosVersion;
    bool driverLoaded = false;
    bool nouveauActive = false;
    bool openKernelModulesInstalled = false;
    bool secureBootEnabled = false;
    bool secureBootKnown = false;
    QString sessionType;
  };

  explicit NvidiaDetector(QObject *parent = nullptr);

  bool gpuFound() const { return m_info.found; }
  QString gpuName() const { return m_info.name; }
  QString displayAdapterName() const { return m_info.displayAdapterName; }
  QString driverVersion() const { return m_info.driverVersion; }
  bool driverLoaded() const { return m_info.driverLoaded; }
  bool nouveauActive() const { return m_info.nouveauActive; }
  bool secureBootEnabled() const { return m_info.secureBootEnabled; }
  bool secureBootKnown() const { return m_info.secureBootKnown; }
  QString sessionType() const { return m_info.sessionType; }
  bool waylandSession() const {
    return m_info.sessionType.compare(QStringLiteral("wayland"),
                                      Qt::CaseInsensitive) == 0;
  }
  QString activeDriver() const;
  QString verificationReport() const;

  Q_INVOKABLE void refresh();

  GpuInfo detect() const;
  bool hasNvidiaGpu() const;
  bool isDriverInstalled() const;
  QString installedDriverVersion() const;

signals:
  void infoChanged();

private:
  QString detectGpuName() const;
  QString detectDisplayAdapterName() const;
  QString detectDriverVersion() const;
  bool isPackageInstalled(const QString &packageName) const;
  bool isModuleLoaded(const QString &moduleName) const;
  bool detectSecureBoot(bool *known = nullptr) const;

  GpuInfo m_info;
};
