#pragma once

#include <QObject>
#include <QJsonObject>

#include "qtlib_config.h"

class ConfigHandler : public QObject
{
    Q_OBJECT

public:
    explicit ConfigHandler(QObject *parent = nullptr);

public slots:
    QJsonObject getAppConfigApplication() const;
    QJsonObject getAppConfigInstallTypes() const;

    QJsonObject getUsrConfigApplication() const;
    bool setUsrConfigApplication(const QJsonObject &object) const;
    QJsonObject getUsrConfigProviders() const;
    bool setUsrConfigProviders(const QJsonObject &object) const;
    QJsonObject getUsrConfigCategories() const;
    bool setUsrConfigCategories(const QJsonObject &object) const;
    QJsonObject getUsrConfigInstalledItems() const;
    bool setUsrConfigInstalledItems(const QJsonObject &object) const;
    QJsonObject getUsrConfigUpdateAvailable() const;
    bool setUsrConfigUpdateAvailable(const QJsonObject &object) const;

    bool setUsrConfigProvidersProvider(const QString &providerKey, const QJsonObject &object) const;
    bool removeUsrConfigProvidersProvider(const QString &providerKey) const;
    bool setUsrConfigCategoriesProvider(const QString &providerKey, const QJsonObject &object) const;
    bool removeUsrConfigCategoriesProvider(const QString &providerKey) const;
    bool setUsrConfigCategoriesInstallType(const QString &providerKey, const QString &categoryKey, const QString &installType) const;
    bool setUsrConfigInstalledItemsItem(const QString &itemKey, const QJsonObject &object) const;
    bool removeUsrConfigInstalledItemsItem(const QString &itemKey) const;
    bool setUsrConfigUpdateAvailableFile(const QString &fileKey, const QJsonObject &object) const;
    bool removeUsrConfigUpdateAvailableFile(const QString &fileKey) const;

private:
    void importAppConfigApplication();
    void importAppConfigInstallTypes();
    QString convertPathString(const QString &path) const;

    qtlib::Config appConfig_;
    qtlib::Config usrConfig_;
    QJsonObject appConfigApplication_;
    QJsonObject appConfigInstallTypes_;
};
