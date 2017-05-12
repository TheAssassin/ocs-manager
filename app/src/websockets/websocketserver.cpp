#include "websocketserver.h"

#include <QHostAddress>
#include <QWebSocketServer>
#include <QWebSocket>

#include "qtlib_json.h"

#include "handlers/confighandler.h"
#include "handlers/systemhandler.h"
#include "handlers/ocshandler.h"
#include "handlers/itemhandler.h"

WebSocketServer::WebSocketServer(ConfigHandler *configHandler, const QString &serverName, quint16 serverPort, QObject *parent)
    : QObject(parent), configHandler_(configHandler), serverName_(serverName), serverPort_(serverPort)
{
    wsServer_ = new QWebSocketServer(serverName_, QWebSocketServer::NonSecureMode, this);
    connect(wsServer_, &QWebSocketServer::newConnection, this, &WebSocketServer::wsNewConnection);
    connect(wsServer_, &QWebSocketServer::closed, this, &WebSocketServer::stopped);

    configHandler_->setParent(this);
    systemHandler_ = new SystemHandler(this);
    ocsHandler_ = new OcsHandler(configHandler_, this);
    itemHandler_ = new ItemHandler(configHandler_, this);
    connect(itemHandler_, &ItemHandler::metadataSetChanged, this, &WebSocketServer::itemMetadataSetChanged);
    connect(itemHandler_, &ItemHandler::downloadStarted, this, &WebSocketServer::itemDownloadStarted);
    connect(itemHandler_, &ItemHandler::downloadFinished, this, &WebSocketServer::itemDownloadFinished);
    connect(itemHandler_, &ItemHandler::downloadProgress, this, &WebSocketServer::itemDownloadProgress);
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

bool WebSocketServer::isError()
{
    if (wsServer_->error() != QWebSocketProtocol::CloseCodeNormal) {
        return true;
    }
    return false;
}

QString WebSocketServer::errorString()
{
    return wsServer_->errorString();
}

QUrl WebSocketServer::serverUrl()
{
    return wsServer_->serverUrl();
}

void WebSocketServer::wsNewConnection()
{
    QWebSocket *wsClient = wsServer_->nextPendingConnection();
    connect(wsClient, &QWebSocket::disconnected, this, &WebSocketServer::wsDisconnected);
    connect(wsClient, &QWebSocket::textMessageReceived, this, &WebSocketServer::wsTextMessageReceived);
    connect(wsClient, &QWebSocket::binaryMessageReceived, this, &WebSocketServer::wsBinaryMessageReceived);
    wsClients_ << wsClient;
}

void WebSocketServer::wsDisconnected()
{
    QWebSocket *wsClient = qobject_cast<QWebSocket *>(sender());
    if (wsClient) {
        wsClients_.removeAll(wsClient);
        wsClient->deleteLater();
    }
}

void WebSocketServer::wsTextMessageReceived(const QString &message)
{
    QWebSocket *wsClient = qobject_cast<QWebSocket *>(sender());
    if (wsClient) {
        qtlib::Json json(message.toUtf8());
        if (json.isObject()) {
            QJsonObject object = json.toObject();
            receiveMessage(object["id"].toString(), object["func"].toString(), object["data"].toArray());
        }
    }
}

void WebSocketServer::wsBinaryMessageReceived(const QByteArray &message)
{
    QWebSocket *wsClient = qobject_cast<QWebSocket *>(sender());
    if (wsClient) {
        qtlib::Json json(message);
        if (json.isObject()) {
            QJsonObject object = json.toObject();
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
    data[0] = result;
    sendMessage("", "ItemHandler::downloadStarted", data);
}

void WebSocketServer::itemDownloadFinished(QJsonObject result)
{
    QJsonArray data;
    data[0] = result;
    sendMessage("", "ItemHandler::downloadFinished", data);
}

void WebSocketServer::itemDownloadProgress(QString id, qint64 bytesReceived, qint64 bytesTotal)
{
    QJsonArray data;
    data[0] = id;
    data[1] = bytesReceived;
    data[2] = bytesTotal;
    sendMessage("", "ItemHandler::downloadProgress", data);
}

void WebSocketServer::itemInstallStarted(QJsonObject result)
{
    QJsonArray data;
    data[0] = result;
    sendMessage("", "ItemHandler::installStarted", data);
}

void WebSocketServer::itemInstallFinished(QJsonObject result)
{
    QJsonArray data;
    data[0] = result;
    sendMessage("", "ItemHandler::installFinished", data);
}

void WebSocketServer::itemUninstallStarted(QJsonObject result)
{
    QJsonArray data;
    data[0] = result;
    sendMessage("", "ItemHandler::uninstallStarted", data);
}

void WebSocketServer::itemUninstallFinished(QJsonObject result)
{
    QJsonArray data;
    data[0] = result;
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
        resultData[0] = isError();
    }
    else if (func == "WebSocketServer::errorString") {
        resultData[0] = errorString();
    }
    else if (func == "WebSocketServer::serverUrl") {
        resultData[0] = serverUrl().toString();
    }
    // ConfigHandler
    else if (func == "ConfigHandler::getAppConfigApplication") {
        resultData[0] = configHandler_->getAppConfigApplication();
    }
    else if (func == "ConfigHandler::getAppConfigInstallTypes") {
        resultData[0] = configHandler_->getAppConfigInstallTypes();
    }
    else if (func == "ConfigHandler::getUsrConfigApplication") {
        resultData[0] = configHandler_->getUsrConfigApplication();
    }
    else if (func == "ConfigHandler::setUsrConfigApplication") {
        resultData[0] = configHandler_->setUsrConfigApplication(data[0].toObject());
    }
    else if (func == "ConfigHandler::getUsrConfigProviders") {
        resultData[0] = configHandler_->getUsrConfigProviders();
    }
    else if (func == "ConfigHandler::setUsrConfigProviders") {
        resultData[0] = configHandler_->setUsrConfigProviders(data[0].toObject());
    }
    else if (func == "ConfigHandler::getUsrConfigCategories") {
        resultData[0] = configHandler_->getUsrConfigCategories();
    }
    else if (func == "ConfigHandler::setUsrConfigCategories") {
        resultData[0] = configHandler_->setUsrConfigCategories(data[0].toObject());
    }
    else if (func == "ConfigHandler::getUsrConfigInstalledItems") {
        resultData[0] = configHandler_->getUsrConfigInstalledItems();
    }
    else if (func == "ConfigHandler::setUsrConfigInstalledItems") {
        resultData[0] = configHandler_->setUsrConfigInstalledItems(data[0].toObject());
    }
    else if (func == "ConfigHandler::setUsrConfigProvidersProvider") {
        resultData[0] = configHandler_->setUsrConfigProvidersProvider(data[0].toString(), data[1].toObject());
    }
    else if (func == "ConfigHandler::removeUsrConfigProvidersProvider") {
        resultData[0] = configHandler_->removeUsrConfigProvidersProvider(data[0].toString());
    }
    else if (func == "ConfigHandler::setUsrConfigCategoriesProvider") {
        resultData[0] = configHandler_->setUsrConfigCategoriesProvider(data[0].toString(), data[1].toObject());
    }
    else if (func == "ConfigHandler::removeUsrConfigCategoriesProvider") {
        resultData[0] = configHandler_->removeUsrConfigCategoriesProvider(data[0].toString());
    }
    else if (func == "ConfigHandler::setUsrConfigCategoriesInstallType") {
        resultData[0] = configHandler_->setUsrConfigCategoriesInstallType(data[0].toString(), data[1].toString(), data[2].toString());
    }
    else if (func == "ConfigHandler::setUsrConfigInstalledItemsItem") {
        resultData[0] = configHandler_->setUsrConfigInstalledItemsItem(data[0].toString(), data[1].toObject());
    }
    else if (func == "ConfigHandler::removeUsrConfigInstalledItemsItem") {
        resultData[0] = configHandler_->removeUsrConfigInstalledItemsItem(data[0].toString());
    }
    // SystemHandler
    else if (func == "SystemHandler::isUnix") {
        resultData[0] = systemHandler_->isUnix();
    }
    else if (func == "SystemHandler::desktopEnvironment") {
        resultData[0] = systemHandler_->desktopEnvironment();
    }
    else if (func == "SystemHandler::isApplicableType") {
        resultData[0] = systemHandler_->isApplicableType(data[0].toString());
    }
    else if (func == "SystemHandler::applyFile") {
        resultData[0] = false;
#ifdef QTLIB_UNIX
        resultData[0] = systemHandler_->applyFile(data[0].toString(), data[1].toString());
#endif
    }
    // OcsHandler
    else if (func == "OcsHandler::addProviders") {
        resultData[0] = ocsHandler_->addProviders(data[0].toString());
    }
    else if (func == "OcsHandler::removeProvider") {
        resultData[0] = ocsHandler_->removeProvider(data[0].toString());
    }
    else if (func == "OcsHandler::updateAllCategories") {
        resultData[0] = ocsHandler_->updateAllCategories(data[0].toBool());
    }
    else if (func == "OcsHandler::updateCategories") {
        resultData[0] = ocsHandler_->updateCategories(data[0].toString(), data[1].toBool());
    }
    else if (func == "OcsHandler::getContents") {
        resultData[0] = ocsHandler_->getContents(data[0].toString(), data[1].toString(),
                data[2].toString(), data[3].toString(),
                data[4].toString(), data[5].toString(), data[6].toInt(), data[7].toInt());
    }
    else if (func == "OcsHandler::getContent") {
        resultData[0] = ocsHandler_->getContent(data[0].toString(), data[1].toString());
    }
    // ItemHandler
    else if (func == "ItemHandler::metadataSet") {
        resultData[0] = itemHandler_->metadataSet();
    }
    else if (func == "ItemHandler::download") {
        itemHandler_->download(data[0].toString(), data[1].toString(), data[2].toString(), data[3].toString());
    }
    else if (func == "ItemHandler::uninstall") {
        itemHandler_->uninstall(data[0].toString());
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

    QByteArray binaryMessage = qtlib::Json(object).toJson();
    QString textMessage = QString::fromUtf8(binaryMessage);

    foreach (QWebSocket *wsClient, wsClients_) {
        wsClient->sendTextMessage(textMessage);
        //wsClient->sendBinaryMessage(binaryMessage);
    }
}
