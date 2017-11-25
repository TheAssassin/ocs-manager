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
    void updateStarted(QString fileKey);
    void updateFinished(QString fileKey);
    void updateProgress(QString fileKey, int progress);

public slots:
    void checkAll();
    void update(const QString &fileKey);

private:
#ifdef QTLIB_UNIX
    QString describeAppImage(const QString &path) const;
    bool checkAppImage(const QString &path) const;
    void updateAppImage(const QString &fileKey);
#endif

    ConfigHandler *configHandler_;
};
