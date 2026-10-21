#pragma once

#include <QLocalServer>
#include <QLockFile>
#include <QObject>
#include <QPointer>

class QWindow;

class GuiInstance : public QObject {
  Q_OBJECT

public:
  enum class Result { Primary, ActivatedExisting, Error };

  explicit GuiInstance(const QString &directory, QObject *parent = nullptr);
  Result start();
  QString errorString() const;
  void setWindow(QWindow *window);
  void activateWindow();

signals:
  void activationRequested();

private:
  QLockFile m_lock;
  QLocalServer m_server;
  QString m_directory;
  QString m_serverName;
  QString m_error;
  QPointer<QWindow> m_window;
  bool m_pendingActivation = false;
};
