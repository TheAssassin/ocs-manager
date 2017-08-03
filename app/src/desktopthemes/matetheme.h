#pragma once

#include <QObject>

class MateTheme : public QObject
{
    Q_OBJECT

public:
    explicit MateTheme(const QString &path, QObject *parent = nullptr);

    bool applyAsWallpaper() const;
    bool applyAsIcon() const;
    bool applyAsCursor() const;
    bool applyAsGtk2Theme() const;
    bool applyAsMetacityTheme() const;

private:
    bool setConfig(const QString &schema, const QString &key, const QString &value) const;

    QString path_;
    QString themeName_;
};
