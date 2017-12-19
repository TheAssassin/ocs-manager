#pragma once

#include <QObject>

namespace appimage {
namespace update {
class Updater;
}
}

class AppImageUpdater : public QObject
{
    Q_OBJECT

public:
    explicit AppImageUpdater(const QString &id, const QString &path, QObject *parent = nullptr);
    ~AppImageUpdater();

    QString id() const;
    QString path() const;
    bool isFinishedWithNoError() const;
    QString errorString() const;

    QString describeAppImage() const;
    bool checkForChanges() const;
    void start();

signals:
    void finished(AppImageUpdater *updater);
    void updateProgress(QString id, double progress);

private slots:
    void checkProgress();

private:
    QString id_;
    QString path_;
    bool isFinishedWithNoError_;
    QString errorString_;
    appimage::update::Updater *updater_;
};
