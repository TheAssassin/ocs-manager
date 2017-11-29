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

    auto application = configHandler_->getUsrConfigApplication();
    auto installedItems = configHandler_->getUsrConfigInstalledItems();

    if (installedItems.isEmpty() || application.contains("update_checked_at")) {
        auto currentDate = QDateTime::currentDateTime();
        auto checkedDate = QDateTime::fromMSecsSinceEpoch(application["update_checked_at"].toInt());
        if (currentDate.daysTo(checkedDate.addDays(1)) <= 0) {
            emit checkAllFinished();
            return;
        }
    }

    // Clear data
    QJsonObject updateAvailable;
    configHandler_->setUsrConfigUpdateAvailable(updateAvailable);

    for (const auto &itemKey : installedItems.keys()) {
        auto installedItem = installedItems[itemKey].toObject();
        auto installType = installedItem["install_type"].toString();

        QString destDir = "";
#ifdef QTLIB_UNIX
        destDir = configHandler_->getAppConfigInstallTypes()[installType].toObject()["destination"].toString();
#else
        destDir = configHandler_->getAppConfigInstallTypes()[installType].toObject()["generic_destination"].toString();
#endif

        if (installType == "bin") {
            for (const auto &filenameValue : installedItem["files"].toArray()) {
                auto filename = filenameValue.toString();
                QString path = destDir + "/" + filename;
                // Use file path as unique key for entry in update_available data
                auto fileKey = path;
                QJsonObject updateAvailableFile;

#ifdef QTLIB_UNIX
                if (filename.endsWith(".appimage", Qt::CaseInsensitive)) {
                    if (checkAppImage(path)) {
                        updateAvailableFile["path"] = path;
                        updateAvailableFile["filename"] = filename;
                        updateAvailableFile["installed_item"] = itemKey;
                        updateAvailableFile["update_method"] = QString("appimageupdate");
                        configHandler_->setUsrConfigUpdateAvailableFile(fileKey, updateAvailableFile);
                    }
                    //else if (checkAppImageWithOcsApi(itemKey, filename)) {}
                }
#endif
            }
        }
    }

    application["update_checked_at"] = QDateTime::currentMSecsSinceEpoch();
    configHandler_->setUsrConfigApplication(application);

    emit checkAllFinished();
}

void UpdateHandler::update(const QString &fileKey)
{
    if (configHandler_->getUsrConfigUpdateAvailable().contains(fileKey)) {
        auto updateMethod = configHandler_->getUsrConfigUpdateAvailable()[fileKey].toObject()["update_method"].toString();

#ifdef QTLIB_UNIX
        if (updateMethod == "appimageupdate") {
            updateAppImage(fileKey);
        }
        //else if (updateMethod == "appimageupdatewithocsapi") {
        //}
#endif
    }
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

void UpdateHandler::updateAppImage(const QString &fileKey)
{
    auto updateAvailableFile = configHandler_->getUsrConfigUpdateAvailable()[fileKey].toObject();

    auto path = updateAvailableFile["path"].toString();
    auto filename = updateAvailableFile["filename"].toString();
    auto itemKey = updateAvailableFile["installed_item"].toString();

    auto newFilename = filename;
    auto updateInformation = describeAppImage(path);
    for (const auto &info : updateInformation.split("\n")) {
        if (info.endsWith(".zsync", Qt::CaseInsensitive)) {
            newFilename = info.split("|").last().split("/").last().replace(".zsync", "", Qt::CaseInsensitive);
            break;
        }
    }

    appimage::update::Updater appImageUpdater(path.toStdString(), false);
    if (appImageUpdater.start()) {
        emit updateStarted(fileKey);

        while (!appImageUpdater.isDone()) {
            QThread::msleep(100);
            double progress;
            if (appImageUpdater.progress(progress)) {
                emit updateProgress(fileKey, progress * 100);
            }
        }

        if (!appImageUpdater.hasError()) {
            if (newFilename != filename) {
                auto installedItem = configHandler_->getUsrConfigInstalledItems()[itemKey].toObject();
                QJsonArray files;
                for (const auto &file : installedItem["files"].toArray()) {
                    if (file.toString() == filename) {
                        files.append(QJsonValue(newFilename));
                    }
                    else {
                        files.append(file);
                    }
                }
                installedItem["files"] = files;
                configHandler_->setUsrConfigInstalledItemsItem(itemKey, installedItem);
                qtlib::File(path).remove();
            }

            configHandler_->removeUsrConfigUpdateAvailableFile(fileKey);
        }
        else {
            std::string nextMessage;
            while (appImageUpdater.nextStatusMessage(nextMessage)) {
                qWarning() << QString::fromStdString(nextMessage);
            }
        }

        emit updateFinished(fileKey);
    }
}
#endif
