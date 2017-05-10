#include "websocketserver.h"

#include <QHostAddress>
#include <QWebSocketServer>
#include <QWebSocket>

#include "qtlib_json.h"

WebSocketServer::WebSocketServer(const QString &serverName, quint16 serverPort, QObject *parent)
    : QObject(parent), serverName_(serverName), serverPort_(serverPort)
{
    wsServer_ = new QWebSocketServer(serverName_, QWebSocketServer::NonSecureMode, this);
    connect(wsServer_, &QWebSocketServer::newConnection, this, &WebSocketServer::wsNewConnection);
    connect(wsServer_, &QWebSocketServer::closed, this, &WebSocketServer::stopped);
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
        "args": {
            "arg1": "value",
            "arg2": 2,
            "arg3": true
        }
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
    QJsonObject args = request["args"].toObject();

    QJsonObject response;
    response["id"] = id;

    if (call == "WebSocketServer::stop") {
        stop();
    }
    else if (call == "WebSocketServer::serverUrl") {
        response["result"] = serverUrl().toString();
    }

    wsClient->sendTextMessage(QString(qtlib::Json(response).toJson()));
    //wsClient->sendBinaryMessage(qtlib::Json(response).toJson());
}
