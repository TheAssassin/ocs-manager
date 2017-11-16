#include "appimagehandler.h"

#ifdef QTLIB_UNIX
#include "appimage/update.h"
#endif

#include "handlers/confighandler.h"

AppImageHandler::AppImageHandler(ConfigHandler *configHandler, QObject *parent)
    : QObject(parent), configHandler_(configHandler)
{}

bool AppImageHandler::isUpdateAvailable(const QString &path) const
{
#ifdef QTLIB_UNIX
    appimage::update::Updater appImageUpdater(path.toStdString());
    bool updateAvailable;
    return appImageUpdater.checkForChanges(updateAvailable);
#endif
    return false;
}

#ifdef QTLIB_UNIX
bool AppImageHandler::updateAppImage(const QString &path) const
{
    appimage::update::Updater appImageUpdater(path.toStdString(), false);
    return appImageUpdater.start();
    // TODO: make signals&slots bindings later
}
#endif
