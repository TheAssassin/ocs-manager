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
    bool applyKdeWallpaper(const QString &path) const;
    bool applyKdeIcon(const QString &path) const;
    bool applyKdeCursor(const QString &path) const;
    bool applyKdePlasmaDesktoptheme(const QString &path) const;
    bool applyKdeAuroraeTheme(const QString &path) const;

    bool applyGnomeWallpaper(const QString &path) const;
    bool applyGnomeIcon(const QString &path) const;
    bool applyGnomeCursor(const QString &path) const;
    bool applyGnomeGtk3Theme(const QString &path) const;
    bool applyGnomeGnomeShellTheme(const QString &path) const;

    bool applyXfceWallpaper(const QString &path) const;
#endif
};
