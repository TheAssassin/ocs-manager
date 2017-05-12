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
            callFunction(json.toObject(), wsClient);
        }
    }
}

void WebSocketServer::wsBinaryMessageReceived(const QByteArray &message)
{
    QWebSocket *wsClient = qobject_cast<QWebSocket *>(sender());
    if (wsClient) {
        qtlib::Json json(message);
        if (json.isValid()) {
            callFunction(json.toObject(), wsClient);
        }
    }
}

void WebSocketServer::callFunction(const QJsonObject &request, QWebSocket *wsClient)
{
    /* request object format
    {
        "id": "example",
        "call": "functionName",
        "arg": ["value", 2, true]
    }
    */

    /* response object format
    {
        "id": "example",
        "result": {}
    }
    */

    QString id = request["id"].toString();
    QString call = request["call"].toString();
    QJsonArray arg = request["arg"].toArray();

    QJsonObject response;
    response["id"] = id;

    // WebSocketServer
    if (call == "WebSocketServer::stop") {
        stop();
    }
    else if (call == "WebSocketServer::isError") {
        response["result"] = isError();
    }
    else if (call == "WebSocketServer::errorString") {
        response["result"] = errorString();
    }
    else if (call == "WebSocketServer::serverUrl") {
        response["result"] = serverUrl().toString();
    }
    // ConfigHandler
    else if (call == "ConfigHandler::getAppConfigApplication") {
        response["result"] = configHandler_->getAppConfigApplication();
    }
    else if (call == "ConfigHandler::getAppConfigInstallTypes") {
        response["result"] = configHandler_->getAppConfigInstallTypes();
    }
    else if (call == "ConfigHandler::getUsrConfigApplication") {
        response["result"] = configHandler_->getUsrConfigApplication();
    }
    else if (call == "ConfigHandler::setUsrConfigApplication") {
        response["result"] = configHandler_->setUsrConfigApplication(arg[0].toObject());
    }
    else if (call == "ConfigHandler::getUsrConfigProviders") {
        response["result"] = configHandler_->getUsrConfigProviders();
    }
    else if (call == "ConfigHandler::setUsrConfigProviders") {
        response["result"] = configHandler_->setUsrConfigProviders(arg[0].toObject());
    }
    else if (call == "ConfigHandler::getUsrConfigCategories") {
        response["result"] = configHandler_->getUsrConfigCategories();
    }
    else if (call == "ConfigHandler::setUsrConfigCategories") {
        response["result"] = configHandler_->setUsrConfigCategories(arg[0].toObject());
    }
    else if (call == "ConfigHandler::getUsrConfigInstalledItems") {
        response["result"] = configHandler_->getUsrConfigInstalledItems();
    }
    else if (call == "ConfigHandler::setUsrConfigInstalledItems") {
        response["result"] = configHandler_->setUsrConfigInstalledItems(arg[0].toObject());
    }
    else if (call == "ConfigHandler::setUsrConfigProvidersProvider") {
        response["result"] = configHandler_->setUsrConfigProvidersProvider(arg[0].toString(), arg[1].toObject());
    }
    else if (call == "ConfigHandler::removeUsrConfigProvidersProvider") {
        response["result"] = configHandler_->removeUsrConfigProvidersProvider(arg[0].toString());
    }
    else if (call == "ConfigHandler::setUsrConfigCategoriesProvider") {
        response["result"] = configHandler_->setUsrConfigCategoriesProvider(arg[0].toString(), arg[1].toObject());
    }
    else if (call == "ConfigHandler::removeUsrConfigCategoriesProvider") {
        response["result"] = configHandler_->removeUsrConfigCategoriesProvider(arg[0].toString());
    }
    else if (call == "ConfigHandler::setUsrConfigCategoriesInstallType") {
        response["result"] = configHandler_->setUsrConfigCategoriesInstallType(arg[0].toString(), arg[1].toString(), arg[2].toString());
    }
    else if (call == "ConfigHandler::setUsrConfigInstalledItemsItem") {
        response["result"] = configHandler_->setUsrConfigInstalledItemsItem(arg[0].toString(), arg[1].toObject());
    }
    else if (call == "ConfigHandler::removeUsrConfigInstalledItemsItem") {
        response["result"] = configHandler_->removeUsrConfigInstalledItemsItem(arg[0].toString());
    }

    wsClient->sendTextMessage(QString(qtlib::Json(response).toJson()));
    //wsClient->sendBinaryMessage(qtlib::Json(response).toJson());
}
