#include "websocketserver.h"

#include <QHostAddress>
#include <QWebSocketServer>
#include <QWebSocket>

#include "qtlib_json.h"

#include "handlers/confighandler.h"
#include "handlers/systemhandler.h"
#include "handlers/ocsapihandler.h"
#include "handlers/itemhandler.h"

WebSocketServer::WebSocketServer(ConfigHandler *configHandler, const QString &serverName, quint16 serverPort, QObject *parent)
    : QObject(parent), configHandler_(configHandler), serverName_(serverName), serverPort_(serverPort)
{
    wsServer_ = new QWebSocketServer(serverName_, QWebSocketServer::NonSecureMode, this);
    connect(wsServer_, &QWebSocketServer::newConnection, this, &WebSocketServer::wsNewConnection);
    connect(wsServer_, &QWebSocketServer::closed, this, &WebSocketServer::stopped);

    configHandler_->setParent(this);
    systemHandler_ = new SystemHandler(this);
    ocsApiHandler_ = new OcsApiHandler(configHandler_, this);
    itemHandler_ = new ItemHandler(configHandler_, this);
    connect(itemHandler_, &ItemHandler::metadataSetChanged, this, &WebSocketServer::itemMetadataSetChanged);
    connect(itemHandler_, &ItemHandler::downloadStarted, this, &WebSocketServer::itemDownloadStarted);
    connect(itemHandler_, &ItemHandler::downloadFinished, this, &WebSocketServer::itemDownloadFinished);
    connect(itemHandler_, &ItemHandler::downloadProgress, this, &WebSocketServer::itemDownloadProgress);
    connect(itemHandler_, &ItemHandler::saveStarted, this, &WebSocketServer::itemSaveStarted);
    connect(itemHandler_, &ItemHandler::saveFinished, this, &WebSocketServer::itemSaveFinished);
    connect(itemHandler_, &ItemHandler::installStarted, this, &WebSocketServer::itemInstallStarted);
    connect(itemHandler_, &ItemHandler::installFinished, this, &WebSocketServer::itemInstallFinished);
    connect(itemHandler_, &ItemHandler::uninstallStarted, this, &WebSocketServer::itemUninstallStarted);
    connect(itemHandler_, &ItemHandler::uninstallFinished, this, &WebSocketServer::itemUninstallFinished);
}

WebSocketServer::~WebSocketServer()
{
    stop();
    wsServer_->deleteLater();
}

bool WebSocketServer::start()
{
    if (wsServer_->listen(QHostAddress::Any, serverPort_)) {
        emit started();
        return true;
    }
    return false;
}

void WebSocketServer::stop()
{
    wsServer_->close();
}

bool WebSocketServer::isError() const
{
    if (wsServer_->error() != QWebSocketProtocol::CloseCodeNormal) {
        return true;
    }
    return false;
}

QString WebSocketServer::errorString() const
{
    return wsServer_->errorString();
}

QUrl WebSocketServer::serverUrl() const
{
    return wsServer_->serverUrl();
}

void WebSocketServer::wsNewConnection()
{
    auto *wsClient = wsServer_->nextPendingConnection();
    connect(wsClient, &QWebSocket::disconnected, this, &WebSocketServer::wsDisconnected);
    connect(wsClient, &QWebSocket::textMessageReceived, this, &WebSocketServer::wsTextMessageReceived);
    connect(wsClient, &QWebSocket::binaryMessageReceived, this, &WebSocketServer::wsBinaryMessageReceived);
    wsClients_ << wsClient;
}

void WebSocketServer::wsDisconnected()
{
    auto *wsClient = qobject_cast<QWebSocket *>(sender());
    if (wsClient) {
        wsClients_.removeAll(wsClient);
        wsClient->deleteLater();
    }
}

void WebSocketServer::wsTextMessageReceived(const QString &message)
{
    auto *wsClient = qobject_cast<QWebSocket *>(sender());
    if (wsClient) {
        qtlib::Json json(message.toUtf8());
        if (json.isObject()) {
            auto object = json.toObject();
            receiveMessage(object["id"].toString(), object["func"].toString(), object["data"].toArray());
        }
    }
}

void WebSocketServer::wsBinaryMessageReceived(const QByteArray &message)
{
    auto *wsClient = qobject_cast<QWebSocket *>(sender());
    if (wsClient) {
        qtlib::Json json(message);
        if (json.isObject()) {
            auto object = json.toObject();
            receiveMessage(object["id"].toString(), object["func"].toString(), object["data"].toArray());
        }
    }
}

void WebSocketServer::itemMetadataSetChanged()
{
    QJsonArray data;
    sendMessage("", "ItemHandler::metadataSetChanged", data);
}

void WebSocketServer::itemDownloadStarted(QJsonObject result)
{
    QJsonArray data;
    data.append(result);
    sendMessage("", "ItemHandler::downloadStarted", data);
}

void WebSocketServer::itemDownloadFinished(QJsonObject result)
{
    QJsonArray data;
    data.append(result);
    sendMessage("", "ItemHandler::downloadFinished", data);
}

void WebSocketServer::itemDownloadProgress(QString id, qint64 bytesReceived, qint64 bytesTotal)
{
    QJsonArray data;
    data.append(id);
    data.append(bytesReceived);
    data.append(bytesTotal);
    sendMessage("", "ItemHandler::downloadProgress", data);
}

void WebSocketServer::itemSaveStarted(QJsonObject result)
{
    QJsonArray data;
    data.append(result);
    sendMessage("", "ItemHandler::saveStarted", data);
}

void WebSocketServer::itemSaveFinished(QJsonObject result)
{
    QJsonArray data;
    data.append(result);
    sendMessage("", "ItemHandler::saveFinished", data);
}

void WebSocketServer::itemInstallStarted(QJsonObject result)
{
    QJsonArray data;
    data.append(result);
    sendMessage("", "ItemHandler::installStarted", data);
}

void WebSocketServer::itemInstallFinished(QJsonObject result)
{
    QJsonArray data;
    data.append(result);
    sendMessage("", "ItemHandler::installFinished", data);
}

void WebSocketServer::itemUninstallStarted(QJsonObject result)
{
    QJsonArray data;
    data.append(result);
    sendMessage("", "ItemHandler::uninstallStarted", data);
}

void WebSocketServer::itemUninstallFinished(QJsonObject result)
{
    QJsonArray data;
    data.append(result);
    sendMessage("", "ItemHandler::uninstallFinished", data);
}

void WebSocketServer::receiveMessage(const QString &id, const QString &func, const QJsonArray &data)
{
    /* message object format
    {
        "id": "example",
        "func": "functionName",
        "data": ["value", 2, true]
    }
    */

    QJsonArray resultData;

    // WebSocketServer
    if (func == "WebSocketServer::stop") {
        stop();
    }
    else if (func == "WebSocketServer::isError") {
        resultData.append(isError());
    }
    else if (func == "WebSocketServer::errorString") {
        resultData.append(errorString());
    }
    else if (func == "WebSocketServer::serverUrl") {
        resultData.append(serverUrl().toString());
    }
    // ConfigHandler
    else if (func == "ConfigHandler::getAppConfigApplication") {
        resultData.append(configHandler_->getAppConfigApplication());
    }
    else if (func == "ConfigHandler::getAppConfigInstallTypes") {
        resultData.append(configHandler_->getAppConfigInstallTypes());
    }
    else if (func == "ConfigHandler::getUsrConfigApplication") {
        resultData.append(configHandler_->getUsrConfigApplication());
    }
    else if (func == "ConfigHandler::setUsrConfigApplication") {
        resultData.append(configHandler_->setUsrConfigApplication(data.at(0).toObject()));
    }
    else if (func == "ConfigHandler::getUsrConfigProviders") {
        resultData.append(configHandler_->getUsrConfigProviders());
    }
    else if (func == "ConfigHandler::setUsrConfigProviders") {
        resultData.append(configHandler_->setUsrConfigProviders(data.at(0).toObject()));
    }
    else if (func == "ConfigHandler::getUsrConfigCategories") {
        resultData.append(configHandler_->getUsrConfigCategories());
    }
    else if (func == "ConfigHandler::setUsrConfigCategories") {
        resultData.append(configHandler_->setUsrConfigCategories(data.at(0).toObject()));
    }
    else if (func == "ConfigHandler::getUsrConfigInstalledItems") {
        resultData.append(configHandler_->getUsrConfigInstalledItems());
    }
    else if (func == "ConfigHandler::setUsrConfigInstalledItems") {
        resultData.append(configHandler_->setUsrConfigInstalledItems(data.at(0).toObject()));
    }
    else if (func == "ConfigHandler::setUsrConfigProvidersProvider") {
        resultData.append(configHandler_->setUsrConfigProvidersProvider(data.at(0).toString(), data.at(1).toObject()));
    }
    else if (func == "ConfigHandler::removeUsrConfigProvidersProvider") {
        resultData.append(configHandler_->removeUsrConfigProvidersProvider(data.at(0).toString()));
    }
    else if (func == "ConfigHandler::setUsrConfigCategoriesProvider") {
        resultData.append(configHandler_->setUsrConfigCategoriesProvider(data.at(0).toString(), data.at(1).toObject()));
    }
    else if (func == "ConfigHandler::removeUsrConfigCategoriesProvider") {
        resultData.append(configHandler_->removeUsrConfigCategoriesProvider(data.at(0).toString()));
    }
    else if (func == "ConfigHandler::setUsrConfigCategoriesInstallType") {
        resultData.append(configHandler_->setUsrConfigCategoriesInstallType(data.at(0).toString(), data.at(1).toString(), data.at(2).toString()));
    }
    else if (func == "ConfigHandler::setUsrConfigInstalledItemsItem") {
        resultData.append(configHandler_->setUsrConfigInstalledItemsItem(data.at(0).toString(), data.at(1).toObject()));
    }
    else if (func == "ConfigHandler::removeUsrConfigInstalledItemsItem") {
        resultData.append(configHandler_->removeUsrConfigInstalledItemsItem(data.at(0).toString()));
    }
    // SystemHandler
    else if (func == "SystemHandler::isUnix") {
        resultData.append(systemHandler_->isUnix());
    }
    else if (func == "SystemHandler::isMobileDevice") {
        resultData.append(systemHandler_->isMobileDevice());
    }
    else if (func == "SystemHandler::openUrl") {
        resultData.append(systemHandler_->openUrl(data.at(0).toString()));
    }
    else if (func == "SystemHandler::desktopEnvironment") {
        resultData.append(systemHandler_->desktopEnvironment());
    }
    else if (func == "SystemHandler::isApplicableType") {
        resultData.append(systemHandler_->isApplicableType(data.at(0).toString()));
    }
    else if (func == "SystemHandler::applyFile") {
#ifdef QTLIB_UNIX
        resultData.append(systemHandler_->applyFile(data.at(0).toString(), data.at(1).toString()));
#else
        resultData.append(false);
#endif
    }
    // OcsApiHandler
    else if (func == "OcsApiHandler::addProviders") {
        resultData.append(ocsApiHandler_->addProviders(data.at(0).toString()));
    }
    else if (func == "OcsApiHandler::removeProvider") {
        resultData.append(ocsApiHandler_->removeProvider(data.at(0).toString()));
    }
    else if (func == "OcsApiHandler::updateAllCategories") {
        resultData.append(ocsApiHandler_->updateAllCategories(data.at(0).toBool()));
    }
    else if (func == "OcsApiHandler::updateCategories") {
        resultData.append(ocsApiHandler_->updateCategories(data.at(0).toString(), data.at(1).toBool()));
    }
    else if (func == "OcsApiHandler::getContents") {
        resultData.append(ocsApiHandler_->getContents(data.at(0).toString(), data.at(1).toString(),
                                                      data.at(2).toString(), data.at(3).toString(),
                                                      data.at(4).toString(), data.at(5).toString(), data.at(6).toInt(), data.at(7).toInt()));
    }
    else if (func == "OcsApiHandler::getContent") {
        resultData.append(ocsApiHandler_->getContent(data.at(0).toString(), data.at(1).toString()));
    }
    // ItemHandler
    else if (func == "ItemHandler::metadataSet") {
        resultData.append(itemHandler_->metadataSet());
    }
    else if (func == "ItemHandler::getItem") {
        itemHandler_->getItem(data.at(0).toString(), data.at(1).toString(), data.at(2).toString(), data.at(3).toString(),
                              data.at(4).toString(), data.at(5).toString());
    }
    else if (func == "ItemHandler::getItemByOcsUrl") {
        itemHandler_->getItemByOcsUrl(data.at(0).toString(), data.at(1).toString(), data.at(2).toString());
    }
    else if (func == "ItemHandler::uninstall") {
        itemHandler_->uninstall(data.at(0).toString());
    }
    // Not supported
    else {
        return;
    }

    sendMessage(id, func, resultData);
}

void WebSocketServer::sendMessage(const QString &id, const QString &func, const QJsonArray &data)
{
    /* message object format
    {
        "id": "example",
        "func": "functionName",
        "data": ["value", 2, true]
    }
    */

    QJsonObject object;
    object["id"] = id;
    object["func"] = func;
    object["data"] = data;

    auto binaryMessage = qtlib::Json(object).toJson();
    auto textMessage = QString::fromUtf8(binaryMessage);

    for (auto *wsClient : wsClients_) {
        wsClient->sendTextMessage(textMessage);
        //wsClient->sendBinaryMessage(binaryMessage);
    }
}
