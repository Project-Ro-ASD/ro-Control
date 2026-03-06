#pragma once

#include <QObject>
#include <QString>

// PolkitHelper: PolicyKit D-Bus arayüzü üzerinden yetki kontrolü.
// Komut çalıştırmaz — sadece yetkinin verilip verilmediğini kontrol eder.
// Gerçek komut yürütme CommandRunner::runAsRoot() ile yapılır.
class PolkitHelper : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool authorized READ isAuthorized NOTIFY authorizedChanged)

public:
  // ro-Control için tanımlı PolicyKit action ID
  static constexpr auto ActionId =
      "com.github.AcikKaynakGelistirmeToplulugu.rocontrol.manage-drivers";

  explicit PolkitHelper(QObject *parent = nullptr);

  // Mevcut kullanıcının yetki durumunu sorgula
  bool isAuthorized() const { return m_authorized; }

  // PolicyKit üzerinden yetki iste (D-Bus CheckAuthorization)
  Q_INVOKABLE bool requestAuthorization();

  // Mevcut yetkiyi kontrol et (dialog göstermeden)
  Q_INVOKABLE bool checkAuthorization();

signals:
  void authorizedChanged();

private:
  bool m_authorized = false;
};
