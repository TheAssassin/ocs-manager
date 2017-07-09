#pragma once

#include <QObject>

class XfceTheme : public QObject
{
    Q_OBJECT

public:
    explicit XfceTheme(const QString &path, QObject *parent = nullptr);

    bool applyAsWallpaper() const;
    bool applyAsIcon() const;
    bool applyAsCursor() const;
    bool applyAsGtk2Theme() const;
    bool applyAsXfwm4Theme() const;

private:
    bool setConfig(const QString &channel, const QString &property, const QString &value) const;

    QString path_;
    QString themeName_;
};
