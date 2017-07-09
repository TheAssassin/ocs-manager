#pragma once

#include <QObject>

class KdeTheme : public QObject
{
    Q_OBJECT

public:
    explicit KdeTheme(const QString &path, QObject *parent = nullptr);

    bool applyAsWallpaper() const;
    bool applyAsIcon() const;
    bool applyAsCursor() const;
    bool applyAsPlasmaDesktoptheme() const;
    bool applyAsAuroraeTheme() const;

private:
    bool evaluateScript(const QString &script) const;

    QString path_;
    QString themeName_;
};
