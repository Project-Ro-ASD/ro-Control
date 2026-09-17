#pragma once

#include <QObject>
#include <QString>
#include <atomic>
#include <functional>
#include <memory>

class CommandRunner;

// NvidiaInstaller manages NVIDIA driver install and removal flows via DNF.
// Privileged operations are routed through polkit-backed helper execution.
class NvidiaInstaller : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool proprietaryAgreementRequired READ proprietaryAgreementRequired
                 NOTIFY proprietaryAgreementChanged)
  Q_PROPERTY(QString proprietaryAgreementText READ proprietaryAgreementText
                 NOTIFY proprietaryAgreementChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
  explicit NvidiaInstaller(QObject *parent = nullptr);

  bool proprietaryAgreementRequired() const {
    return m_proprietaryAgreementRequired;
  }
  QString proprietaryAgreementText() const {
    return m_proprietaryAgreementText;
  }
  bool busy() const { return m_busy; }

  // Refresh the proprietary driver agreement state.
  Q_INVOKABLE void refreshProprietaryAgreement();

  // Install the proprietary driver path after an explicit user confirmation.
  Q_INVOKABLE void installProprietary(bool agreementAccepted);

  // Switch to the community open-source graphics stack.
  Q_INVOKABLE void installOpenSource();

  // Convenience wrapper for the default install path.
  Q_INVOKABLE void install();

  // Remove installed NVIDIA packages.
  Q_INVOKABLE void remove();

  // Remove legacy NVIDIA leftovers.
  Q_INVOKABLE void deepClean();

  // Rebuild NVIDIA kernel modules via akmods and dracut.
  Q_INVOKABLE void rebuildKernelModules();

  // Best-effort cancellation for the active install or removal flow.
  Q_INVOKABLE void cancelOperation();

signals:
  // Progress updates surfaced to QML.
  void progressMessage(const QString &message);

  void proprietaryAgreementChanged();
  void busyChanged();

  // Completion signals for the top-level driver operations.
  void installFinished(bool success, const QString &message);
  void removeFinished(bool success, const QString &message);

private:
  void setBusy(bool busy);
  void runAsyncTask(const std::function<void()> &task);
  void setProprietaryAgreement(bool required, const QString &text);

  bool m_proprietaryAgreementRequired = false;
  QString m_proprietaryAgreementText;
  bool m_busy = false;
  std::shared_ptr<std::atomic_bool> m_cancelRequested;
};
