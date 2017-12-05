#include "itemhandler.h"

#include <QUrlQuery>
#include <QJsonValue>
#include <QJsonArray>
#include <QFileInfo>
#include <QDateTime>

#ifdef QTLIB_UNIX
#include <QProcess>
#endif

#include "qtlib_dir.h"
#include "qtlib_file.h"
#include "qtlib_networkresource.h"
#include "qtlib_package.h"

#include "handlers/confighandler.h"

ItemHandler::ItemHandler(ConfigHandler *configHandler, QObject *parent)
    : QObject(parent), configHandler_(configHandler)
{}

QJsonObject ItemHandler::metadataSet() const
{
    return metadataSet_;
}

void ItemHandler::getItem(const QString &command, const QString &url, const QString &installType, const QString &filename,
                          const QString &providerKey, const QString &contentId)
{
    // Use URL as unique key for metadata, network resource, and installed item
    auto itemKey = url;

    QJsonObject metadata;
    metadata["command"] = command;
    metadata["url"] = url;
    metadata["install_type"] = installType;
    metadata["filename"] = filename;
    if (filename.isEmpty()) {
        metadata["filename"] = QUrl(url).fileName();
    }
    metadata["provider"] = providerKey;
    metadata["content_id"] = contentId;
    metadata["files"] = QJsonArray();
    metadata["installed_at"] = qint64();

    QJsonObject result;
    result["metadata"] = metadata;

    if (command == "install" && configHandler_->getUsrConfigInstalledItems().contains(itemKey)) {
        result["status"] = QString("error_downloadstart");
        result["message"] = tr("The item already installed");
        emit downloadStarted(result);
        return;
    }

    auto itemMetadataSet = metadataSet();

    if (itemMetadataSet.contains(itemKey)) {
        result["status"] = QString("error_downloadstart");
        result["message"] = tr("The file already downloading");
        emit downloadStarted(result);
        return;
    }

    itemMetadataSet[itemKey] = metadata;
    setMetadataSet(itemMetadataSet);

    auto *resource = new qtlib::NetworkResource(itemKey, QUrl(url), true, this);
    connect(resource, &qtlib::NetworkResource::downloadProgress, this, &ItemHandler::downloadProgress);
    connect(resource, &qtlib::NetworkResource::finished, this, &ItemHandler::networkResourceFinished);
    resource->get();

    result["status"] = QString("success_downloadstart");
    result["message"] = tr("Downloading");
    emit downloadStarted(result);
}

void ItemHandler::getItemByOcsUrl(const QString &ocsUrl, const QString &providerKey, const QString &contentId)
{
    QUrl ocsUrlObj(ocsUrl);
    QUrlQuery query(ocsUrlObj);

    QString scheme = "ocs";
    QString command = "download";
    QString url = "";
    QString type = "downloads";
    QString filename = "";

    if (!ocsUrlObj.scheme().isEmpty()) {
        scheme = ocsUrlObj.scheme();
    }

    if (!ocsUrlObj.host().isEmpty()) {
        command = ocsUrlObj.host();
    }

    if (query.hasQueryItem("url") && !query.queryItemValue("url").isEmpty()) {
        url = query.queryItemValue("url", QUrl::FullyDecoded);
    }

    if (query.hasQueryItem("type") && !query.queryItemValue("type").isEmpty()) {
        type = query.queryItemValue("type", QUrl::FullyDecoded);
    }

    if (query.hasQueryItem("filename") && !query.queryItemValue("filename").isEmpty()) {
        filename = QUrl(query.queryItemValue("filename", QUrl::FullyDecoded)).fileName();
    }

    if (!url.isEmpty() && filename.isEmpty()) {
        filename = QUrl(url).fileName();
    }

    // Still support xdg and xdgs schemes for backward compatibility
    if ((scheme == "ocs" || scheme == "ocss" || scheme == "xdg" || scheme == "xdgs")
            && (command == "download" || command == "install")
            && QUrl(url).isValid()
            && configHandler_->getAppConfigInstallTypes().contains(type)
            && !filename.isEmpty()) {
        getItem(command, url, type, filename, providerKey, contentId);
    }
    else {
        QJsonObject result;
        result["status"] = QString("error_downloadstart");
        result["message"] = tr("Invalid OCS-URL");
        emit downloadStarted(result);
    }
}

void ItemHandler::uninstall(const QString &itemKey)
{
    QJsonObject result;

    if (!configHandler_->getUsrConfigInstalledItems().contains(itemKey)) {
        result["status"] = QString("error_uninstallstart");
        result["message"] = tr("The item not installed");
        emit uninstallStarted(result);
        return;
    }

    result["status"] = QString("success_uninstallstart");
    result["message"] = tr("Uninstalling");
    emit uninstallStarted(result);

    auto installedItem = configHandler_->getUsrConfigInstalledItems()[itemKey].toObject();
    auto installType = installedItem["install_type"].toString();

    qtlib::Dir destDir;
#ifdef QTLIB_UNIX
    destDir.setPath(configHandler_->getAppConfigInstallTypes()[installType].toObject()["destination"].toString());

    for (const auto &filename : installedItem["files"].toArray()) {
        QFileInfo fileInfo(destDir.path() + "/" + filename.toString());

        // plasmapkg: Installation process has should be saved plasmapkg into destination directory

        qtlib::Package package(fileInfo.filePath());

        // Uninstall
        if (installType == "bin") {
            if (fileInfo.filePath().endsWith(".appimage", Qt::CaseInsensitive)) {
                QProcess process;
                process.start(fileInfo.filePath() + " --remove-appimage-desktop-integration");
                process.waitForFinished();
            }
        }
        else if (installType == "plasma_plasmoids" || installType == "plasma4_plasmoids" || installType == "plasma5_plasmoids") {
            package.uninstallAsPlasmapkg("plasmoid");
        }
        else if (installType == "plasma_look_and_feel" || installType == "plasma5_look_and_feel") {
            package.uninstallAsPlasmapkg("lookandfeel");
        }
        else if (installType == "plasma_desktopthemes" || installType == "plasma5_desktopthemes") {
            package.uninstallAsPlasmapkg("theme");
        }
        else if (installType == "kwin_effects") {
            package.uninstallAsPlasmapkg("kwineffect");
        }
        else if (installType == "kwin_scripts") {
            package.uninstallAsPlasmapkg("kwinscript");
        }
        else if (installType == "kwin_tabbox") {
            package.uninstallAsPlasmapkg("windowswitcher");
        }

        // Remove file
        if (fileInfo.isDir()) {
            qtlib::Dir(fileInfo.filePath()).remove();
        }
        else {
            qtlib::File(fileInfo.filePath()).remove();
        }
    }
#else
    destDir.setPath(configHandler_->getAppConfigInstallTypes()[installType].toObject()["generic_destination"].toString());

    for (const auto &filename : installedItem["files"].toArray()) {
        QFileInfo fileInfo(destDir.path() + "/" + filename.toString());
        if (fileInfo.isDir()) {
            qtlib::Dir(fileInfo.filePath()).remove();
        }
        else {
            qtlib::File(fileInfo.filePath()).remove();
        }
    }
#endif

    configHandler_->removeUsrConfigInstalledItemsItem(itemKey);
    configHandler_->removeUsrConfigUpdateAvailableItemsItem(itemKey);

    result["status"] = QString("success_uninstall");
    result["message"] = tr("Uninstalled");
    emit uninstallFinished(result);
}

void ItemHandler::networkResourceFinished(qtlib::NetworkResource *resource)
{
    auto itemKey = resource->id();

    auto itemMetadataSet = metadataSet();
    auto metadata = itemMetadataSet[itemKey].toObject();

    QJsonObject result;
    result["metadata"] = metadata;

    if (!resource->isFinishedWithNoError()) {
        itemMetadataSet.remove(itemKey);
        setMetadataSet(itemMetadataSet);
        result["status"] = QString("error_download");
        result["message"] = resource->reply()->errorString();
        emit downloadFinished(result);
        resource->deleteLater();
        return;
    }

    result["status"] = QString("success_download");
    result["message"] = tr("Downloaded");
    emit downloadFinished(result);

    if (metadata["command"].toString() == "download") {
        saveDownloadedFile(resource);
    }
    else if (metadata["command"].toString() == "install") {
        installDownloadedFile(resource);
    }
}

void ItemHandler::setMetadataSet(const QJsonObject &metadataSet)
{
    metadataSet_ = metadataSet;
    emit metadataSetChanged();
}

void ItemHandler::saveDownloadedFile(qtlib::NetworkResource *resource)
{
    auto itemKey = resource->id();

    auto itemMetadataSet = metadataSet();
    auto metadata = itemMetadataSet[itemKey].toObject();

    itemMetadataSet.remove(itemKey);
    setMetadataSet(itemMetadataSet);

    QJsonObject result;
    result["metadata"] = metadata;
    result["status"] = QString("success_savestart");
    result["message"] = tr("Saving");
    emit saveStarted(result);

    auto filename = metadata["filename"].toString();
    auto installType = metadata["install_type"].toString();

    qtlib::Dir destDir(configHandler_->getAppConfigInstallTypes()[installType].toObject()["destination"].toString());
    destDir.make();
    qtlib::File destFile(destDir.path() + "/" + filename);

    if (destFile.exists()) {
        auto filenamePrefix = QString::number(QDateTime::currentMSecsSinceEpoch()) + "_";
        destFile.setPath(destDir.path() + "/" + filenamePrefix + filename);
    }

    if (!resource->saveData(destFile.path())) {
        result["status"] = QString("error_save");
        result["message"] = tr("Failed to save data");
        emit saveFinished(result);
        resource->deleteLater();
        return;
    }

    result["status"] = QString("success_save");
    result["message"] = tr("Saved");
    emit saveFinished(result);

    resource->deleteLater();
}

void ItemHandler::installDownloadedFile(qtlib::NetworkResource *resource)
{
    // Installation pre-process
    auto itemKey = resource->id();

    auto itemMetadataSet = metadataSet();
    auto metadata = itemMetadataSet[itemKey].toObject();

    itemMetadataSet.remove(itemKey);
    setMetadataSet(itemMetadataSet);

    QJsonObject result;
    result["metadata"] = metadata;
    result["status"] = QString("success_savestart");
    result["message"] = tr("Saving");
    emit saveStarted(result);

    auto filename = metadata["filename"].toString();
    auto installType = metadata["install_type"].toString();

    QString tempDirPrefix = "temp_" + filename;
    qtlib::Dir tempDir(qtlib::Dir::genericCachePath() + "/"
                       + configHandler_->getAppConfigApplication()["id"].toString() + "/"
                       + tempDirPrefix);
    tempDir.make();
    qtlib::Dir tempDestDir(tempDir.path() + "/dest");
    tempDestDir.make();
    qtlib::Package package(tempDir.path() + "/" + filename);

    if (!resource->saveData(package.path())) {
        result["status"] = QString("error_save");
        result["message"] = tr("Failed to save data");
        emit saveFinished(result);
        tempDir.remove();
        resource->deleteLater();
        return;
    }

    result["status"] = QString("success_save");
    result["message"] = tr("Saved");
    emit saveFinished(result);

    // Installation main-process
    result["status"] = QString("success_installstart");
    result["message"] = tr("Installing");
    emit installStarted(result);

    qtlib::Dir destDir;
#ifdef QTLIB_UNIX
    destDir.setPath(configHandler_->getAppConfigInstallTypes()[installType].toObject()["destination"].toString());

    // plasmapkg: Need to save package to remove installed files later

    if (installType == "bin"
            && package.installAsProgram(tempDestDir.path() + "/" + filename)) {
        result["message"] = tr("The file has been installed as program");
    }
    else if ((installType == "plasma_plasmoids" || installType == "plasma4_plasmoids" || installType == "plasma5_plasmoids")
             && package.installAsPlasmapkg("plasmoid")) {
        package.installAsFile(tempDestDir.path() + "/" + filename);
        result["message"] = tr("The plasmoid has been installed");
    }
    else if ((installType == "plasma_look_and_feel" || installType == "plasma5_look_and_feel")
             && package.installAsPlasmapkg("lookandfeel")) {
        package.installAsFile(tempDestDir.path() + "/" + filename);
        result["message"] = tr("The plasma look and feel has been installed");
    }
    /*else if ((installType == "plasma_desktopthemes" || installType == "plasma5_desktopthemes")
             && package.installAsPlasmapkg("theme")) {
        package.installAsFile(tempDestDir.path() + "/" + filename);
        result["message"] = tr("The plasma desktop theme has been installed");
    }*/
    else if (installType == "kwin_effects"
             && package.installAsPlasmapkg("kwineffect")) {
        package.installAsFile(tempDestDir.path() + "/" + filename);
        result["message"] = tr("The KWin effect has been installed");
    }
    else if (installType == "kwin_scripts"
             && package.installAsPlasmapkg("kwinscript")) {
        package.installAsFile(tempDestDir.path() + "/" + filename);
        result["message"] = tr("The KWin script has been installed");
    }
    else if (installType == "kwin_tabbox"
             && package.installAsPlasmapkg("windowswitcher")) {
        package.installAsFile(tempDestDir.path() + "/" + filename);
        result["message"] = tr("The KWin window switcher has been installed");
    }
    else if (package.installAsArchive(tempDestDir.path())) {
        result["message"] = tr("The archive file has been extracted");
    }
    else if (package.installAsFile(tempDestDir.path() + "/" + filename)) {
        result["message"] = tr("The file has been installed");
    }
    else {
        result["status"] = QString("error_install");
        result["message"] = tr("Failed to installation");
        emit installFinished(result);
        tempDir.remove();
        resource->deleteLater();
        return;
    }
#else
    destDir.setPath(configHandler_->getAppConfigInstallTypes()[installType].toObject()["generic_destination"].toString());

    if (qtlib::File(package.path()).copy(tempDestDir.path() + "/" + filename)) {
        result["message"] = tr("The file has been installed");
    }
    else {
        result["status"] = QString("error_install");
        result["message"] = tr("Failed to installation");
        emit installFinished(result);
        tempDir.remove();
        resource->deleteLater();
        return;
    }
#endif

    destDir.make();

    QJsonArray installedFiles;
    auto filenamePrefix = QString::number(QDateTime::currentMSecsSinceEpoch()) + "_";

    for (const auto &fileInfo : tempDestDir.list()) {
        auto destFilename = fileInfo.fileName();
        if (QFileInfo::exists(destDir.path() + "/" + destFilename)) {
            destFilename = filenamePrefix + destFilename;
        }

        if (fileInfo.isDir()) {
            qtlib::Dir(fileInfo.filePath()).move(destDir.path() + "/" + destFilename);
        }
        else {
            qtlib::File(fileInfo.filePath()).move(destDir.path() + "/" + destFilename);
        }

        installedFiles.append(QJsonValue(destFilename));
    }

    // Installation post-process
    metadata.remove("command");
    metadata["files"] = installedFiles;
    metadata["installed_at"] = QDateTime::currentMSecsSinceEpoch();
    configHandler_->setUsrConfigInstalledItemsItem(itemKey, metadata);

    result["metadata"] = metadata;
    result["status"] = QString("success_install");
    emit installFinished(result);

    tempDir.remove();
    resource->deleteLater();
}
