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

    QJsonObject response;
    response["id"] = id;
    response["func"] = func;

    // WebSocketServer
    if (func == "WebSocketServer::stop") {
        stop();
        return;
    }
    else if (func == "WebSocketServer::isError") {
        response["result"] = isError();
    }
    else if (func == "WebSocketServer::errorString") {
        response["result"] = errorString();
    }
    else if (func == "WebSocketServer::serverUrl") {
        response["result"] = serverUrl().toString();
    }
    // ConfigHandler
    else if (func == "ConfigHandler::getAppConfigApplication") {
        response["result"] = configHandler_->getAppConfigApplication();
    }
    else if (func == "ConfigHandler::getAppConfigInstallTypes") {
        response["result"] = configHandler_->getAppConfigInstallTypes();
    }
    else if (func == "ConfigHandler::getUsrConfigApplication") {
        response["result"] = configHandler_->getUsrConfigApplication();
    }
    else if (func == "ConfigHandler::setUsrConfigApplication") {
        response["result"] = configHandler_->setUsrConfigApplication(arg[0].toObject());
    }
    else if (func == "ConfigHandler::getUsrConfigProviders") {
        response["result"] = configHandler_->getUsrConfigProviders();
    }
    else if (func == "ConfigHandler::setUsrConfigProviders") {
        response["result"] = configHandler_->setUsrConfigProviders(arg[0].toObject());
    }
    else if (func == "ConfigHandler::getUsrConfigCategories") {
        response["result"] = configHandler_->getUsrConfigCategories();
    }
    else if (func == "ConfigHandler::setUsrConfigCategories") {
        response["result"] = configHandler_->setUsrConfigCategories(arg[0].toObject());
    }
    else if (func == "ConfigHandler::getUsrConfigInstalledItems") {
        response["result"] = configHandler_->getUsrConfigInstalledItems();
    }
    else if (func == "ConfigHandler::setUsrConfigInstalledItems") {
        response["result"] = configHandler_->setUsrConfigInstalledItems(arg[0].toObject());
    }
    else if (func == "ConfigHandler::setUsrConfigProvidersProvider") {
        response["result"] = configHandler_->setUsrConfigProvidersProvider(arg[0].toString(), arg[1].toObject());
    }
    else if (func == "ConfigHandler::removeUsrConfigProvidersProvider") {
        response["result"] = configHandler_->removeUsrConfigProvidersProvider(arg[0].toString());
    }
    else if (func == "ConfigHandler::setUsrConfigCategoriesProvider") {
        response["result"] = configHandler_->setUsrConfigCategoriesProvider(arg[0].toString(), arg[1].toObject());
    }
    else if (func == "ConfigHandler::removeUsrConfigCategoriesProvider") {
        response["result"] = configHandler_->removeUsrConfigCategoriesProvider(arg[0].toString());
    }
    else if (func == "ConfigHandler::setUsrConfigCategoriesInstallType") {
        response["result"] = configHandler_->setUsrConfigCategoriesInstallType(arg[0].toString(), arg[1].toString(), arg[2].toString());
    }
    else if (func == "ConfigHandler::setUsrConfigInstalledItemsItem") {
        response["result"] = configHandler_->setUsrConfigInstalledItemsItem(arg[0].toString(), arg[1].toObject());
    }
    else if (func == "ConfigHandler::removeUsrConfigInstalledItemsItem") {
        response["result"] = configHandler_->removeUsrConfigInstalledItemsItem(arg[0].toString());
    }
    // SystemHandler
    else if (func == "SystemHandler::isUnix") {
        response["result"] = systemHandler_->isUnix();
    }
    else if (func == "SystemHandler::desktopEnvironment") {
        response["result"] = systemHandler_->desktopEnvironment();
    }
    else if (func == "SystemHandler::isApplicableType") {
        response["result"] = systemHandler_->isApplicableType(arg[0].toString());
    }
    else if (func == "SystemHandler::applyFile") {
        response["result"] = false;
#ifdef QTLIB_UNIX
        response["result"] = systemHandler_->applyFile(arg[0].toString(), arg[1].toString());
#endif
    }
    // OcsHandler
    else if (func == "OcsHandler::addProviders") {
        response["result"] = ocsHandler_->addProviders(arg[0].toString());
    }
    else if (func == "OcsHandler::removeProvider") {
        response["result"] = ocsHandler_->removeProvider(arg[0].toString());
    }
    else if (func == "OcsHandler::updateAllCategories") {
        response["result"] = ocsHandler_->updateAllCategories(arg[0].toBool());
    }
    else if (func == "OcsHandler::updateCategories") {
        response["result"] = ocsHandler_->updateCategories(arg[0].toString(), arg[1].toBool());
    }
    else if (func == "OcsHandler::getContents") {
        response["result"] = ocsHandler_->getContents(arg[0].toString(), arg[1].toString(),
                arg[2].toString(), arg[3].toString(),
                arg[4].toString(), arg[5].toString(), arg[6].toInt(), arg[7].toInt());
    }
    else if (func == "OcsHandler::getContent") {
        response["result"] = ocsHandler_->getContent(arg[0].toString(), arg[1].toString());
    }
    // ItemHandler
    else if (func == "ItemHandler::metadataSet") {
        response["result"] = itemHandler_->metadataSet();
    }
    else if (func == "ItemHandler::download") {
        itemHandler_->download(arg[0].toString(), arg[1].toString(), arg[2].toString(), arg[3].toString());
        return;
    }
    else if (func == "ItemHandler::uninstall") {
        itemHandler_->uninstall(arg[0].toString());
        return;
    }
    // Default
    else {
        return;
    }

    sendResponse(response);
}

void WebSocketServer::sendResponse(const QJsonObject &response)
{
    /* response object format
    {
        "id": "example",
        "func": "functionName",
        "result": mixed
    }
    */

    foreach (QWebSocket *wsClient, wsClients_) {
        wsClient->sendTextMessage(QString(qtlib::Json(response).toJson()));
        //wsClient->sendBinaryMessage(qtlib::Json(response).toJson());
    }
}
