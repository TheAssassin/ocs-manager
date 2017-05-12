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
        if (json.isValid()) {
            execRequest(json.toObject());
        }
    }
}

void WebSocketServer::wsBinaryMessageReceived(const QByteArray &message)
{
    QWebSocket *wsClient = qobject_cast<QWebSocket *>(sender());
    if (wsClient) {
        qtlib::Json json(message);
        if (json.isValid()) {
            execRequest(json.toObject());
        }
    }
}

void WebSocketServer::itemMetadataSetChanged()
{
    QJsonArray results;

    QJsonObject response;
    response["id"] = QString("");
    response["func"] = QString("ItemHandler::metadataSetChanged");
    response["result"] = results;

    sendResponse(response);
}

void WebSocketServer::itemDownloadStarted(QJsonObject result)
{
    QJsonArray results;
    results[0] = result;

    QJsonObject response;
    response["id"] = QString("");
    response["func"] = QString("ItemHandler::downloadStarted");
    response["result"] = results;

    sendResponse(response);
}

void WebSocketServer::itemDownloadFinished(QJsonObject result)
{
    QJsonArray results;
    results[0] = result;

    QJsonObject response;
    response["id"] = QString("");
    response["func"] = QString("ItemHandler::downloadFinished");
    response["result"] = results;

    sendResponse(response);
}

void WebSocketServer::itemDownloadProgress(QString id, qint64 bytesReceived, qint64 bytesTotal)
{
    QJsonArray results;
    results[0] = id;
    results[1] = bytesReceived;
    results[2] = bytesTotal;

    QJsonObject response;
    response["id"] = QString("");
    response["func"] = QString("ItemHandler::downloadProgress");
    response["result"] = results;

    sendResponse(response);
}

void WebSocketServer::itemInstallStarted(QJsonObject result)
{
    QJsonArray results;
    results[0] = result;

    QJsonObject response;
    response["id"] = QString("");
    response["func"] = QString("ItemHandler::installStarted");
    response["result"] = results;

    sendResponse(response);
}

void WebSocketServer::itemInstallFinished(QJsonObject result)
{
    QJsonArray results;
    results[0] = result;

    QJsonObject response;
    response["id"] = QString("");
    response["func"] = QString("ItemHandler::installFinished");
    response["result"] = results;

    sendResponse(response);
}

void WebSocketServer::itemUninstallStarted(QJsonObject result)
{
    QJsonArray results;
    results[0] = result;

    QJsonObject response;
    response["id"] = QString("");
    response["func"] = QString("ItemHandler::uninstallStarted");
    response["result"] = results;

    sendResponse(response);
}

void WebSocketServer::itemUninstallFinished(QJsonObject result)
{
    QJsonArray results;
    results[0] = result;

    QJsonObject response;
    response["id"] = QString("");
    response["func"] = QString("ItemHandler::uninstallFinished");
    response["result"] = results;

    sendResponse(response);
}

void WebSocketServer::execRequest(const QJsonObject &request)
{
    /* request object format
    {
        "id": "example",
        "func": "functionName",
        "arg": ["value", 2, true]
    }
    */

    QString id = request["id"].toString();
    QString func = request["func"].toString();
    QJsonArray arg = request["arg"].toArray();

    QJsonArray results;

    // WebSocketServer
    if (func == "WebSocketServer::stop") {
        stop();
    }
    else if (func == "WebSocketServer::isError") {
        results[0] = isError();
    }
    else if (func == "WebSocketServer::errorString") {
        results[0] = errorString();
    }
    else if (func == "WebSocketServer::serverUrl") {
        results[0] = serverUrl().toString();
    }
    // ConfigHandler
    else if (func == "ConfigHandler::getAppConfigApplication") {
        results[0] = configHandler_->getAppConfigApplication();
    }
    else if (func == "ConfigHandler::getAppConfigInstallTypes") {
        results[0] = configHandler_->getAppConfigInstallTypes();
    }
    else if (func == "ConfigHandler::getUsrConfigApplication") {
        results[0] = configHandler_->getUsrConfigApplication();
    }
    else if (func == "ConfigHandler::setUsrConfigApplication") {
        results[0] = configHandler_->setUsrConfigApplication(arg[0].toObject());
    }
    else if (func == "ConfigHandler::getUsrConfigProviders") {
        results[0] = configHandler_->getUsrConfigProviders();
    }
    else if (func == "ConfigHandler::setUsrConfigProviders") {
        results[0] = configHandler_->setUsrConfigProviders(arg[0].toObject());
    }
    else if (func == "ConfigHandler::getUsrConfigCategories") {
        results[0] = configHandler_->getUsrConfigCategories();
    }
    else if (func == "ConfigHandler::setUsrConfigCategories") {
        results[0] = configHandler_->setUsrConfigCategories(arg[0].toObject());
    }
    else if (func == "ConfigHandler::getUsrConfigInstalledItems") {
        results[0] = configHandler_->getUsrConfigInstalledItems();
    }
    else if (func == "ConfigHandler::setUsrConfigInstalledItems") {
        results[0] = configHandler_->setUsrConfigInstalledItems(arg[0].toObject());
    }
    else if (func == "ConfigHandler::setUsrConfigProvidersProvider") {
        results[0] = configHandler_->setUsrConfigProvidersProvider(arg[0].toString(), arg[1].toObject());
    }
    else if (func == "ConfigHandler::removeUsrConfigProvidersProvider") {
        results[0] = configHandler_->removeUsrConfigProvidersProvider(arg[0].toString());
    }
    else if (func == "ConfigHandler::setUsrConfigCategoriesProvider") {
        results[0] = configHandler_->setUsrConfigCategoriesProvider(arg[0].toString(), arg[1].toObject());
    }
    else if (func == "ConfigHandler::removeUsrConfigCategoriesProvider") {
        results[0] = configHandler_->removeUsrConfigCategoriesProvider(arg[0].toString());
    }
    else if (func == "ConfigHandler::setUsrConfigCategoriesInstallType") {
        results[0] = configHandler_->setUsrConfigCategoriesInstallType(arg[0].toString(), arg[1].toString(), arg[2].toString());
    }
    else if (func == "ConfigHandler::setUsrConfigInstalledItemsItem") {
        results[0] = configHandler_->setUsrConfigInstalledItemsItem(arg[0].toString(), arg[1].toObject());
    }
    else if (func == "ConfigHandler::removeUsrConfigInstalledItemsItem") {
        results[0] = configHandler_->removeUsrConfigInstalledItemsItem(arg[0].toString());
    }
    // SystemHandler
    else if (func == "SystemHandler::isUnix") {
        results[0] = systemHandler_->isUnix();
    }
    else if (func == "SystemHandler::desktopEnvironment") {
        results[0] = systemHandler_->desktopEnvironment();
    }
    else if (func == "SystemHandler::isApplicableType") {
        results[0] = systemHandler_->isApplicableType(arg[0].toString());
    }
    else if (func == "SystemHandler::applyFile") {
        results[0] = false;
#ifdef QTLIB_UNIX
        results[0] = systemHandler_->applyFile(arg[0].toString(), arg[1].toString());
#endif
    }
    // OcsHandler
    else if (func == "OcsHandler::addProviders") {
        results[0] = ocsHandler_->addProviders(arg[0].toString());
    }
    else if (func == "OcsHandler::removeProvider") {
        results[0] = ocsHandler_->removeProvider(arg[0].toString());
    }
    else if (func == "OcsHandler::updateAllCategories") {
        results[0] = ocsHandler_->updateAllCategories(arg[0].toBool());
    }
    else if (func == "OcsHandler::updateCategories") {
        results[0] = ocsHandler_->updateCategories(arg[0].toString(), arg[1].toBool());
    }
    else if (func == "OcsHandler::getContents") {
        results[0] = ocsHandler_->getContents(arg[0].toString(), arg[1].toString(),
                arg[2].toString(), arg[3].toString(),
                arg[4].toString(), arg[5].toString(), arg[6].toInt(), arg[7].toInt());
    }
    else if (func == "OcsHandler::getContent") {
        results[0] = ocsHandler_->getContent(arg[0].toString(), arg[1].toString());
    }
    // ItemHandler
    else if (func == "ItemHandler::metadataSet") {
        results[0] = itemHandler_->metadataSet();
    }
    else if (func == "ItemHandler::download") {
        itemHandler_->download(arg[0].toString(), arg[1].toString(), arg[2].toString(), arg[3].toString());
    }
    else if (func == "ItemHandler::uninstall") {
        itemHandler_->uninstall(arg[0].toString());
    }
    // Default
    else {
        return;
    }

    QJsonObject response;
    response["id"] = id;
    response["func"] = func;
    response["result"] = results;

    sendResponse(response);
}

void WebSocketServer::sendResponse(const QJsonObject &response)
{
    /* response object format
    {
        "id": "example",
        "func": "functionName",
        "result": ["value", 2, true]
    }
    */

    foreach (QWebSocket *wsClient, wsClients_) {
        wsClient->sendTextMessage(QString(qtlib::Json(response).toJson()));
        //wsClient->sendBinaryMessage(qtlib::Json(response).toJson());
    }
}
