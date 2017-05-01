#pragma once

#include <QObject>
#include <QUrl>

class QWebSocketServer;
class QWebSocket;

class WebSocketServer : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketServer(const QString &serverName, quint16 serverPort = 0, QObject *parent = 0);
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
    void wsBinaryMessageReceived(const QByteArray &message);
    void wsTextMessageReceived(const QString &message);
    void wsDisconnected();

private:
    QString serverName_;
    quint16 serverPort_;
    QWebSocketServer *wsServer_;
    QList<QWebSocket *> wsClients_;
};
