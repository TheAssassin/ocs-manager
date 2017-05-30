#pragma once

#include <QObject>

class SystemHandler : public QObject
{
    Q_OBJECT

public:
    explicit SystemHandler(QObject *parent = nullptr);

public slots:
    bool isUnix() const;
    bool isMobileDevice() const;
    bool openUrl(const QString &url) const;

    QString desktopEnvironment() const;
    bool isApplicableType(const QString &installType) const;

#ifdef QTLIB_UNIX
    bool applyFile(const QString &path, const QString &installType) const;
#endif

private:
#ifdef QTLIB_UNIX
    bool applyWallpaper(const QString &path) const;
    bool applyIcon(const QString &path) const;
    bool applyCursor(const QString &path) const;
    bool applyWindowTheme(const QString &path) const;
#endif
};
