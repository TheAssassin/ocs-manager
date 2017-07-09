#pragma once

#include <QObject>

class DesktopThemeHandler : public QObject
{
    Q_OBJECT

public:
    explicit DesktopThemeHandler(QObject *parent = nullptr);

public slots:
    QString desktopEnvironment() const;
    bool isApplicableType(const QString &installType) const;

#ifdef QTLIB_UNIX
    bool applyTheme(const QString &path, const QString &installType) const;
#endif
};
