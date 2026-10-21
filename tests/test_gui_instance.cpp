#include <QFile>
#include <QGuiApplication>
#include <QProcess>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>
#include <QWindow>

#include <memory>
#include <vector>

#include "backend/system/guiinstance.h"

class TestGuiInstance : public QObject {
  Q_OBJECT

private slots:
  void restoresHiddenAndMinimizedWindow();
  void queuesActivationUntilWindowExists();
  void excludesOtherProcessesAndRecoversAfterCrash();
  void simultaneousLaunchesChooseOneOwner();
  void rejectsUnusableLockDirectory();
};

void TestGuiInstance::restoresHiddenAndMinimizedWindow() {
  QTemporaryDir directory;
  QVERIFY(directory.isValid());
  GuiInstance primary(directory.path());
  const auto result = primary.start();
  QVERIFY2(result == GuiInstance::Result::Primary,
           qPrintable(primary.errorString()));
  QWindow window;
  primary.setWindow(&window);
  QSignalSpy activated(&primary, &GuiInstance::activationRequested);
  {
    GuiInstance secondary(directory.path());
    QCOMPARE(secondary.start(), GuiInstance::Result::ActivatedExisting);
  }
  QTRY_COMPARE(activated.count(), 1);
  QTRY_VERIFY(window.isVisible());
  window.showMinimized();
  {
    GuiInstance secondary(directory.path());
    QCOMPARE(secondary.start(), GuiInstance::Result::ActivatedExisting);
  }
  QTRY_COMPARE(activated.count(), 2);
  QTRY_VERIFY(window.visibility() != QWindow::Minimized);
}

void TestGuiInstance::queuesActivationUntilWindowExists() {
  QTemporaryDir directory;
  GuiInstance primary(directory.path());
  const auto result = primary.start();
  QVERIFY2(result == GuiInstance::Result::Primary,
           qPrintable(primary.errorString()));
  QSignalSpy activated(&primary, &GuiInstance::activationRequested);
  GuiInstance secondary(directory.path());
  QCOMPARE(secondary.start(), GuiInstance::Result::ActivatedExisting);
  QTRY_COMPARE(activated.count(), 1);
  QWindow window;
  QVERIFY(!window.isVisible());
  primary.setWindow(&window);
  QVERIFY(window.isVisible());
}

void TestGuiInstance::excludesOtherProcessesAndRecoversAfterCrash() {
  QTemporaryDir directory;
  QProcess first;
  first.start(QCoreApplication::applicationFilePath(),
              {"--instance-probe", directory.path(), "hold"});
  QVERIFY(first.waitForStarted());
  QVERIFY(first.waitForReadyRead());
  QCOMPARE(first.readAllStandardOutput().trimmed(), QByteArray("primary"));
  for (int i = 0; i < 3; ++i) {
    QProcess second;
    second.start(QCoreApplication::applicationFilePath(),
                 {"--instance-probe", directory.path(), "once"});
    QVERIFY(second.waitForFinished(5000));
    QCOMPARE(second.exitStatus(), QProcess::NormalExit);
    QCOMPARE(second.exitCode(), 10);
    QCOMPARE(first.state(), QProcess::Running);
  }
  first.kill();
  QVERIFY(first.waitForFinished());
  QProcess recovered;
  recovered.start(QCoreApplication::applicationFilePath(),
                  {"--instance-probe", directory.path(), "once"});
  QVERIFY(recovered.waitForFinished(5000));
  QCOMPARE(recovered.exitCode(), 0);
  QCOMPARE(recovered.readAllStandardOutput().trimmed(), QByteArray("primary"));
}

void TestGuiInstance::simultaneousLaunchesChooseOneOwner() {
  QTemporaryDir directory;
  std::vector<std::unique_ptr<QProcess>> processes;
  for (int i = 0; i < 6; ++i) {
    auto process = std::make_unique<QProcess>();
    process->start(QCoreApplication::applicationFilePath(),
                   {"--instance-probe", directory.path(), "hold"});
    QVERIFY(process->waitForStarted());
    processes.push_back(std::move(process));
  }
  auto runningCount = [&processes]() {
    int running = 0;
    for (const auto &process : processes)
      running += process->state() != QProcess::NotRunning;
    return running;
  };
  QTRY_COMPARE_WITH_TIMEOUT(runningCount(), 1, 5000);
  for (const auto &process : processes) {
    if (process->state() == QProcess::Running) {
      process->kill();
      QVERIFY(process->waitForFinished());
    } else {
      QCOMPARE(process->exitStatus(), QProcess::NormalExit);
      QCOMPARE(process->exitCode(), 10);
    }
  }
}

void TestGuiInstance::rejectsUnusableLockDirectory() {
  QTemporaryDir directory;
  QFile file(directory.filePath("file"));
  QVERIFY(file.open(QIODevice::WriteOnly));
  file.close();
  GuiInstance instance(file.fileName() + "/child");
  QCOMPARE(instance.start(), GuiInstance::Result::Error);
  QVERIFY(!instance.errorString().isEmpty());
}

int main(int argc, char **argv) {
  QGuiApplication app(argc, argv);
  const QStringList arguments = app.arguments();
  if (arguments.size() == 4 && arguments[1] == "--instance-probe") {
    GuiInstance instance(arguments[2]);
    const auto result = instance.start();
    if (result == GuiInstance::Result::ActivatedExisting)
      return 10;
    if (result != GuiInstance::Result::Primary)
      return 1;
    QTextStream(stdout) << "primary" << Qt::endl;
    if (arguments[3] == "hold")
      return app.exec();
    return 0;
  }
  TestGuiInstance test;
  return QTest::qExec(&test, argc, argv);
}

#include "test_gui_instance.moc"
