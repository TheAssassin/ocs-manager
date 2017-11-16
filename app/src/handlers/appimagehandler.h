#pragma once

#include <QObject>

class ConfigHandler;

class AppImageHandler : public QObject
{
    Q_OBJECT

public:
    explicit AppImageHandler(ConfigHandler *configHandler, QObject *parent = nullptr);

public slots:
    bool isUpdateAvailable(const QString &path) const;

#ifdef QTLIB_UNIX
    bool updateAppImage(const QString &path) const;
#endif

private:
    ConfigHandler *configHandler_;
};
