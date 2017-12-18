#pragma once

#include <QObject>

class AppImageUpdater : public QObject
{
    Q_OBJECT

public:
    explicit AppImageUpdater(const QString &id, const QString &path, QObject *parent = nullptr);

    QString id() const;
    QString path() const;

    QString describeAppImage() const;
    bool checkAppImage() const;
    void updateAppImage();
    bool isFinishedWithNoError() const;
    QString errorString() const;

signals:
    void finished(AppImageUpdater *updater);
    void updateProgress(QString id, double progress);

private:
    QString id_;
    QString path_;
    bool isFinishedWithNoError_;
    QString errorString_;
};
