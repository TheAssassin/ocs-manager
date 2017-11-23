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
    void updateStarted(QString path);
    void updateFinished(QString path, QString newPath);
    void updateProgress(QString path, int progress);

public slots:
    bool checkAll() const;
    bool update(const QString &path) const;

private:
#ifdef QTLIB_UNIX
    QString describeAppImage(const QString &path) const;
    bool checkAppImage(const QString &path) const;
    bool updateAppImage(const QString &path);
#endif

    ConfigHandler *configHandler_;
};
