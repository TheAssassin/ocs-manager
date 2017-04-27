#include "websocketserver.h"

#include <QHostAddress>
#include <QWebSocketServer>

WebSocketServer::WebSocketServer(const QString &serverName, quint16 serverPort, QObject *parent)
    : QObject(parent), serverName_(serverName), serverPort_(serverPort)
{
    webSocketServer_ = new QWebSocketServer(serverName_, QWebSocketServer::NonSecureMode, this);
}

WebSocketServer::~WebSocketServer()
{
    webSocketServer_->deleteLater();
}

bool WebSocketServer::start()
{
    return webSocketServer_->listen(QHostAddress::Any, serverPort_);
}

void WebSocketServer::stop()
{
    webSocketServer_->close();
}
