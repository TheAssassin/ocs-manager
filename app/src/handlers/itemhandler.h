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
    explicit ItemHandler(ConfigHandler *configHandler, QObject *parent = 0);

signals:
    void metadataSetChanged();
    void downloadStarted(QJsonObject result);
    void downloadFinished(QJsonObject result);
    void downloadProgress(QString id, qint64 bytesReceived, qint64 bytesTotal);
    void installStarted(QJsonObject result);
    void installFinished(QJsonObject result);
    void uninstallStarted(QJsonObject result);
    void uninstallFinished(QJsonObject result);

public slots:
    QJsonObject metadataSet() const;

    void download(const QString &url, const QString &installType, const QString &providerKey, const QString &contentId);
    void uninstall(const QString &itemKey);

private slots:
    void networkResourceFinished(qtlib::NetworkResource *resource);

private:
    void setMetadataSet(const QJsonObject &metadataSet);

    void install(qtlib::NetworkResource *resource);

    ConfigHandler *configHandler_;
    QJsonObject metadataSet_;
};
