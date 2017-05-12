#pragma once

#include <QObject>
#include <QUrl>
//#include <QJsonObject>

class QWebSocketServer;
class QWebSocket;

class ConfigHandler;
class SystemHandler;
class OcsHandler;
class ItemHandler;

class WebSocketServer : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketServer(ConfigHandler *configHandler, const QString &serverName = "WebSocketServer", quint16 serverPort = 0, QObject *parent = 0);
    ~WebSocketServer();

signals:
    void started();
    void stopped();

public slots:
    bool start();
    void stop();
    bool isError();
    QString errorString();
    QUrl serverUrl();

private slots:
    void wsNewConnection();
    void wsDisconnected();
    void wsTextMessageReceived(const QString &message);
    void wsBinaryMessageReceived(const QByteArray &message);

private:
    void callFunction(const QJsonObject &request, QWebSocket *wsClient);

    ConfigHandler *configHandler_;
    SystemHandler *systemHandler_;
    OcsHandler *ocsHandler_;
    ItemHandler *itemHandler_;

    QString serverName_;
    quint16 serverPort_;
    QWebSocketServer *wsServer_;
    QList<QWebSocket *> wsClients_;
};
