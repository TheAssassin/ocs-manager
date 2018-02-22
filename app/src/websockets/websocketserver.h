#pragma once

#include <QObject>
#include <QUrl>
#include <QJsonObject>
#include <QJsonArray>

class QWebSocketServer;
class QWebSocket;

class ConfigHandler;
class SystemHandler;
class OcsApiHandler;
class ItemHandler;
class UpdateHandler;
class DesktopThemeHandler;

class WebSocketServer : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketServer(ConfigHandler *configHandler, const QString &serverName = QString("WebSocketServer"), quint16 serverPort = 0, QObject *parent = nullptr);
    ~WebSocketServer();

signals:
    void started();
    void stopped();

public slots:
    bool start();
    void stop();
    bool isError() const;
    QString errorString() const;
    QUrl serverUrl() const;

private slots:
    void wsNewConnection();
    void wsDisconnected();
    void wsTextMessageReceived(const QString &message);
    void wsBinaryMessageReceived(const QByteArray &message);

    void itemHandlerMetadataSetChanged();
    void itemHandlerDownloadStarted(QJsonObject result);
    void itemHandlerDownloadFinished(QJsonObject result);
    void itemHandlerDownloadProgress(QString id, qint64 bytesReceived, qint64 bytesTotal);
    void itemHandlerSaveStarted(QJsonObject result);
    void itemHandlerSaveFinished(QJsonObject result);
    void itemHandlerInstallStarted(QJsonObject result);
    void itemHandlerInstallFinished(QJsonObject result);
    void itemHandlerUninstallStarted(QJsonObject result);
    void itemHandlerUninstallFinished(QJsonObject result);

    void updateHandlerCheckAllStarted(bool status);
    void updateHandlerCheckAllFinished(bool status);
    void updateHandlerUpdateStarted(QString itemKey, bool status);
    void updateHandlerUpdateFinished(QString itemKey, bool status);
    void updateHandlerUpdateProgress(QString itemKey, double progress);

private:
    void receiveMessage(const QString &id, const QString &func, const QJsonArray &data);
    void sendMessage(const QString &id, const QString &func, const QJsonArray &data);

    ConfigHandler *configHandler_;
    SystemHandler *systemHandler_;
    OcsApiHandler *ocsApiHandler_;
    ItemHandler *itemHandler_;
    UpdateHandler *updateHandler_;
    DesktopThemeHandler *desktopThemeHandler_;

    QString serverName_;
    quint16 serverPort_;
    QWebSocketServer *wsServer_;
    QList<QWebSocket *> wsClients_;
};
