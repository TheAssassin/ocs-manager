#include "gnometheme.h"

#include <QStringList>
#include <QDir>
#include <QProcess>

GnomeTheme::GnomeTheme(const QString &path, QObject *parent)
    : QObject(parent), path_(path)
{
    themeName_ = QDir(path_).dirName();
}

bool GnomeTheme::applyAsWallpaper() const
{
    return setConfig("org.gnome.desktop.background", "picture-uri", "file://" + path_);
}

bool GnomeTheme::applyAsIcon() const
{
    return setConfig("org.gnome.desktop.interface", "icon-theme", themeName_);
}

bool GnomeTheme::applyAsCursor() const
{
    return setConfig("org.gnome.desktop.interface", "cursor-theme", themeName_);
}

bool GnomeTheme::applyAsGtk3Theme() const
{
    return setConfig("org.gnome.desktop.interface", "gtk-theme", themeName_);
}

bool GnomeTheme::applyAsGnomeShellTheme() const
{
    return setConfig("org.gnome.shell.extensions.user-theme", "name", themeName_);
}

bool GnomeTheme::setConfig(const QString &schema, const QString &key, const QString &value) const
{
    return QProcess::startDetached("gsettings", QStringList() << "set" << schema << key << value);
}
