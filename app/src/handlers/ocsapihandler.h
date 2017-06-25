#pragma once

#include <QObject>
#include <QJsonObject>

class ConfigHandler;

class OcsApiHandler : public QObject
{
    Q_OBJECT

public:
    explicit OcsApiHandler(ConfigHandler *configHandler, QObject *parent = nullptr);

public slots:
    bool addProviders(const QString &providerFileUrl) const;
    bool removeProvider(const QString &providerKey) const;
    bool updateAllCategories(bool force = false) const;
    bool updateCategories(const QString &providerKey, bool force = false) const;
    QJsonObject getContents(const QString &providerKeys = QString(), const QString &categoryKeys = QString(),
                            const QString &xdgTypes = QString(), const QString &packageTypes = QString(),
                            const QString &search = QString(), const QString &sortmode = QString("new"), int pagesize = 25, int page = 0) const;
    QJsonObject getContent(const QString &providerKey, const QString &contentId) const;

private:
    ConfigHandler *configHandler_;
};
