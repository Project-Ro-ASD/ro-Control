#include "guiinstance.h"

#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QLocalSocket>
#include <QThread>
#include <QWindow>

GuiInstance::GuiInstance(const QString &directory, QObject *parent)
    : QObject(parent), m_lock(directory + QStringLiteral("/gui.lock")),
      m_directory(directory),
      m_serverName(QStringLiteral("ro-control-gui-") +
                   QString::fromLatin1(
                       QCryptographicHash::hash(directory.toUtf8(),
                                                QCryptographicHash::Sha256)
                           .toHex()
                           .left(32))) {
  m_lock.setStaleLockTime(0);
  m_server.setSocketOptions(QLocalServer::UserAccessOption);
  connect(&m_server, &QLocalServer::newConnection, this, [this]() {
    while (auto *socket = m_server.nextPendingConnection()) {
      socket->setParent(this);
      socket->setReadBufferSize(64);
      auto receive = [this, socket]() {
        if (!socket->canReadLine())
          return;
        if (socket->readLine(64) == QByteArrayLiteral("activate\n")) {
          emit activationRequested();
          activateWindow();
        }
        socket->disconnectFromServer();
      };
      connect(socket, &QLocalSocket::readyRead, this, receive);
      connect(socket, &QLocalSocket::disconnected, socket,
              &QObject::deleteLater);
      receive();
    }
  });
}

GuiInstance::Result GuiInstance::start() {
  if (!QDir().mkpath(m_directory)) {
    m_error = QStringLiteral("Cannot create GUI instance directory.");
    return Result::Error;
  }
  if (!m_lock.tryLock()) {
    if (m_lock.error() != QLockFile::LockFailedError) {
      m_error = QStringLiteral("Cannot acquire GUI instance lock.");
      return Result::Error;
    }
    QElapsedTimer deadline;
    deadline.start();
    while (deadline.elapsed() < 2000) {
      QLocalSocket socket;
      socket.connectToServer(m_serverName);
      if (socket.waitForConnected(100)) {
        socket.write("activate\n");
        if (socket.bytesToWrite() == 0 || socket.waitForBytesWritten(1000)) {
          socket.disconnectFromServer();
          return Result::ActivatedExisting;
        }
      }
      QThread::msleep(25);
    }
    m_error = QStringLiteral("The running GUI could not be activated.");
    return Result::Error;
  }

  // Yalnızca kilidin sahibi çökmüş süreçten kalan soketi temizleyebilir.
  QLocalServer::removeServer(m_serverName);
  if (!m_server.listen(m_serverName)) {
    m_error = m_server.errorString();
    m_lock.unlock();
    return Result::Error;
  }
  return Result::Primary;
}

QString GuiInstance::errorString() const { return m_error; }

void GuiInstance::setWindow(QWindow *window) {
  m_window = window;
  if (m_pendingActivation)
    activateWindow();
}

void GuiInstance::activateWindow() {
  if (!m_window) {
    m_pendingActivation = true;
    return;
  }
  m_pendingActivation = false;
  if (m_window->visibility() == QWindow::Minimized)
    m_window->showNormal();
  else
    m_window->show();
  m_window->raise();
  m_window->requestActivate();
}
