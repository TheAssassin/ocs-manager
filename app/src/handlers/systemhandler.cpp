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
    QString desktop = "";
    QString currentDesktop = "";

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
                        << "plasma_desktopthemes";
    }
    else if (desktop == "gnome") {
        applicableTypes << "wallpapers"
                        << "icons"
                        << "cursors"
                        << "gtk3_themes"
                        << "gnome_shell_themes";
    }
    else if (desktop == "xfce") {
        applicableTypes << "wallpapers";
    }

    return applicableTypes.contains(installType);
}

#ifdef QTLIB_UNIX
bool SystemHandler::applyFile(const QString &path, const QString &installType) const
{
    if (QFileInfo::exists(path) && isApplicableType(installType)) {
        auto desktop = desktopEnvironment();

        if (desktop == "kde") {
            if (installType == "wallpapers") {
                return applyKdeWallpaper(path);
            }
            else if (installType == "icons") {
                return applyKdeIcon(path);
            }
            else if (installType == "cursors") {
                return applyKdeCursor(path);
            }
            else if (installType == "plasma_desktopthemes") {
                return applyKdePlasmaDesktoptheme(path);
            }
        }
        else if (desktop == "gnome") {
            if (installType == "wallpapers") {
                return applyGnomeWallpaper(path);
            }
            else if (installType == "icons") {
                return applyGnomeIcon(path);
            }
            else if (installType == "cursors") {
                return applyGnomeCursor(path);
            }
            else if (installType == "gtk3_themes") {
                return applyGnomeGtk3Theme(path);
            }
            else if (installType == "gnome_shell_themes") {
                return applyGnomeGnomeShellTheme(path);
            }
        }
        else if (desktop == "xfce") {
            if (installType == "wallpapers") {
                return applyXfceWallpaper(path);
            }
        }
    }

    return false;
}
#endif

#ifdef QTLIB_UNIX
bool SystemHandler::applyKdeWallpaper(const QString &path) const
{
    auto message = QDBusMessage::createMethodCall("org.kde.plasmashell", "/PlasmaShell", "org.kde.PlasmaShell", "evaluateScript");

    QString script;
    QTextStream out(&script);
    out << "for (var key in desktops()) {"
        << "var d = desktops()[key];"
        << "d.wallpaperPlugin = 'org.kde.image';"
        << "d.currentConfigGroup = ['Wallpaper', 'org.kde.image', 'General'];"
        << "d.writeConfig('Image', 'file://" + path + "');"
        << "}";

    QVariantList arguments;
    arguments << QVariant(script);
    message.setArguments(arguments);

    auto reply = QDBusConnection::sessionBus().call(message);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << reply.errorMessage();
        return false;
    }

    return true;
}

bool SystemHandler::applyKdeIcon(const QString &path) const
{
    auto themeName = QFileInfo(path).fileName();
    auto message = QDBusMessage::createMethodCall("org.kde.plasmashell", "/PlasmaShell", "org.kde.PlasmaShell", "evaluateScript");

    QString script;
    QTextStream out(&script);
    out << "var c = ConfigFile('kdeglobals');"
        << "c.group = 'Icons';"
        << "c.writeEntry('Theme', '" + themeName + "');";

    QVariantList arguments;
    arguments << QVariant(script);
    message.setArguments(arguments);

    auto reply = QDBusConnection::sessionBus().call(message);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << reply.errorMessage();
        return false;
    }

    return true;
}

bool SystemHandler::applyKdeCursor(const QString &path) const
{
    auto themeName = QFileInfo(path).fileName();
    auto message = QDBusMessage::createMethodCall("org.kde.plasmashell", "/PlasmaShell", "org.kde.PlasmaShell", "evaluateScript");

    QString script;
    QTextStream out(&script);
    out << "var c = ConfigFile('kcminputrc');"
        << "c.group = 'Mouse';"
        << "c.writeEntry('cursorTheme', '" + themeName + "');";

    QVariantList arguments;
    arguments << QVariant(script);
    message.setArguments(arguments);

    auto reply = QDBusConnection::sessionBus().call(message);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << reply.errorMessage();
        return false;
    }

    return true;
}

bool SystemHandler::applyKdePlasmaDesktoptheme(const QString &path) const
{
    auto themeName = QFileInfo(path).fileName();
    auto message = QDBusMessage::createMethodCall("org.kde.plasmashell", "/PlasmaShell", "org.kde.PlasmaShell", "evaluateScript");

    QString script;
    QTextStream out(&script);
    out << "var c = ConfigFile('plasmarc');"
        << "c.group = 'Theme';"
        << "c.writeEntry('name', '" + themeName + "');";

    QVariantList arguments;
    arguments << QVariant(script);
    message.setArguments(arguments);

    auto reply = QDBusConnection::sessionBus().call(message);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << reply.errorMessage();
        return false;
    }

    return true;
}

bool SystemHandler::applyGnomeWallpaper(const QString &path) const
{
    QStringList arguments{"set", "org.gnome.desktop.background", "picture-uri", "file://" + path};
    return QProcess::startDetached("gsettings", arguments);
}

bool SystemHandler::applyGnomeIcon(const QString &path) const
{
    auto themeName = QFileInfo(path).fileName();
    QStringList arguments{"set", "org.gnome.desktop.interface", "icon-theme", themeName};
    return QProcess::startDetached("gsettings", arguments);
}

bool SystemHandler::applyGnomeCursor(const QString &path) const
{
    auto themeName = QFileInfo(path).fileName();
    QStringList arguments{"set", "org.gnome.desktop.interface", "cursor-theme", themeName};
    return QProcess::startDetached("gsettings", arguments);
}

bool SystemHandler::applyGnomeGtk3Theme(const QString &path) const
{
    auto themeName = QFileInfo(path).fileName();
    QStringList arguments{"set", "org.gnome.desktop.interface", "gtk-theme", themeName};
    return QProcess::startDetached("gsettings", arguments);
}

bool SystemHandler::applyGnomeGnomeShellTheme(const QString &path) const
{
    auto themeName = QFileInfo(path).fileName();
    QStringList arguments{"set", "org.gnome.shell.extensions.user-theme", "name", themeName};
    return QProcess::startDetached("gsettings", arguments);
}

bool SystemHandler::applyXfceWallpaper(const QString &path) const
{
    auto message = QDBusMessage::createMethodCall("org.xfce.Xfconf", "/org/xfce/Xfconf", "org.xfce.Xfconf", "SetProperty");

    QString channelValue = "xfce4-desktop";
    //QString propertyValue = "/backdrop/screen0/monitor0/image-path";
    QString propertyValue = "/backdrop/screen0/monitor0/workspace0/last-image";
    QDBusVariant valueValue(path);

    QVariantList arguments;
    arguments << QVariant(channelValue) << QVariant(propertyValue) << QVariant::fromValue(valueValue);
    message.setArguments(arguments);

    auto reply = QDBusConnection::sessionBus().call(message);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << reply.errorMessage();
        return false;
    }

    return true;
}
#endif
