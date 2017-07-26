#pragma once

#include <QObject>

class CinnamonTheme : public QObject
{
    Q_OBJECT

public:
    explicit CinnamonTheme(const QString &path, QObject *parent = nullptr);

    bool applyAsWallpaper() const;
    bool applyAsIcon() const;
    bool applyAsCursor() const;
    bool applyAsGtk3Theme() const;
    bool applyAsCinnamonTheme() const;

private:
    bool setConfig(const QString &schema, const QString &key, const QString &value) const;

    QString path_;
    QString themeName_;
};
