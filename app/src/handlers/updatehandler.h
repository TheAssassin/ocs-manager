#pragma once

#include <QObject>
#include <QJsonObject>

class ConfigHandler;

#ifdef QTIL_OS_UNIX
class AppImageUpdater;
#endif

class UpdateHandler : public QObject
{
    Q_OBJECT

public:
    explicit UpdateHandler(ConfigHandler *configHandler, QObject *parent = nullptr);

signals:
    void checkAllStarted(bool status);
    void checkAllFinished(bool status);
    void updateStarted(QString itemKey, bool status);
    void updateFinished(QString itemKey, bool status);
    void updateProgress(QString itemKey, double progress);

public slots:
    void checkAll();
    void update(const QString &itemKey);

private slots:
#ifdef QTIL_OS_UNIX
    void appImageUpdaterFinished(AppImageUpdater *updater);
#endif

private:
#ifdef QTIL_OS_UNIX
    void updateAppImage(const QString &itemKey);
#endif

    ConfigHandler *configHandler_;
    QJsonObject metadataSet_;
};
