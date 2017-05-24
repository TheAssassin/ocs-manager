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

bool SystemHandler::isUnix()
{
#ifdef QTLIB_UNIX
    return true;
#endif
    return false;
}

bool SystemHandler::isMobileDevice()
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

bool SystemHandler::openUrl(const QString &url)
{
    QString path = url;
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

QString SystemHandler::desktopEnvironment()
{
    QString desktop = "unknown";
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

bool SystemHandler::isApplicableType(const QString &installType)
{
    QString desktop = desktopEnvironment();

    if (installType == "wallpapers"
            && (desktop == "kde" || desktop == "gnome" || desktop == "xfce")) {
        return true;
    }
    /*else if (installType == "icons"
             && (desktop == "kde" || desktop == "gnome" || desktop == "xfce")) {
        return true;
    }
    else if (installType == "cursors"
             && (desktop == "kde" || desktop == "gnome" || desktop == "xfce")) {
        return true;
    }
    else if ((installType == "aurorae_themes" && desktop == "kde")
             || (installType == "metacity_themes" && desktop == "gnome")
             || (installType == "xfwm4_themes" && desktop == "xfce")) {
        return true;
    }*/
    return false;
}

#ifdef QTLIB_UNIX
bool SystemHandler::applyFile(const QString &path, const QString &installType)
{
    if (QFileInfo::exists(path) && isApplicableType(installType)) {
        if (installType == "wallpapers") {
            return applyWallpaper(path);
        }
        /*else if (installType == "icons") {
            return applyIcon(path);
        }
        else if (installType == "cursors") {
            return applyCursor(path);
        }
        else if (installType == "aurorae_themes"
                 || installType == "metacity_themes"
                 || installType == "xfwm4_themes") {
            return applyWindowTheme(path);
        }*/
    }
    return false;
}
#endif

#ifdef QTLIB_UNIX
bool SystemHandler::applyWallpaper(const QString &path)
{
    QString desktop = desktopEnvironment();

    if (desktop == "kde") {
        // plasma5.6+
        QDBusMessage message = QDBusMessage::createMethodCall("org.kde.plasmashell", "/PlasmaShell", "org.kde.PlasmaShell", "evaluateScript");
        QVariantList arguments;

        QString script;
        QTextStream out(&script);
        out << "for (var key in desktops()) {"
            << "var d = desktops()[key];"
            << "d.wallpaperPlugin = 'org.kde.image';"
            << "d.currentConfigGroup = ['Wallpaper', 'org.kde.image', 'General'];"
            << "d.writeConfig('Image', 'file://" + path + "');"
            << "}";

        arguments << QVariant(script);
        message.setArguments(arguments);

        QDBusMessage reply = QDBusConnection::sessionBus().call(message);

        if (reply.type() == QDBusMessage::ErrorMessage) {
            qWarning() << reply.errorMessage();
            return false;
        }
        return true;
    }
    else if (desktop == "gnome") {
        QStringList arguments;
        // gnome3
        arguments << "set" << "org.gnome.desktop.background" << "picture-uri" << "file://" + path;
        return QProcess::startDetached("gsettings", arguments);
        // gnome2
        //arguments << "--type=string" << "--set" << "/desktop/gnome/background/picture_filename" << path;
        //return QProcess::startDetached("gconftool-2", arguments);
    }
    else if (desktop == "xfce") {
        QDBusMessage message = QDBusMessage::createMethodCall("org.xfce.Xfconf", "/org/xfce/Xfconf", "org.xfce.Xfconf", "SetProperty");
        QVariantList arguments;

        QString channelValue = "xfce4-desktop";
        //QString propertyValue = "/backdrop/screen0/monitor0/image-path";
        QString propertyValue = "/backdrop/screen0/monitor0/workspace0/last-image";
        QDBusVariant valueValue(path);

        arguments << QVariant(channelValue) << QVariant(propertyValue) << QVariant::fromValue(valueValue);
        message.setArguments(arguments);

        QDBusMessage reply = QDBusConnection::sessionBus().call(message);

        if (reply.type() == QDBusMessage::ErrorMessage) {
            qWarning() << reply.errorMessage();
            return false;
        }
        return true;
    }
    return false;
}

bool SystemHandler::applyIcon(const QString &path)
{
    qDebug() << path;

    QString desktop = desktopEnvironment();

    if (desktop == "kde") {
    }
    else if (desktop == "gnome") {
    }
    else if (desktop == "xfce") {
    }
    return false;
}

bool SystemHandler::applyCursor(const QString &path)
{
    qDebug() << path;

    QString desktop = desktopEnvironment();

    if (desktop == "kde") {
    }
    else if (desktop == "gnome") {
    }
    else if (desktop == "xfce") {
    }
    return false;
}

bool SystemHandler::applyWindowTheme(const QString &path)
{
    qDebug() << path;

    QString desktop = desktopEnvironment();

    if (desktop == "kde") {
    }
    else if (desktop == "gnome") {
    }
    else if (desktop == "xfce") {
    }
    return false;
}
#endif
