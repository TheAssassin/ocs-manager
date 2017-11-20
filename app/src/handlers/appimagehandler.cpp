#include "appimagehandler.h"

#ifdef QTLIB_UNIX
#include <thread>
#include <chrono>

#include "appimage/update.h"
#endif

#include "handlers/confighandler.h"

AppImageHandler::AppImageHandler(ConfigHandler *configHandler, QObject *parent)
    : QObject(parent), configHandler_(configHandler)
{}

QString AppImageHandler::describeAppImage(const QString &path) const
{
    QString updateInformation;

#ifdef QTLIB_UNIX
    appimage::update::Updater appImageUpdater(path.toStdString());
    std::string description;
    if (appImageUpdater.describeAppImage(description)) {
        updateInformation = QString::fromStdString(description);
    }
#endif

    return updateInformation;
}

bool AppImageHandler::isUpdateAvailable(const QString &path) const
{
#ifdef QTLIB_UNIX
    appimage::update::Updater appImageUpdater(path.toStdString());
    bool updateAvailable;
    if (appImageUpdater.checkForChanges(updateAvailable)) {
        return updateAvailable;
    }
#endif

    return false;
}

#ifdef QTLIB_UNIX
bool AppImageHandler::updateAppImage(const QString &path)
{
    appimage::update::Updater appImageUpdater(path.toStdString(), false);
    if (appImageUpdater.start()) {
        emit updateStarted(path);
        while (!appImageUpdater.isDone()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            double progress;
            if (appImageUpdater.progress(progress)) {
                emit updateProgress(path, progress * 100);
            }
        }
        emit updateFinished(path);
        if (!appImageUpdater.hasError()) {
            return true;
        }
    }

    return false;
}
#endif
