#pragma once

#include <QObject>

class SystemHandler : public QObject
{
    Q_OBJECT

public:
    explicit SystemHandler(QObject *parent = 0);

public slots:
    bool isUnix();

    QString desktopEnvironment();
    bool isApplicableType(const QString &installType);

#ifdef QTLIB_UNIX
    bool applyFile(const QString &path, const QString &installType);
#endif

private:
#ifdef QTLIB_UNIX
    bool applyWallpaper(const QString &path);
    bool applyIcon(const QString &path);
    bool applyCursor(const QString &path);
    bool applyWindowTheme(const QString &path);
#endif
};
