#include "websocketserver.h"

#include <QHostAddress>
#include <QWebSocketServer>
#include <QWebSocket>

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
    connect(wsClient, &QWebSocket::binaryMessageReceived, this, &WebSocketServer::wsBinaryMessageReceived);
    connect(wsClient, &QWebSocket::textMessageReceived, this, &WebSocketServer::wsTextMessageReceived);
    connect(wsClient, &QWebSocket::disconnected, this, &WebSocketServer::wsDisconnected);
    wsClients_ << wsClient;
}

void WebSocketServer::wsBinaryMessageReceived(const QByteArray &message)
{
    QWebSocket *wsClient = qobject_cast<QWebSocket *>(sender());
    if (wsClient) {
        wsClient->sendBinaryMessage(message);
    }
}

void WebSocketServer::wsTextMessageReceived(const QString &message)
{
    QWebSocket *wsClient = qobject_cast<QWebSocket *>(sender());
    if (wsClient) {
        wsClient->sendTextMessage(message);
    }
}

void WebSocketServer::wsDisconnected()
{
    QWebSocket *wsClient = qobject_cast<QWebSocket *>(sender());
    if (wsClient) {
        wsClients_.removeAll(wsClient);
        wsClient->deleteLater();
    }
}
