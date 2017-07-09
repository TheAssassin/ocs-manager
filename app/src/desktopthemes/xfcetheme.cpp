#include "xfcetheme.h"

#include <QVariantList>
#include <QDir>
#include <QProcess>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusVariant>
#include <QDebug>

XfceTheme::XfceTheme(const QString &path, QObject *parent)
    : QObject(parent), path_(path)
{
    themeName_ = QDir(path_).dirName();
}

bool XfceTheme::applyAsWallpaper() const
{
    return setConfig("xfce4-desktop", "/backdrop/screen0/monitor0/workspace0/last-image", path_);
}

bool XfceTheme::applyAsIcon() const
{
    return setConfig("xsettings", "/Net/IconThemeName", themeName_);
}

bool XfceTheme::applyAsCursor() const
{
    return setConfig("xsettings", "/Gtk/CursorThemeName", themeName_);
}

bool XfceTheme::applyAsGtk2Theme() const
{
    return setConfig("xsettings", "/Net/ThemeName", themeName_);
}

bool XfceTheme::applyAsXfwm4Theme() const
{
    return setConfig("xfwm4", "/general/theme", themeName_);
}

bool XfceTheme::setConfig(const QString &channel, const QString &property, const QString &value) const
{
    auto message = QDBusMessage::createMethodCall("org.xfce.Xfconf", "/org/xfce/Xfconf", "org.xfce.Xfconf", "SetProperty");
    message.setArguments(QVariantList() << QVariant(channel) << QVariant(property) << QVariant::fromValue(QDBusVariant(value)));
    auto reply = QDBusConnection::sessionBus().call(message);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << reply.errorMessage();
        return false;
    }
    return true;
}
