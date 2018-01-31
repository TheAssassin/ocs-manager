#include "systemhandler.h"

#include <QUrl>
#include <QDesktopServices>

#ifdef QTIL_OS_UNIX
#include <QFileInfo>
#include <QProcess>
#endif

#ifdef Q_OS_ANDROID
#include "qtil_package.h"
#endif

SystemHandler::SystemHandler(QObject *parent)
    : QObject(parent)
{}

bool SystemHandler::isUnix() const
{
#ifdef QTIL_OS_UNIX
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

#ifdef QTIL_OS_UNIX
    if ((path.endsWith(".appimage", Qt::CaseInsensitive) || path.endsWith(".exe", Qt::CaseInsensitive))
            && QFileInfo(path).isExecutable()) {
        return QProcess::startDetached(path);
    }
#endif

#ifdef Q_OS_ANDROID
    if (path.endsWith(".apk", Qt::CaseInsensitive)) {
        return qtil::Package(path).installAsApk();
    }
#endif

    return QDesktopServices::openUrl(QUrl(url));
}
