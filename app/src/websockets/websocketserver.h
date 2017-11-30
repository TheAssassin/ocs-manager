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

    void itemMetadataSetChanged();
    void itemDownloadStarted(QJsonObject result);
    void itemDownloadFinished(QJsonObject result);
    void itemDownloadProgress(QString id, qint64 bytesReceived, qint64 bytesTotal);
    void itemSaveStarted(QJsonObject result);
    void itemSaveFinished(QJsonObject result);
    void itemInstallStarted(QJsonObject result);
    void itemInstallFinished(QJsonObject result);
    void itemUninstallStarted(QJsonObject result);
    void itemUninstallFinished(QJsonObject result);

    void updateCheckAllStarted();
    void updateCheckAllFinished();
    void updateUpdateStarted(QString itemKey);
    void updateUpdateFinished(QString itemKey);
    void updateUpdateProgress(QString itemKey, int progress);

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
