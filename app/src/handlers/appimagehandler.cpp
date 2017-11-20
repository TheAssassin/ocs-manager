#include "appimagehandler.h"

#ifdef QTLIB_UNIX
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
bool AppImageHandler::updateAppImage(const QString &path) const
{
    appimage::update::Updater appImageUpdater(path.toStdString(), false);
    /*if (appImageUpdater.start()) {
        // TODO: make signals&slots bindings later
    }*/

    return false;
}
#endif
