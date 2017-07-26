#include "cinnamontheme.h"

#include <QStringList>
#include <QDir>
#include <QProcess>

CinnamonTheme::CinnamonTheme(const QString &path, QObject *parent)
    : QObject(parent), path_(path)
{
    themeName_ = QDir(path_).dirName();
}

bool CinnamonTheme::applyAsWallpaper() const
{
    return setConfig("org.cinnamon.desktop.background", "picture-uri", "file://" + path_);
}

bool CinnamonTheme::applyAsIcon() const
{
    return setConfig("org.cinnamon.desktop.interface", "icon-theme", themeName_);
}

bool CinnamonTheme::applyAsCursor() const
{
    return setConfig("org.cinnamon.desktop.interface", "cursor-theme", themeName_);
}

bool CinnamonTheme::applyAsGtk3Theme() const
{
    return setConfig("org.cinnamon.desktop.interface", "gtk-theme", themeName_);
}

bool CinnamonTheme::applyAsCinnamonTheme() const
{
    return setConfig("org.cinnamon.theme", "name", themeName_);
}

bool CinnamonTheme::setConfig(const QString &schema, const QString &key, const QString &value) const
{
    return QProcess::startDetached("gsettings", QStringList() << "set" << schema << key << value);
}
