#include "ocsapihandler.h"

#include <QStringList>
#include <QJsonValue>

#include "qtlib_ocsapi.h"

#include "handlers/confighandler.h"

OcsApiHandler::OcsApiHandler(ConfigHandler *configHandler, QObject *parent)
    : QObject(parent), configHandler_(configHandler)
{}

bool OcsApiHandler::addProviders(const QString &providerFileUrl)
{
    auto providers = qtlib::OcsApi::getProviderFile(QUrl(providerFileUrl));
    if (!providers.isEmpty()) {
        for (const auto &providerValue : providers) {
            auto provider = providerValue.toObject();
            if (provider.contains("location")) {
                // Use location (API base URL) as unique key
                auto providerKey = provider["location"].toString();
                if (configHandler_->setUsrConfigProvidersProvider(providerKey, provider)) {
                    updateCategories(providerKey, true);
                }
            }
        }
        return true;
    }
    return false;
}

bool OcsApiHandler::removeProvider(const QString &providerKey)
{
    if (configHandler_->removeUsrConfigProvidersProvider(providerKey)) {
        configHandler_->removeUsrConfigCategoriesProvider(providerKey);
        return true;
    }
    return false;
}

bool OcsApiHandler::updateAllCategories(bool force)
{
    auto providers = configHandler_->getUsrConfigProviders();
    if (!providers.isEmpty()) {
        for (const auto &providerKey : providers.keys()) {
            updateCategories(providerKey, force);
        }
        return true;
    }
    return false;
}

bool OcsApiHandler::updateCategories(const QString &providerKey, bool force)
{
    auto providers = configHandler_->getUsrConfigProviders();

    if (!providers.contains(providerKey)) {
        return false;
    }

    auto baseUrl = providers[providerKey].toObject()["location"].toString();
    auto response = qtlib::OcsApi(baseUrl, QUrl(baseUrl)).getContentCategories();

    if (!response.contains("data")) {
        return false;
    }

    // Data type variation workaround, convert object to array
    QJsonArray responseData;
    if (response["data"].isObject()) {
        for (const auto &dataValue : response["data"].toObject()) {
            responseData.append(dataValue);
        }
    }
    else {
        responseData = response["data"].toArray();
    }

    auto installTypes = configHandler_->getAppConfigInstallTypes();

    auto categories = configHandler_->getUsrConfigCategories();
    QJsonObject providerCategories;
    if (!force && categories.contains(providerKey)) {
        providerCategories = categories[providerKey].toObject();
    }

    QJsonObject newProviderCategories;
    for (const auto &dataValue : responseData) {
        auto data = dataValue.toObject();

        // Data type variation workaround, convert int to string
        QString id;
        if (data["id"].isString()) {
            id = data["id"].toString();
        }
        else {
            id = QString::number(data["id"].toInt());
        }

        // Use category id as unique key
        auto categoryKey = id;

        auto name = data["name"].toString();
        // display_name: Not compatible to legacy OCS-API
        if (data.contains("display_name") && data["display_name"].toString() != "") {
            name = data["display_name"].toString();
        }

        // parent_id: Not compatible to legacy OCS-API
        QString parentId = "";
        if (data.contains("parent_id") && data["parent_id"].toString() != "") {
            parentId = data["parent_id"].toString();
        }

        auto installType = configHandler_->getAppConfigApplication()["options"].toObject()["default_install_type"].toString();
        if (!force && providerCategories.contains(categoryKey)) {
            installType = providerCategories[categoryKey].toObject()["install_type"].toString();
        }
        // xdg_type: Not compatible to legacy OCS-API
        else if (data.contains("xdg_type") && data["xdg_type"].toString() != ""
                && installTypes.contains(data["xdg_type"].toString())) {
            installType = data["xdg_type"].toString();
        }

        QJsonObject category;
        category["id"] = id;
        category["name"] = name;
        category["parent_id"] = parentId;
        category["install_type"] = installType;
        newProviderCategories[categoryKey] = category;
    }

    return configHandler_->setUsrConfigCategoriesProvider(providerKey, newProviderCategories);
}

QJsonObject OcsApiHandler::getContents(const QString &providerKeys, const QString &categoryKeys,
                                       const QString &xdgTypes, const QString &packageTypes,
                                       const QString &search, const QString &sortmode, int pagesize, int page)
{
    QJsonObject responseSet;

    QStringList providerKeyList;
    if (!providerKeys.isEmpty()) {
        providerKeyList = providerKeys.split(",");
    }

    QStringList categoryKeyList;
    if (!categoryKeys.isEmpty()) {
        categoryKeyList = categoryKeys.split(",");
    }

    auto providers = configHandler_->getUsrConfigProviders();
    auto categories = configHandler_->getUsrConfigCategories();

    for (const auto &providerKey : providers.keys()) {
        if (!providerKeyList.isEmpty() && !providerKeyList.contains(providerKey)) {
            continue;
        }
        QStringList categoryIdList;
        auto providerCategories = categories[providerKey].toObject();
        for (const auto &categoryKey : providerCategories.keys()) {
            if (!categoryKeyList.isEmpty() && !categoryKeyList.contains(categoryKey)) {
                continue;
            }
            categoryIdList.append(providerCategories[categoryKey].toObject()["id"].toString());
        }
        if (!categoryIdList.isEmpty()) {
            auto baseUrl = providers[providerKey].toObject()["location"].toString();
            QUrlQuery query;
            // categories: Comma-separated list is not compatible to legacy OCS-API
            //query.addQueryItem("categories", categoryIdList.join(","));
            query.addQueryItem("categories", categoryIdList.join("x"));
            // xdg_types: Not compatible to legacy OCS-API
            if (!xdgTypes.isEmpty()) {
                query.addQueryItem("xdg_types", xdgTypes);
            }
            // package_types: Not compatible to legacy OCS-API
            if (!packageTypes.isEmpty()) {
                query.addQueryItem("package_types", packageTypes);
            }
            if (!search.isEmpty()) {
                query.addQueryItem("search", search);
            }
            query.addQueryItem("sortmode", sortmode);
            query.addQueryItem("pagesize", QString::number(pagesize));
            query.addQueryItem("page", QString::number(page));
            responseSet[providerKey] = qtlib::OcsApi(baseUrl, QUrl(baseUrl)).getContentDataSet(query);
        }
    }

    return responseSet;
}

QJsonObject OcsApiHandler::getContent(const QString &providerKey, const QString &contentId)
{
    QJsonObject response;
    auto providers = configHandler_->getUsrConfigProviders();
    if (providers.contains(providerKey)) {
        auto baseUrl = providers[providerKey].toObject()["location"].toString();
        response = qtlib::OcsApi(baseUrl, QUrl(baseUrl)).getContentData(contentId);
    }
    return response;
}
