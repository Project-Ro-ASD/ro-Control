#include "polkit.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusArgument>
#include <QVariantMap>

#include <unistd.h>  // getpid()

PolkitHelper::PolkitHelper(QObject *parent)
    : QObject(parent)
{}

bool PolkitHelper::checkAuthorization()
{
    QDBusInterface polkit(
        QStringLiteral("org.freedesktop.PolicyKit1"),
        QStringLiteral("/org/freedesktop/PolicyKit1/Authority"),
        QStringLiteral("org.freedesktop.PolicyKit1.Authority"),
        QDBusConnection::systemBus()
    );

    if (!polkit.isValid())
        return false;

    // Subject: mevcut prosesin PID'si
    QVariantMap subject;
    subject[QStringLiteral("pid")]            = static_cast<quint32>(getpid());
    subject[QStringLiteral("start-time")]     = static_cast<quint64>(0);

    QDBusArgument subjectArg;
    subjectArg.beginStructure();
    subjectArg << QStringLiteral("unix-process") << subject;
    subjectArg.endStructure();

    // CheckAuthorization çağrısı — interactivity 0 = dialog gösterme
    QDBusReply<QVariantMap> reply = polkit.call(
        QStringLiteral("CheckAuthorization"),
        QVariant::fromValue(subjectArg),
        QString::fromLatin1(ActionId),
        QVariantMap(),                         // details
        static_cast<quint32>(0),               // AllowUserInteraction = 0 (check only)
        QString()                              // cancellation id
    );

    if (!reply.isValid())
        return false;

    const bool authorized = reply.value().value(QStringLiteral("is-authorized")).toBool();

    if (authorized != m_authorized) {
        m_authorized = authorized;
        emit authorizedChanged();
    }

    return m_authorized;
}

bool PolkitHelper::requestAuthorization()
{
    QDBusInterface polkit(
        QStringLiteral("org.freedesktop.PolicyKit1"),
        QStringLiteral("/org/freedesktop/PolicyKit1/Authority"),
        QStringLiteral("org.freedesktop.PolicyKit1.Authority"),
        QDBusConnection::systemBus()
    );

    if (!polkit.isValid())
        return false;

    QVariantMap subject;
    subject[QStringLiteral("pid")]            = static_cast<quint32>(getpid());
    subject[QStringLiteral("start-time")]     = static_cast<quint64>(0);

    QDBusArgument subjectArg;
    subjectArg.beginStructure();
    subjectArg << QStringLiteral("unix-process") << subject;
    subjectArg.endStructure();

    // AllowUserInteraction = 1 → kullanıcıya parola dialogu gösterilir
    QDBusReply<QVariantMap> reply = polkit.call(
        QStringLiteral("CheckAuthorization"),
        QVariant::fromValue(subjectArg),
        QString::fromLatin1(ActionId),
        QVariantMap(),
        static_cast<quint32>(1),               // AllowUserInteraction
        QString()
    );

    if (!reply.isValid())
        return false;

    const bool authorized = reply.value().value(QStringLiteral("is-authorized")).toBool();

    if (authorized != m_authorized) {
        m_authorized = authorized;
        emit authorizedChanged();
    }

    return m_authorized;
}
