#include "updatehandler.h"

#ifdef QTLIB_UNIX
#include <thread>
#include <chrono>

#include "appimage/update.h"
#endif

#include "handlers/confighandler.h"

UpdateHandler::UpdateHandler(ConfigHandler *configHandler, QObject *parent)
    : QObject(parent), configHandler_(configHandler)
{}

QString UpdateHandler::checkAll() const
{
}

QString UpdateHandler::update(const QString &path) const
{
}

#ifdef QTLIB_UNIX
QString UpdateHandler::describeAppImage(const QString &path) const
{
    appimage::update::Updater appImageUpdater(path.toStdString());
    QString updateInformation;
    std::string description;
    if (appImageUpdater.describeAppImage(description)) {
        updateInformation = QString::fromStdString(description);
    }
    return updateInformation;
}

bool UpdateHandler::checkAppImage(const QString &path) const
{
    appimage::update::Updater appImageUpdater(path.toStdString());
    bool updateAvailable;
    if (appImageUpdater.checkForChanges(updateAvailable)) {
        return updateAvailable;
    }
    return false;
}

bool UpdateHandler::updateAppImage(const QString &path)
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
        auto newPath = path; // dummy
        emit updateFinished(path, newPath);
        if (!appImageUpdater.hasError()) {
            return true;
        }
    }
    return false;
}
#endif
