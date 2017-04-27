#pragma once

#include <QObject>
#include <QJsonObject>

#include "qtlib_config.h"

class ConfigHandler : public QObject
{
    Q_OBJECT

public:
    explicit ConfigHandler(QObject *parent = 0);

public slots:
    QJsonObject getAppConfigApplication();
    QJsonObject getAppConfigInstallTypes();

    QJsonObject getUsrConfigApplication();
    bool setUsrConfigApplication(const QJsonObject &object);
    QJsonObject getUsrConfigProviders();
    bool setUsrConfigProviders(const QJsonObject &object);
    QJsonObject getUsrConfigCategories();
    bool setUsrConfigCategories(const QJsonObject &object);
    QJsonObject getUsrConfigInstalledItems();
    bool setUsrConfigInstalledItems(const QJsonObject &object);

    bool setUsrConfigProvidersProvider(const QString &providerKey, const QJsonObject &object);
    bool removeUsrConfigProvidersProvider(const QString &providerKey);
    bool setUsrConfigCategoriesProvider(const QString &providerKey, const QJsonObject &object);
    bool removeUsrConfigCategoriesProvider(const QString &providerKey);
    bool setUsrConfigCategoriesInstallType(const QString &providerKey, const QString &categoryKey, const QString &installType);
    bool setUsrConfigInstalledItemsItem(const QString &itemKey, const QJsonObject &object);
    bool removeUsrConfigInstalledItemsItem(const QString &itemKey);

private:
    QString convertPathString(const QString &path);

    qtlib::Config appConfig_;
    qtlib::Config usrConfig_;
    QJsonObject appConfigApplication_;
    QJsonObject appConfigInstallTypes_;
};
