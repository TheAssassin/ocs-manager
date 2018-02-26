#pragma once

#include <QObject>
#include <QJsonObject>

namespace Qtil {
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

    void getItem(const QString &command, const QString &url, const QString &installType, const QString &filename = QString(),
                 const QString &providerKey = QString(), const QString &contentId = QString());
    void getItemByOcsUrl(const QString &ocsUrl, const QString &providerKey = QString(), const QString &contentId = QString());
    void uninstall(const QString &itemKey);

private slots:
    void networkResourceFinished(Qtil::NetworkResource *resource);

private:
    void setMetadataSet(const QJsonObject &metadataSet);

    void saveDownloadedFile(Qtil::NetworkResource *resource);
    void installDownloadedFile(Qtil::NetworkResource *resource);

    ConfigHandler *configHandler_;
    QJsonObject metadataSet_;
};
