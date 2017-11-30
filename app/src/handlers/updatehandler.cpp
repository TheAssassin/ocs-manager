#include "updatehandler.h"

#include <QStringList>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QThread>
#include <QDebug>

#ifdef QTLIB_UNIX
#include "appimage/update.h"
#endif

#include "qtlib_file.h"

#include "handlers/confighandler.h"

UpdateHandler::UpdateHandler(ConfigHandler *configHandler, QObject *parent)
    : QObject(parent), configHandler_(configHandler)
{}

void UpdateHandler::checkAll()
{
    emit checkAllStarted();

    QJsonObject updateAvailableItems;
    configHandler_->setUsrConfigUpdateAvailableItems(updateAvailableItems); // Resets data

    auto installedItems = configHandler_->getUsrConfigInstalledItems();

    if (installedItems.isEmpty()) {
        emit checkAllFinished();
        return;
    }

    for (const auto &itemKey : installedItems.keys()) {
        auto installedItem = installedItems[itemKey].toObject();
        auto filename = installedItem["filename"].toString();
        auto installType = installedItem["install_type"].toString();

        qtlib::File file;
#ifdef QTLIB_UNIX
        file.setPath(configHandler_->getAppConfigInstallTypes()[installType].toObject()["destination"].toString() + "/" + filename);
#else
        file.setPath(configHandler_->getAppConfigInstallTypes()[installType].toObject()["generic_destination"].toString() + "/" + filename);
#endif

        QJsonObject updateAvailableItem;

        if (installType == "bin") {
#ifdef QTLIB_UNIX
            if (file.path().endsWith(".appimage", Qt::CaseInsensitive)) {
                if (checkAppImage(file.path())) {
                    updateAvailableItem["installed_item"] = itemKey;
                    updateAvailableItem["update_method"] = QString("appimageupdate");
                    configHandler_->setUsrConfigUpdateAvailableItemsItem(itemKey, updateAvailableItem);
                }
                //else if (checkAppImageWithOcsApi(itemKey)) {}
            }
#endif
        }
    }

    auto application = configHandler_->getUsrConfigApplication();
    application["update_checked_at"] = QDateTime::currentMSecsSinceEpoch();
    configHandler_->setUsrConfigApplication(application);

    emit checkAllFinished();
}

void UpdateHandler::update(const QString &itemKey)
{
    auto updateAvailableItems = configHandler_->getUsrConfigUpdateAvailableItems();

    if (!updateAvailableItems.contains(itemKey)) {
        return;
    }

    auto updateMethod = updateAvailableItems[itemKey].toObject()["update_method"].toString();

#ifdef QTLIB_UNIX
    if (updateMethod == "appimageupdate") {
        updateAppImage(itemKey);
    }
    //else if (updateMethod == "appimageupdatewithocsapi") {}
#endif
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

void UpdateHandler::updateAppImage(const QString &itemKey)
{
    auto installedItems = configHandler_->getUsrConfigInstalledItems();
    auto updateAvailableItem = configHandler_->getUsrConfigUpdateAvailableItems()[itemKey].toObject();
    auto installedItemKey = updateAvailableItem["installed_item"].toString();

    if (!installedItems.contains(installedItemKey)) {
        return;
    }

    auto installedItem = installedItems[installedItemKey].toObject();

    auto filename = installedItem["filename"].toString();
    auto installType = installedItem["install_type"].toString();

    qtlib::File file(configHandler_->getAppConfigInstallTypes()[installType].toObject()["destination"].toString() + "/" + filename);

    auto newFilename = filename;
    auto updateInformation = describeAppImage(file.path());
    for (const auto &info : updateInformation.split("\n")) {
        if (info.endsWith(".zsync", Qt::CaseInsensitive)) {
            newFilename = info.split("|").last().split("/").last().replace(".zsync", "", Qt::CaseInsensitive);
            break;
        }
    }

    appimage::update::Updater appImageUpdater(file.path().toStdString(), false);
    if (!appImageUpdater.start()) {
        return;
    }

    emit updateStarted(itemKey);

    while (!appImageUpdater.isDone()) {
        QThread::msleep(100);
        double progress;
        if (appImageUpdater.progress(progress)) {
            emit updateProgress(itemKey, progress * 100);
        }
    }

    if (appImageUpdater.hasError()) {
        std::string nextMessage;
        while (appImageUpdater.nextStatusMessage(nextMessage)) {
            qWarning() << QString::fromStdString(nextMessage);
        }

        emit updateFinished(itemKey);
        return;
    }

    installedItem["filename"] = newFilename;
    QJsonArray files;
    files.append(QJsonValue(newFilename));
    installedItem["files"] = files;
    installedItem["installed_at"] = QDateTime::currentMSecsSinceEpoch();

    configHandler_->setUsrConfigInstalledItemsItem(installedItemKey, installedItem);

    if (newFilename != filename) {
        file.remove();
    }

    configHandler_->removeUsrConfigUpdateAvailableItemsItem(itemKey);

    emit updateFinished(itemKey);
}
#endif
