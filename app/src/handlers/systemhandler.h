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
    bool setConfigWithPlasmaShell(const QString &script) const;
    bool applyKdeWallpaper(const QString &path) const;
    bool applyKdeIcon(const QString &themeName) const;
    bool applyKdeCursor(const QString &themeName) const;
    bool applyKdePlasmaDesktoptheme(const QString &themeName) const;
    bool applyKdeAuroraeTheme(const QString &themeName) const;

    bool setConfigWithGsettings(const QString &schema, const QString &key, const QString &value) const;
    bool applyGnomeWallpaper(const QString &path) const;
    bool applyGnomeIcon(const QString &themeName) const;
    bool applyGnomeCursor(const QString &themeName) const;
    bool applyGnomeGtk3Theme(const QString &themeName) const;
    bool applyGnomeGnomeShellTheme(const QString &themeName) const;

    bool setConfigWithXfconf(const QString &channel, const QString &property, const QString &value) const;
    bool applyXfceWallpaper(const QString &path) const;
    bool applyXfceIcon(const QString &themeName) const;
    bool applyXfceCursor(const QString &themeName) const;
    bool applyXfceGtk2Theme(const QString &themeName) const;
    bool applyXfceXfwm4Theme(const QString &themeName) const;
#endif
};
