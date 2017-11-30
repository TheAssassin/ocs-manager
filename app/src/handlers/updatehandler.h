#pragma once

#include <QObject>

class ConfigHandler;

class UpdateHandler : public QObject
{
    Q_OBJECT

public:
    explicit UpdateHandler(ConfigHandler *configHandler, QObject *parent = nullptr);

signals:
    void checkAllStarted();
    void checkAllFinished();
    void updateStarted(QString itemKey);
    void updateFinished(QString itemKey);
    void updateProgress(QString itemKey, int progress);

public slots:
    void checkAll();
    void update(const QString &itemKey);

private:
#ifdef QTLIB_UNIX
    QString describeAppImage(const QString &path) const;
    bool checkAppImage(const QString &path) const;
    void updateAppImage(const QString &itemKey);
#endif

    ConfigHandler *configHandler_;
};
