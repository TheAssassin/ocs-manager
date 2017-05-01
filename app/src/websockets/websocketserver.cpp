#include "websocketserver.h"

#include <QHostAddress>
#include <QWebSocketServer>

WebSocketServer::WebSocketServer(const QString &serverName, quint16 serverPort, QObject *parent)
    : QObject(parent), serverName_(serverName), serverPort_(serverPort)
{
    webSocketServer_ = new QWebSocketServer(serverName_, QWebSocketServer::NonSecureMode, this);
    connect(webSocketServer_, &QWebSocketServer::closed, this, &WebSocketServer::stopped);
}

WebSocketServer::~WebSocketServer()
{
    stop();
    webSocketServer_->deleteLater();
}

bool WebSocketServer::start()
{
    if (webSocketServer_->listen(QHostAddress::Any, serverPort_)) {
        emit started();
        return true;
    }
    return false;
}

void WebSocketServer::stop()
{
    webSocketServer_->close();
}

bool WebSocketServer::isError()
{
    if (webSocketServer_->error() != QWebSocketProtocol::CloseCodeNormal) {
        return true;
    }
    return false;
}

QString WebSocketServer::errorString()
{
    return webSocketServer_->errorString();
}

QUrl WebSocketServer::serverUrl()
{
    return webSocketServer_->serverUrl();
}
