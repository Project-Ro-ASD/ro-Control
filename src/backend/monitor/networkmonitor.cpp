#include "networkmonitor.h"

#include <QFile>
#include <QTextStream>

namespace {

constexpr int kDefaultUpdateIntervalMs = 2000;
constexpr int kMinUpdateIntervalMs = 1000;

QString qEnvOrDefault(const char *name, const QString &fallback) {
  const QString v = qEnvironmentVariable(name).trimmed();
  return v.isEmpty() ? fallback : v;
}

} // namespace

NetworkMonitor::NetworkMonitor(QObject *parent) : QObject(parent) {
  m_timer.setInterval(kDefaultUpdateIntervalMs);
  m_timer.setTimerType(Qt::CoarseTimer);
  connect(&m_timer, &QTimer::timeout, this, &NetworkMonitor::refresh);

  m_sampleTimer.start();
  refresh();
  start();
}

QString NetworkMonitor::netDevPath() {
  return qEnvOrDefault("RO_CONTROL_NET_DEV_PATH",
                       QStringLiteral("/proc/net/dev"));
}

bool NetworkMonitor::available() const {
  return m_available;
}

bool NetworkMonitor::running() const {
  return m_timer.isActive();
}

int NetworkMonitor::interfaceCount() const {
  return static_cast<int>(m_interfaces.size());
}

quint64 NetworkMonitor::totalRxBytesPerSec() const {
  return m_totalRxBytesPerSec;
}

quint64 NetworkMonitor::totalTxBytesPerSec() const {
  return m_totalTxBytesPerSec;
}

QVariantList NetworkMonitor::interfaces() const {
  QVariantList list;
  list.reserve(m_interfaces.size());
  for (const auto &iface : m_interfaces) {
    QVariantMap map;
    map.insert(QStringLiteral("name"), iface.name);
    map.insert(QStringLiteral("rxBytes"),
               QVariant::fromValue<quint64>(iface.rxBytes));
    map.insert(QStringLiteral("txBytes"),
               QVariant::fromValue<quint64>(iface.txBytes));
    map.insert(QStringLiteral("rxBytesPerSec"),
               QVariant::fromValue<quint64>(iface.rxBytesPerSec));
    map.insert(QStringLiteral("txBytesPerSec"),
               QVariant::fromValue<quint64>(iface.txBytesPerSec));
    map.insert(QStringLiteral("rxPackets"),
               QVariant::fromValue<quint64>(iface.rxPackets));
    map.insert(QStringLiteral("txPackets"),
               QVariant::fromValue<quint64>(iface.txPackets));
    map.insert(QStringLiteral("rxErrors"),
               QVariant::fromValue<quint64>(iface.rxErrors));
    map.insert(QStringLiteral("txErrors"),
               QVariant::fromValue<quint64>(iface.txErrors));
    map.insert(QStringLiteral("rxDropped"),
               QVariant::fromValue<quint64>(iface.rxDropped));
    map.insert(QStringLiteral("txDropped"),
               QVariant::fromValue<quint64>(iface.txDropped));
    list.append(map);
  }
  return list;
}

int NetworkMonitor::updateInterval() const {
  return m_timer.interval();
}

QString NetworkMonitor::statusMessage() const {
  return m_statusMessage;
}

void NetworkMonitor::start() {
  if (m_timer.isActive()) {
    return;
  }
  m_timer.start();
  emit runningChanged();
}

void NetworkMonitor::stop() {
  if (!m_timer.isActive()) {
    return;
  }
  m_timer.stop();
  emit runningChanged();
}

void NetworkMonitor::refresh() {
  parseNetDev();
  computeDeltas();

  setAvailable(!m_interfaces.isEmpty());
  emit networkStatsChanged();
}

void NetworkMonitor::setUpdateInterval(int intervalMs) {
  if (intervalMs < kMinUpdateIntervalMs || m_timer.interval() == intervalMs) {
    return;
  }
  m_timer.setInterval(intervalMs);
  emit updateIntervalChanged();
}

void NetworkMonitor::setAvailable(bool value) {
  if (m_available == value) {
    return;
  }
  m_available = value;
  emit availableChanged();
}

void NetworkMonitor::setStatusMessage(const QString &value) {
  if (m_statusMessage == value) {
    return;
  }
  m_statusMessage = value;
  emit statusMessageChanged();
}

void NetworkMonitor::parseNetDev() {
  QFile file(netDevPath());
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    setStatusMessage(QStringLiteral("Cannot read /proc/net/dev"));
    m_interfaces.clear();
    return;
  }

  QTextStream stream(&file);
  QList<NetworkInterfaceStats> fresh;

  // /proc/net/dev format:
  // Inter-|   Receive                                                |  Transmit
  // face |bytespackets errs drop fifo frame compressed multicast|bytespackets errs drop fifo colls carrier compressed
  //   eth0:  123456  1000    0    0    0    0    0    0  789012  2000    0    0    0    0    0    0

  // skip the two header lines
  stream.readLine();
  stream.readLine();

  while (!stream.atEnd()) {
    QString line = stream.readLine();
    if (line.isEmpty()) {
      continue;
    }

    const int colonPos = line.indexOf(':');
    if (colonPos < 0) {
      continue;
    }

    QString interfaceName = line.left(colonPos).trimmed();
    // Remove any trailing whitespace and common interface prefixes
    if (interfaceName.isEmpty()) {
      continue;
    }

    QStringList fields = line.mid(colonPos + 1).split(' ', Qt::SkipEmptyParts);

    if (fields.size() < 16) {
      continue;
    }

    bool ok = false;
    NetworkInterfaceStats iface;
    iface.name = interfaceName;

    iface.rxBytes = fields.at(0).toULongLong(&ok);
    if (!ok) continue;
    iface.rxPackets = fields.at(1).toULongLong(&ok);
    if (!ok) continue;
    iface.rxErrors = fields.at(2).toULongLong(&ok);
    if (!ok) continue;
    iface.rxDropped = fields.at(3).toULongLong(&ok);
    if (!ok) continue;

    iface.txBytes = fields.at(8).toULongLong(&ok);
    if (!ok) continue;
    iface.txPackets = fields.at(9).toULongLong(&ok);
    if (!ok) continue;
    iface.txErrors = fields.at(10).toULongLong(&ok);
    if (!ok) continue;
    iface.txDropped = fields.at(11).toULongLong(&ok);
    if (!ok) continue;

    fresh.append(iface);
  }

  file.close();

  m_interfaces = fresh;
}

void NetworkMonitor::computeDeltas() {
  const quint64 elapsedMs = static_cast<quint64>(m_sampleTimer.elapsed());
  if (elapsedMs == 0) {
    m_totalRxBytesPerSec = 0;
    m_totalTxBytesPerSec = 0;
    m_sampleTimer.restart();
    return;
  }

  quint64 totalRx = 0;
  quint64 totalTx = 0;

  for (auto &iface : m_interfaces) {
    const quint64 currRx = iface.rxBytes;
    const quint64 currTx = iface.txBytes;

    const quint64 prevRx = m_prevRxBytes.value(iface.name, 0);
    const quint64 prevTx = m_prevTxBytes.value(iface.name, 0);

    if (prevRx > 0 && currRx >= prevRx) {
      iface.rxBytesPerSec = ((currRx - prevRx) * 1000 + elapsedMs / 2) / elapsedMs;
    }
    if (prevTx > 0 && currTx >= prevTx) {
      iface.txBytesPerSec = ((currTx - prevTx) * 1000 + elapsedMs / 2) / elapsedMs;
    }

    totalRx += iface.rxBytesPerSec;
    totalTx += iface.txBytesPerSec;

    // Update previous counters
    m_prevRxBytes.insert(iface.name, currRx);
    m_prevTxBytes.insert(iface.name, currTx);
  }

  m_totalRxBytesPerSec = totalRx;
  m_totalTxBytesPerSec = totalTx;

  m_sampleTimer.restart();
}
