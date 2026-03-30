#include "capabilityprobe.h"

#include "commandrunner.h"

#include <QSysInfo>

namespace CapabilityProbe {

QString normalizedCpuArchitecture() {
  const QString architecture = QSysInfo::currentCpuArchitecture().trimmed();
  if (architecture.isEmpty()) {
    return QStringLiteral("unknown");
  }

  const QString lowered = architecture.toLower();
  if (lowered == QStringLiteral("i386") || lowered == QStringLiteral("i486") ||
      lowered == QStringLiteral("i586") || lowered == QStringLiteral("i686") ||
      lowered == QStringLiteral("x86")) {
    return QStringLiteral("i686");
  }

  if (lowered == QStringLiteral("x86_64") ||
      lowered == QStringLiteral("amd64")) {
    return QStringLiteral("x86_64");
  }

  if (lowered == QStringLiteral("arm64")) {
    return QStringLiteral("aarch64");
  }

  return lowered;
}

ToolStatus probeTool(const QString &program) {
  ToolStatus status;
  status.program = program;
  status.resolvedPath = CommandRunner::resolveProgramPath(program);
  status.available = !status.resolvedPath.isEmpty();
  return status;
}

bool isToolAvailable(const QString &program) {
  return probeTool(program).available;
}

QStringList missingTools(const QStringList &programs) {
  QStringList missing;
  for (const QString &program : programs) {
    if (!isToolAvailable(program)) {
      missing << program;
    }
  }

  missing.removeDuplicates();
  return missing;
}

QString missingToolsMessage(const QStringList &programs) {
  const QStringList missing = missingTools(programs);
  if (missing.isEmpty()) {
    return {};
  }

  return QStringLiteral("Missing required tools: %1")
      .arg(missing.join(QStringLiteral(", ")));
}

bool supportsFedoraNvidiaDriverFlow() {
  const QString architecture = normalizedCpuArchitecture();
  return architecture == QStringLiteral("x86_64") ||
         architecture == QStringLiteral("aarch64");
}

QString fedoraNvidiaDriverFlowSupportMessage() {
  if (supportsFedoraNvidiaDriverFlow()) {
    return {};
  }

  return QStringLiteral(
      "Fedora NVIDIA driver management is currently supported only on x86_64 "
      "and aarch64 builds. The current build architecture is %1.")
      .arg(normalizedCpuArchitecture());
}

} // namespace CapabilityProbe
