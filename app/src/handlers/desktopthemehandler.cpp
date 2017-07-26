#include "desktopthemehandler.h"

#include <QStringList>

#ifdef QTLIB_UNIX
#include <QFileInfo>

#include "desktopthemes/kdetheme.h"
#include "desktopthemes/gnometheme.h"
#include "desktopthemes/xfcetheme.h"
#include "desktopthemes/cinnamontheme.h"
#include "desktopthemes/matetheme.h"
#endif

DesktopThemeHandler::DesktopThemeHandler(QObject *parent)
    : QObject(parent)
{}

QString DesktopThemeHandler::desktopEnvironment() const
{
    QString desktop;
    QString currentDesktop;

    if (!qgetenv("XDG_CURRENT_DESKTOP").isEmpty()) {
        currentDesktop = QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP").constData()).toLower();
    }
    else if (!qgetenv("XDG_SESSION_DESKTOP").isEmpty()) {
        currentDesktop = QString::fromLocal8Bit(qgetenv("XDG_SESSION_DESKTOP").constData()).toLower();
    }
    else if (!qgetenv("DESKTOP_SESSION").isEmpty()) {
        currentDesktop = QString::fromLocal8Bit(qgetenv("DESKTOP_SESSION").constData()).toLower();
    }

    if (currentDesktop.contains("kde") || currentDesktop.contains("plasma")) {
        desktop = "kde";
    }
    else if (currentDesktop.contains("gnome") || currentDesktop.contains("unity")) {
        desktop = "gnome";
    }
    else if (currentDesktop.contains("xfce")) {
        desktop = "xfce";
    }
    else if (currentDesktop.contains("cinnamon")) {
        desktop = "cinnamon";
    }
    else if (currentDesktop.contains("mate")) {
        desktop = "mate";
    }

    return desktop;
}

bool DesktopThemeHandler::isApplicableType(const QString &installType) const
{
    auto desktop = desktopEnvironment();

    QStringList applicableTypes;

    if (desktop == "kde") {
        applicableTypes << "wallpapers"
                        << "icons"
                        << "cursors"
                        << "plasma5_desktopthemes"
                        << "aurorae_themes";
    }
    else if (desktop == "gnome") {
        applicableTypes << "wallpapers"
                        << "icons"
                        << "cursors"
                        << "gtk3_themes"
                        << "gnome_shell_themes";
    }
    else if (desktop == "xfce") {
        applicableTypes << "wallpapers"
                        << "icons"
                        << "cursors"
                        << "gtk2_themes"
                        << "xfwm4_themes";
    }
    else if (desktop == "cinnamon") {
        applicableTypes << "wallpapers"
                        << "icons"
                        << "cursors"
                        << "gtk3_themes"
                        << "cinnamon_themes";
    }
    else if (desktop == "mate") {
        applicableTypes << "wallpapers"
                        << "icons"
                        << "cursors"
                        << "gtk2_themes"
                        << "metacity_themes";
    }

    return applicableTypes.contains(installType);
}

#ifdef QTLIB_UNIX
bool DesktopThemeHandler::applyTheme(const QString &path, const QString &installType) const
{
    if (QFileInfo::exists(path) && isApplicableType(installType)) {
        auto desktop = desktopEnvironment();

        if (desktop == "kde") {
            KdeTheme kdeTheme(path);
            if (installType == "wallpapers") {
                return kdeTheme.applyAsWallpaper();
            }
            else if (installType == "icons") {
                return kdeTheme.applyAsIcon();
            }
            else if (installType == "cursors") {
                return kdeTheme.applyAsCursor();
            }
            else if (installType == "plasma5_desktopthemes") {
                return kdeTheme.applyAsPlasma5Desktoptheme();
            }
            else if (installType == "aurorae_themes") {
                return kdeTheme.applyAsAuroraeTheme();
            }
        }
        else if (desktop == "gnome") {
            GnomeTheme gnomeTheme(path);
            if (installType == "wallpapers") {
                return gnomeTheme.applyAsWallpaper();
            }
            else if (installType == "icons") {
                return gnomeTheme.applyAsIcon();
            }
            else if (installType == "cursors") {
                return gnomeTheme.applyAsCursor();
            }
            else if (installType == "gtk3_themes") {
                return gnomeTheme.applyAsGtk3Theme();
            }
            else if (installType == "gnome_shell_themes") {
                return gnomeTheme.applyAsGnomeShellTheme();
            }
        }
        else if (desktop == "xfce") {
            XfceTheme xfceTheme(path);
            if (installType == "wallpapers") {
                return xfceTheme.applyAsWallpaper();
            }
            else if (installType == "icons") {
                return xfceTheme.applyAsIcon();
            }
            else if (installType == "cursors") {
                return xfceTheme.applyAsCursor();
            }
            else if (installType == "gtk2_themes") {
                return xfceTheme.applyAsGtk2Theme();
            }
            else if (installType == "xfwm4_themes") {
                return xfceTheme.applyAsXfwm4Theme();
            }
        }
        else if (desktop == "cinnamon") {
            CinnamonTheme cinnamonTheme(path);
            if (installType == "wallpapers") {
                return cinnamonTheme.applyAsWallpaper();
            }
            else if (installType == "icons") {
                return cinnamonTheme.applyAsIcon();
            }
            else if (installType == "cursors") {
                return cinnamonTheme.applyAsCursor();
            }
            else if (installType == "gtk3_themes") {
                return cinnamonTheme.applyAsGtk3Theme();
            }
            else if (installType == "gnome_shell_themes") {
                return cinnamonTheme.applyAsCinnamonTheme();
            }
        }
        else if (desktop == "mate") {
            MateTheme mateTheme(path);
            if (installType == "wallpapers") {
                return mateTheme.applyAsWallpaper();
            }
            else if (installType == "icons") {
                return mateTheme.applyAsIcon();
            }
            else if (installType == "cursors") {
                return mateTheme.applyAsCursor();
            }
            else if (installType == "gtk2_themes") {
                return mateTheme.applyAsGtk2Theme();
            }
            else if (installType == "metacity_themes") {
                return mateTheme.applyAsMetacityTheme();
            }
        }
    }

    return false;
}
#endif
