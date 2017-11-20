#pragma once

#include <QObject>

class ConfigHandler;

class AppImageHandler : public QObject
{
    Q_OBJECT

public:
    explicit AppImageHandler(ConfigHandler *configHandler, QObject *parent = nullptr);

signals:
    void updateStarted(QString path);
    void updateFinished(QString path);
    void updateProgress(QString path, int progress);

public slots:
    QString describeAppImage(const QString &path) const;
    bool isUpdateAvailable(const QString &path) const;

#ifdef QTLIB_UNIX
    bool updateAppImage(const QString &path);
#endif

private:
    ConfigHandler *configHandler_;
};
