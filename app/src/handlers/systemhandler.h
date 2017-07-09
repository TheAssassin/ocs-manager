#pragma once

#include <QObject>

class SystemHandler : public QObject
{
    Q_OBJECT

public:
    explicit SystemHandler(QObject *parent = nullptr);

public slots:
    bool isUnix() const;
    bool isMobileDevice() const;
    bool openUrl(const QString &url) const;
};
