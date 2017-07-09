#include "kdetheme.h"

#include <QVariantList>
#include <QTextStream>
#include <QDir>
#include <QProcess>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDebug>

KdeTheme::KdeTheme(const QString &path, QObject *parent)
    : QObject(parent), path_(path)
{
    themeName_ = QDir(path_).dirName();
}

bool KdeTheme::applyAsWallpaper() const
{
    QString script;
    QTextStream out(&script);
    out << "for (var key in desktops()) {"
        << "var d = desktops()[key];"
        << "d.wallpaperPlugin = 'org.kde.image';"
        << "d.currentConfigGroup = ['Wallpaper', 'org.kde.image', 'General'];"
        << "d.writeConfig('Image', 'file://" + path_ + "');"
        << "}";

    return evaluateScript(script);
}

bool KdeTheme::applyAsIcon() const
{
    QString script;
    QTextStream out(&script);
    out << "var c = ConfigFile('kdeglobals');"
        << "c.group = 'Icons';"
        << "c.writeEntry('Theme', '" + themeName_ + "');";

    if (evaluateScript(script)) {
        auto iconChanged = QDBusMessage::createSignal("/KIconLoader", "org.kde.KIconLoader", "iconChanged");
        iconChanged.setArguments(QVariantList() << QVariant(qint32(0)));
        QDBusConnection::sessionBus().send(iconChanged);

        auto notifyChange = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
        notifyChange.setArguments(QVariantList() << QVariant(qint32(4)) << QVariant(qint32(0)));
        QDBusConnection::sessionBus().send(notifyChange);

        return true;
    }
    return false;
}

bool KdeTheme::applyAsCursor() const
{
    QString script;
    QTextStream out(&script);
    out << "var c = ConfigFile('kcminputrc');"
        << "c.group = 'Mouse';"
        << "c.writeEntry('cursorTheme', '" + themeName_ + "');";

    if (evaluateScript(script)) {
        auto setLaunchEnv = QDBusMessage::createMethodCall("org.kde.klauncher5", "/KLauncher", "org.kde.KLauncher", "setLaunchEnv");
        setLaunchEnv.setArguments(QVariantList() << QVariant(QString("XCURSOR_THEME")) << QVariant(themeName_));
        QDBusConnection::sessionBus().call(setLaunchEnv);

        auto notifyChange = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
        notifyChange.setArguments(QVariantList() << QVariant(qint32(5)) << QVariant(qint32(0)));
        QDBusConnection::sessionBus().send(notifyChange);

        return true;
    }
    return false;
}

bool KdeTheme::applyAsPlasmaDesktoptheme() const
{
    QString script;
    QTextStream out(&script);
    out << "var c = ConfigFile('plasmarc');"
        << "c.group = 'Theme';"
        << "c.writeEntry('name', '" + themeName_ + "');";

    if (evaluateScript(script)) {
        QProcess::startDetached("kquitapp5 plasmashell");
        QProcess::startDetached("kstart5 plasmashell");
        return true;
    }
    return false;
}

bool KdeTheme::applyAsAuroraeTheme() const
{
    QString script;
    QTextStream out(&script);
    out << "var c = ConfigFile('kwinrc');"
        << "c.group = 'org.kde.kdecoration2';"
        << "c.writeEntry('library', 'org.kde.kwin.aurorae');"
        << "c.writeEntry('theme', '__aurorae__svg__" + themeName_ + "');";

    if (evaluateScript(script)) {
        auto reloadConfig = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
        QDBusConnection::sessionBus().send(reloadConfig);
        return true;
    }
    return false;
}

bool KdeTheme::evaluateScript(const QString &script) const
{
    auto message = QDBusMessage::createMethodCall("org.kde.plasmashell", "/PlasmaShell", "org.kde.PlasmaShell", "evaluateScript");
    message.setArguments(QVariantList() << QVariant(script));
    auto reply = QDBusConnection::sessionBus().call(message);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << reply.errorMessage();
        return false;
    }
    return true;
}
