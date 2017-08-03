#include "matetheme.h"

#include <QStringList>
#include <QDir>
#include <QProcess>

MateTheme::MateTheme(const QString &path, QObject *parent)
    : QObject(parent), path_(path)
{
    themeName_ = QDir(path_).dirName();
}

bool MateTheme::applyAsWallpaper() const
{
    return setConfig("org.mate.background", "picture-filename", path_);
}

bool MateTheme::applyAsIcon() const
{
    return setConfig("org.mate.interface", "icon-theme", themeName_);
}

bool MateTheme::applyAsCursor() const
{
    return setConfig("org.mate.peripherals-mouse", "cursor-theme", themeName_);
}

bool MateTheme::applyAsGtk2Theme() const
{
    return setConfig("org.mate.interface", "gtk-theme", themeName_);
}

bool MateTheme::applyAsMetacityTheme() const
{
    return setConfig("org.mate.Marco.general", "theme", themeName_);
}

bool MateTheme::setConfig(const QString &schema, const QString &key, const QString &value) const
{
    return QProcess::startDetached("gsettings", QStringList() << "set" << schema << key << value);
}
