#include "systemhandler.h"

#include <QUrl>
#include <QDesktopServices>

#ifdef QTLIB_UNIX
#include <QFileInfo>
#include <QProcess>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusVariant>
#include <QDebug>
#endif

#ifdef Q_OS_ANDROID
#include "qtlib_package.h"
#endif

SystemHandler::SystemHandler(QObject *parent)
    : QObject(parent)
{}

bool SystemHandler::isUnix() const
{
#ifdef QTLIB_UNIX
    return true;
#endif
    return false;
}

bool SystemHandler::isMobileDevice() const
{
#if defined(APP_MOBILE)
    return true;
#elif defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE)
    return true;
#elif defined(Q_OS_LINUX) && defined(Q_PROCESSOR_ARM) // Ubuntu Phone, Plasma Phone
    return true;
#endif
    return false;
}

bool SystemHandler::openUrl(const QString &url) const
{
    auto path = url;
    path.replace("file://localhost", "", Qt::CaseInsensitive);
    path.replace("file://", "", Qt::CaseInsensitive);

#ifdef QTLIB_UNIX
    if ((path.endsWith(".appimage", Qt::CaseInsensitive) || path.endsWith(".exe", Qt::CaseInsensitive))
            && QFileInfo(path).isExecutable()) {
        return QProcess::startDetached(path);
    }
#endif

#ifdef Q_OS_ANDROID
    if (path.endsWith(".apk", Qt::CaseInsensitive)) {
        return qtlib::Package(path).installAsApk();
    }
#endif

    return QDesktopServices::openUrl(QUrl(url));
}

QString SystemHandler::desktopEnvironment() const
{
    QString desktop;
    QString currentDesktop;

    if (!qgetenv("XDG_CURRENT_DESKTOP").isEmpty()) {
        currentDesktop = QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP").constData()).toLower();
    }
    else if (!qgetenv("XDG_SESSION_DESKTOP").isEmpty()) {
        currentDesktop = QString::fromLocal8Bit(qgetenv("XDG_SESSION_DESKTOP").constData()).toLower();
    }
    else if (!qgetenv("DESKTOP_SESSION").isEmpty()) {
        currentDesktop = QString::fromLocal8Bit(qgetenv("DESKTOP_SESSION").constData()).toLower();
    }

    if (currentDesktop.contains("kde") || currentDesktop.contains("plasma")) {
        desktop = "kde";
    }
    else if (currentDesktop.contains("gnome") || currentDesktop.contains("unity")) {
        desktop = "gnome";
    }
    else if (currentDesktop.contains("xfce")) {
        desktop = "xfce";
    }
    return desktop;
}

bool SystemHandler::isApplicableType(const QString &installType) const
{
    auto desktop = desktopEnvironment();

    QStringList applicableTypes;

    if (desktop == "kde") {
        applicableTypes << "wallpapers"
                        << "icons"
                        << "cursors"
                        << "plasma5_desktopthemes"
                        << "aurorae_themes";
    }
    else if (desktop == "gnome") {
        applicableTypes << "wallpapers"
                        << "icons"
                        << "cursors"
                        << "gtk3_themes"
                        << "gnome_shell_themes";
    }
    else if (desktop == "xfce") {
        applicableTypes << "wallpapers"
                        << "icons";
    }

    return applicableTypes.contains(installType);
}

#ifdef QTLIB_UNIX
bool SystemHandler::applyFile(const QString &path, const QString &installType) const
{
    if (QFileInfo::exists(path) && isApplicableType(installType)) {
        auto desktop = desktopEnvironment();
        auto themeName = QFileInfo(path).fileName();

        if (desktop == "kde") {
            if (installType == "wallpapers") {
                return applyKdeWallpaper(path);
            }
            else if (installType == "icons") {
                return applyKdeIcon(themeName);
            }
            else if (installType == "cursors") {
                return applyKdeCursor(themeName);
            }
            else if (installType == "plasma5_desktopthemes") {
                return applyKdePlasmaDesktoptheme(themeName);
            }
            else if (installType == "aurorae_themes") {
                return applyKdeAuroraeTheme(themeName);
            }
        }
        else if (desktop == "gnome") {
            if (installType == "wallpapers") {
                return applyGnomeWallpaper(path);
            }
            else if (installType == "icons") {
                return applyGnomeIcon(themeName);
            }
            else if (installType == "cursors") {
                return applyGnomeCursor(themeName);
            }
            else if (installType == "gtk3_themes") {
                return applyGnomeGtk3Theme(themeName);
            }
            else if (installType == "gnome_shell_themes") {
                return applyGnomeGnomeShellTheme(themeName);
            }
        }
        else if (desktop == "xfce") {
            if (installType == "wallpapers") {
                return applyXfceWallpaper(path);
            }
            else if (installType == "icons") {
                return applyXfceIcon(themeName);
            }
        }
    }

    return false;
}
#endif

#ifdef QTLIB_UNIX
bool SystemHandler::setConfigWithPlasmaShell(const QString &script) const
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

bool SystemHandler::applyKdeWallpaper(const QString &path) const
{
    QString script;
    QTextStream out(&script);
    out << "for (var key in desktops()) {"
        << "var d = desktops()[key];"
        << "d.wallpaperPlugin = 'org.kde.image';"
        << "d.currentConfigGroup = ['Wallpaper', 'org.kde.image', 'General'];"
        << "d.writeConfig('Image', 'file://" + path + "');"
        << "}";

    return setConfigWithPlasmaShell(script);
}

bool SystemHandler::applyKdeIcon(const QString &themeName) const
{
    QString script;
    QTextStream out(&script);
    out << "var c = ConfigFile('kdeglobals');"
        << "c.group = 'Icons';"
        << "c.writeEntry('Theme', '" + themeName + "');";

    if (setConfigWithPlasmaShell(script)) {
        QProcess::startDetached("kquitapp5 plasmashell && kstart5 plasmashell");
        return true;
    }
    return false;
}

bool SystemHandler::applyKdeCursor(const QString &themeName) const
{
    QString script;
    QTextStream out(&script);
    out << "var c = ConfigFile('kcminputrc');"
        << "c.group = 'Mouse';"
        << "c.writeEntry('cursorTheme', '" + themeName + "');";

    return setConfigWithPlasmaShell(script);
}

bool SystemHandler::applyKdePlasmaDesktoptheme(const QString &themeName) const
{
    QString script;
    QTextStream out(&script);
    out << "var c = ConfigFile('plasmarc');"
        << "c.group = 'Theme';"
        << "c.writeEntry('name', '" + themeName + "');";

    if (setConfigWithPlasmaShell(script)) {
        QProcess::startDetached("kquitapp5 plasmashell && kstart5 plasmashell");
        return true;
    }
    return false;
}

bool SystemHandler::applyKdeAuroraeTheme(const QString &themeName) const
{
    QString script;
    QTextStream out(&script);
    out << "var c = ConfigFile('kwinrc');"
        << "c.group = 'org.kde.kdecoration2';"
        << "c.writeEntry('library', 'org.kde.kwin.aurorae');"
        << "c.writeEntry('theme', '__aurorae__svg__" + themeName + "');";

    if (setConfigWithPlasmaShell(script)) {
        auto message = QDBusMessage::createMethodCall("org.kde.KWin", "/KWin", "org.kde.KWin", "reconfigure");
        QDBusConnection::sessionBus().call(message);
        return true;
    }
    return false;
}

bool SystemHandler::setConfigWithGsettings(const QString &schema, const QString &key, const QString &value) const
{
    return QProcess::startDetached("gsettings", QStringList() << "set" << schema << key << value);
}

bool SystemHandler::applyGnomeWallpaper(const QString &path) const
{
    return setConfigWithGsettings("org.gnome.desktop.background", "picture-uri", "file://" + path);
}

bool SystemHandler::applyGnomeIcon(const QString &themeName) const
{
    return setConfigWithGsettings("org.gnome.desktop.interface", "icon-theme", themeName);
}

bool SystemHandler::applyGnomeCursor(const QString &themeName) const
{
    return setConfigWithGsettings("org.gnome.desktop.interface", "cursor-theme", themeName);
}

bool SystemHandler::applyGnomeGtk3Theme(const QString &themeName) const
{
    return setConfigWithGsettings("org.gnome.desktop.interface", "gtk-theme", themeName);
}

bool SystemHandler::applyGnomeGnomeShellTheme(const QString &themeName) const
{
    return setConfigWithGsettings("org.gnome.shell.extensions.user-theme", "name", themeName);
}

bool SystemHandler::setConfigWithXfconf(const QString &channel, const QString &property, const QString &value) const
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

bool SystemHandler::applyXfceWallpaper(const QString &path) const
{
    return setConfigWithXfconf("xfce4-desktop", "/backdrop/screen0/monitor0/workspace0/last-image", path);
}

bool SystemHandler::applyXfceIcon(const QString &themeName) const
{
    return setConfigWithXfconf("xsettings", "/Net/IconThemeName", themeName);
}
#endif
