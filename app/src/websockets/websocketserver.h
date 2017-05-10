#pragma once

#include <QObject>
#include <QUrl>
//#include <QJsonObject>

class QWebSocketServer;
class QWebSocket;

class WebSocketServer : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketServer(const QString &serverName, quint16 serverPort = 0, QObject *parent = 0);
    ~WebSocketServer();

    bool start();
    void stop();
    bool isError();
    QString errorString();
    QUrl serverUrl();

signals:
    void started();
    void stopped();

private slots:
    void wsNewConnection();
    void wsDisconnected();
    void wsTextMessageReceived(const QString &message);
    void wsBinaryMessageReceived(const QByteArray &message);

private:
    void callFunction(const QJsonObject &request, QWebSocket *wsClient);

    QString serverName_;
    quint16 serverPort_;
    QWebSocketServer *wsServer_;
    QList<QWebSocket *> wsClients_;
};
