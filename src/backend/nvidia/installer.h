#pragma once

#include <QObject>
#include <QString>

class CommandRunner;

// NvidiaInstaller: DNF üzerinden NVIDIA sürücü kurulum/kaldırma işlemleri.
// Tüm işlemler root gerektirir — polkit üzerinden yetki alınır.
class NvidiaInstaller : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool proprietaryAgreementRequired READ proprietaryAgreementRequired
                 NOTIFY proprietaryAgreementChanged)
  Q_PROPERTY(QString proprietaryAgreementText READ proprietaryAgreementText
                 NOTIFY proprietaryAgreementChanged)

public:
  explicit NvidiaInstaller(QObject *parent = nullptr);

  bool proprietaryAgreementRequired() const {
    return m_proprietaryAgreementRequired;
  }
  QString proprietaryAgreementText() const {
    return m_proprietaryAgreementText;
  }

  // Sozlesme durumunu yeniden kontrol et
  Q_INVOKABLE void refreshProprietaryAgreement();

  // Kapali kaynak kurulum (kullanici onayi bilgisiyle)
  Q_INVOKABLE void installProprietary(bool agreementAccepted);

  // Acik kaynak surucuye gecis/kurulum
  Q_INVOKABLE void installOpenSource();

  // Sürücüyü kur (akmod-nvidia)
  Q_INVOKABLE void install();

  // Sürücüyü kaldır
  Q_INVOKABLE void remove();

  // Eski sürücü kalıntılarını temizle
  Q_INVOKABLE void deepClean();

signals:
  // İşlem adımları — QML'e ilerleme göstermek için
  void progressMessage(const QString &message);

  void proprietaryAgreementChanged();

  // İşlem tamamlandı
  void installFinished(bool success, const QString &message);
  void removeFinished(bool success, const QString &message);

private:
  void setProprietaryAgreement(bool required, const QString &text);
  QString detectSessionType() const;
  bool applySessionSpecificSetup(CommandRunner &runner,
                                 const QString &sessionType,
                                 QString *errorMessage);

  bool m_proprietaryAgreementRequired = false;
  QString m_proprietaryAgreementText;
};
