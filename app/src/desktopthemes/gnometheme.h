#pragma once

#include <QObject>

class GnomeTheme : public QObject
{
    Q_OBJECT

public:
    explicit GnomeTheme(const QString &path, QObject *parent = nullptr);

    bool applyAsWallpaper() const;
    bool applyAsIcon() const;
    bool applyAsCursor() const;
    bool applyAsGtk3Theme() const;
    bool applyAsGnomeShellTheme() const;

private:
    bool setConfig(const QString &schema, const QString &key, const QString &value) const;

    QString path_;
    QString themeName_;
};
