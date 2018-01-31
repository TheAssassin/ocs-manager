#pragma once

#include <QObject>
#include <QJsonObject>

#include "qtil_config.h"

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
    QJsonObject getUsrConfigUpdateAvailableItems() const;
    bool setUsrConfigUpdateAvailableItems(const QJsonObject &object) const;

    bool setUsrConfigProvidersProvider(const QString &providerKey, const QJsonObject &object) const;
    bool removeUsrConfigProvidersProvider(const QString &providerKey) const;
    bool setUsrConfigCategoriesProvider(const QString &providerKey, const QJsonObject &object) const;
    bool removeUsrConfigCategoriesProvider(const QString &providerKey) const;
    bool setUsrConfigCategoriesInstallType(const QString &providerKey, const QString &categoryKey, const QString &installType) const;
    bool setUsrConfigInstalledItemsItem(const QString &itemKey, const QJsonObject &object) const;
    bool removeUsrConfigInstalledItemsItem(const QString &itemKey) const;
    bool setUsrConfigUpdateAvailableItemsItem(const QString &itemKey, const QJsonObject &object) const;
    bool removeUsrConfigUpdateAvailableItemsItem(const QString &itemKey) const;

private:
    void importAppConfigApplication();
    void importAppConfigInstallTypes();
    QString convertPathString(const QString &path) const;

    qtil::Config appConfig_;
    qtil::Config usrConfig_;
    QJsonObject appConfigApplication_;
    QJsonObject appConfigInstallTypes_;
};
