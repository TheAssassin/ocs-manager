#include "confighandler.h"

#include <QStringList>

#include "qtlib_dir.h"

ConfigHandler::ConfigHandler(QObject *parent)
    : QObject(parent)
{
    appConfig_ = qtlib::Config(":/configs");
    usrConfig_ = qtlib::Config(qtlib::Dir::genericConfigPath() + "/" + getAppConfigApplication()["id"].toString());
}

QJsonObject ConfigHandler::getAppConfigApplication()
{
    if (appConfigApplication_.isEmpty()) {
        appConfigApplication_ = appConfig_.get("application");
    }
    return appConfigApplication_;
}

QJsonObject ConfigHandler::getAppConfigInstallTypes()
{
    if (appConfigInstallTypes_.isEmpty()) {
        QJsonObject installTypes = appConfig_.get("install_types");
        foreach (const QString &key, installTypes.keys()) {
            QJsonObject installtype = installTypes[key].toObject();
            installtype["destination"] = convertPathString(installtype["destination"].toString());
            installtype["generic_destination"] = convertPathString(installtype["generic_destination"].toString());
            installTypes[key] = installtype;
        }
        QJsonObject installTypesAlias = appConfig_.get("install_types_alias");
        foreach (const QString &key, installTypesAlias.keys()) {
            QJsonObject installTypeAlias = installTypesAlias[key].toObject();
            QString baseKey = installTypeAlias["base"].toString();
            if (installTypes.contains(baseKey)) {
                QJsonObject installType = installTypes[baseKey].toObject();
                installType["base"] = baseKey;
                installType["name"] = installTypeAlias["name"].toString();
                installTypes[key] = installType;
            }
        }
        appConfigInstallTypes_ = installTypes;
    }
    return appConfigInstallTypes_;
}

QJsonObject ConfigHandler::getAppConfigFeaturedProviders()
{
    return appConfig_.get("featured_providers");
}

QJsonObject ConfigHandler::getUsrConfigApplication()
{
    return usrConfig_.get("application");
}

bool ConfigHandler::setUsrConfigApplication(const QJsonObject &object)
{
    return usrConfig_.set("application", object);
}

QJsonObject ConfigHandler::getUsrConfigProviders()
{
    return usrConfig_.get("providers");
}

bool ConfigHandler::setUsrConfigProviders(const QJsonObject &object)
{
    return usrConfig_.set("providers", object);
}

QJsonObject ConfigHandler::getUsrConfigCategories()
{
    return usrConfig_.get("categories");
}

bool ConfigHandler::setUsrConfigCategories(const QJsonObject &object)
{
    return usrConfig_.set("categories", object);
}

QJsonObject ConfigHandler::getUsrConfigInstalledItems()
{
    return usrConfig_.get("installed_items");
}

bool ConfigHandler::setUsrConfigInstalledItems(const QJsonObject &object)
{
    return usrConfig_.set("installed_items", object);
}

bool ConfigHandler::setUsrConfigProvidersProvider(const QString &providerKey, const QJsonObject &object)
{
    /* object format
    {
        "id": "example",
        "location": "https://example.com/ocs/v1/",
        "name": "Example",
        "icon": "https://example.com/icon.png",
        "termsofuse": "https://example.com/termsofuse",
        "register": "https://example.com/register",
        "_providerfile": "https://example.com/ocs/providers.xml"
    }
    */
    QJsonObject providers = getUsrConfigProviders();
    providers[providerKey] = object;
    return setUsrConfigProviders(providers);
}

bool ConfigHandler::removeUsrConfigProvidersProvider(const QString &providerKey)
{
    QJsonObject providers = getUsrConfigProviders();
    providers.remove(providerKey);
    return setUsrConfigProviders(providers);
}

bool ConfigHandler::setUsrConfigCategoriesProvider(const QString &providerKey, const QJsonObject &object)
{
    /* object format
    {
        "1": {
            "id": "1",
            "name": "Desktop Icons",
            "install_type": "icons"
        },
        "2": {
            "id": "2",
            "name": "Desktop Themes",
            "install_type": "themes"
        }
    }
    */
    QJsonObject categories = getUsrConfigCategories();
    categories[providerKey] = object;
    return setUsrConfigCategories(categories);
}

bool ConfigHandler::removeUsrConfigCategoriesProvider(const QString &providerKey)
{
    QJsonObject categories = getUsrConfigCategories();
    categories.remove(providerKey);
    return setUsrConfigCategories(categories);
}

bool ConfigHandler::setUsrConfigCategoriesInstallType(const QString &providerKey, const QString &categoryKey, const QString &installType)
{
    QJsonObject categories = getUsrConfigCategories();
    QJsonObject providerCategories;
    if (categories.contains(providerKey)) {
        providerCategories = categories[providerKey].toObject();
    }
    QJsonObject providerCategory;
    if (providerCategories.contains(categoryKey)) {
        providerCategory = providerCategories[categoryKey].toObject();
    }
    providerCategory["install_type"] = installType;
    providerCategories[categoryKey] = providerCategory;
    categories[providerKey] = providerCategories;
    return setUsrConfigCategories(categories);
}

bool ConfigHandler::setUsrConfigInstalledItemsItem(const QString &itemKey, const QJsonObject &object)
{
    /* object format
    {
        "url": "http://example.com/downloads/123-1.tar.gz",
        "filename": "123-1.tar.gz",
        "install_type": "icons",
        "provider": "http://example.com/ocs/v1/",
        "content_id": "123",
        "files": [
            "iconset-light",
            "iconset-dark"
        ],
        "installed_at": 1483658977219
    }
    */
    QJsonObject installedItems = getUsrConfigInstalledItems();
    installedItems[itemKey] = object;
    return setUsrConfigInstalledItems(installedItems);
}

bool ConfigHandler::removeUsrConfigInstalledItemsItem(const QString &itemKey)
{
    QJsonObject installedItems = getUsrConfigInstalledItems();
    installedItems.remove(itemKey);
    return setUsrConfigInstalledItems(installedItems);
}

QString ConfigHandler::convertPathString(const QString &path)
{
    QString newPath = path;
    if (newPath.contains("$HOME")) {
        newPath.replace("$HOME", qtlib::Dir::homePath());
    }
    else if (newPath.contains("$XDG_DATA_HOME")) {
        newPath.replace("$XDG_DATA_HOME", qtlib::Dir::genericDataPath());
    }
    else if (newPath.contains("$KDEHOME")) {
        newPath.replace("$KDEHOME", qtlib::Dir::kdehomePath());
    }
    else if (newPath.contains("$APP_DATA")) {
        newPath.replace("$APP_DATA", qtlib::Dir::genericDataPath() + "/" + getAppConfigApplication()["id"].toString());
    }
    return newPath;
}
