#include "capabilityprobe.h"

#include "commandrunner.h"

namespace CapabilityProbe {

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

} // namespace CapabilityProbe
