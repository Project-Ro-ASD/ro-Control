#pragma once

#include <QString>
#include <QStringList>

namespace CapabilityProbe {

struct ToolStatus {
  QString program;
  QString resolvedPath;
  bool available = false;
};

ToolStatus probeTool(const QString &program);
bool isToolAvailable(const QString &program);
QStringList missingTools(const QStringList &programs);
QString missingToolsMessage(const QStringList &programs);

} // namespace CapabilityProbe
