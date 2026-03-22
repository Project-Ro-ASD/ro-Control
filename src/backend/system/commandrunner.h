#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

// CommandRunner: Tüm backend modüllerinin kullandığı shell komut çalıştırıcı.
// Hiçbir modül doğrudan sistem çağrısı yapmaz — hepsi bu sınıfı kullanır.
class CommandRunner : public QObject {
  Q_OBJECT

public:
  struct RunOptions {
    int startTimeoutMs = 3000;
    int timeoutMs = -1;
    int retries = 0;
    int retryDelayMs = 250;
  };

  struct Result {
    int exitCode;
    QString stdout;
    QString stderr;
    int attempt = 1;
    bool success() const { return exitCode == 0; }
  };

  explicit CommandRunner(QObject *parent = nullptr);
  static QString resolveProgramPath(const QString &program);

  // Bloklayan komut — sonuç dönene kadar bekler
  Result run(const QString &program, const QStringList &args = {});
  Result run(const QString &program, const QStringList &args,
             const RunOptions &options);

  // Root gerektiren komut — pkexec ile çalıştırır
  Result runAsRoot(const QString &program, const QStringList &args = {});
  Result runAsRoot(const QString &program, const QStringList &args,
                   const RunOptions &options);

signals:
  // Uzun işlemler için anlık çıktı (DNF install vb.)
  void outputLine(const QString &line);
  void errorLine(const QString &line);
  void commandStarted(const QString &program, const QStringList &args,
                      int attempt);
  void commandFinished(const QString &program, int exitCode, int attempt,
                       int elapsedMs);

private:
  QString resolveProgram(const QString &program) const;
  static QString overrideEnvironmentVariableName(const QString &program);
  Result runOnce(const QString &program, const QStringList &args,
                 const RunOptions &options, int attempt);
};
