#ifndef IMGBBTASK_H
#define IMGBBTASK_H

#include <QObject>
#include <QString>
#include <QRunnable>

class ImgBBTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    ImgBBTask(QObject *parent, QString filepath);
    void run() override;
signals:
    void UploadFinished(QString link);
    void UploadFailed(QString errorCode);
private:
    const QString m_apikey = "55c47d5a13627071d32dbe3ea4125869";
    const QString m_url = "https://api.imgbb.com/1/upload";
    QString m_filepath;
};

#endif // IMGBBTASK_H
