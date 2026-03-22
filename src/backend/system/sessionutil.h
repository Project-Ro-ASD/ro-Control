#pragma once

#include <QString>

namespace SessionUtil {

// XDG_SESSION_TYPE ortam degiskeni veya loginctl
// uzerinden geçerli oturum turunu tespit eder.
// "wayland", "x11" veya "unknown" doner.
QString detectSessionType();

} // namespace SessionUtil
