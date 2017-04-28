#pragma once

#include <QObject>
#include <QUrl>

class QWebSocketServer;

class WebSocketServer : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketServer(const QString &serverName, quint16 serverPort = 0, QObject *parent = 0);
    ~WebSocketServer();

public slots:
    bool start();
    void stop();

    bool isError();
    QString errorString();

    QUrl serverUrl();

private:
    QString serverName_;
    quint16 serverPort_;
    QWebSocketServer *webSocketServer_;
};
