#pragma once

#include <QObject>
#include <QJsonObject>

namespace qtlib {
class NetworkResource;
}

class ConfigHandler;

class ItemHandler : public QObject
{
    Q_OBJECT

public:
    explicit ItemHandler(ConfigHandler *configHandler, QObject *parent = nullptr);

signals:
    void metadataSetChanged();
    void downloadStarted(QJsonObject result);
    void downloadFinished(QJsonObject result);
    void downloadProgress(QString id, qint64 bytesReceived, qint64 bytesTotal);
    void saveStarted(QJsonObject result);
    void saveFinished(QJsonObject result);
    void installStarted(QJsonObject result);
    void installFinished(QJsonObject result);
    void uninstallStarted(QJsonObject result);
    void uninstallFinished(QJsonObject result);

public slots:
    QJsonObject metadataSet() const;

    void getItem(const QString &command, const QString &url, const QString &installType, const QString &filename = "",
                 const QString &providerKey = "", const QString &contentId = "");
    void getItemByOcsUrl(const QString &ocsUrl);
    void uninstall(const QString &itemKey);

private slots:
    void networkResourceFinished(qtlib::NetworkResource *resource);

private:
    void setMetadataSet(const QJsonObject &metadataSet);

    void saveDownloadedFile(qtlib::NetworkResource *resource);
    void installDownloadedFile(qtlib::NetworkResource *resource);

    ConfigHandler *configHandler_;
    QJsonObject metadataSet_;
};
