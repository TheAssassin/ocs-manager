#include "appimageupdater.h"

#include <QThread>

#include "appimage/update.h"

AppImageUpdater::AppImageUpdater(const QString &id, const QString &path, QObject *parent)
    : QObject(parent), id_(id), path_(path)
{
    isFinishedWithNoError_ = false;
    errorString_ = "";
}

QString AppImageUpdater::id() const
{
    return id_;
}

QString AppImageUpdater::path() const
{
    return path_;
}

QString AppImageUpdater::describeAppImage() const
{
    std::string description = "";
    appimage::update::Updater updater(path_.toStdString());
    updater.describeAppImage(description);
    return QString::fromStdString(description);
}

bool AppImageUpdater::checkAppImage() const
{
    bool updateAvailable = false;
    appimage::update::Updater updater(path_.toStdString());
    updater.checkForChanges(updateAvailable);
    return updateAvailable;
}

void AppImageUpdater::updateAppImage()
{
    isFinishedWithNoError_ = false;
    errorString_ = "";
    appimage::update::Updater updater(path_.toStdString(), false);

    if (!updater.start()) {
        emit finished(this);
        return;
    }

    while (!updater.isDone()) {
        QThread::msleep(100);
        double progress;
        if (updater.progress(progress)) {
            emit updateProgress(id_, progress);
        }
    }

    if (updater.hasError()) {
        std::string message;
        while (updater.nextStatusMessage(message)) {
            errorString_ += QString::fromStdString(message) + "\n";
        }
        emit finished(this);
        return;
    }

    isFinishedWithNoError_ = true;
    emit finished(this);
}

bool AppImageUpdater::isFinishedWithNoError() const
{
    return isFinishedWithNoError_;
}

QString AppImageUpdater::errorString() const
{
    return errorString_;
}
