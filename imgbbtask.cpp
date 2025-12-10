#include "imgbbtask.h"
#include <QDebug>
#include <QThread>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QHttpMultiPart>
#include <QFile>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>

ImgBBTask::ImgBBTask(QObject *parent, QString filepath)
    : QObject{parent}
{
    qInfo() << this << "Constructed";
    m_filepath = filepath;
}

void ImgBBTask::run()
{
    qInfo() << this << "Running on: " << QThread::currentThread();

    QEventLoop loop;
    QNetworkAccessManager manager;
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart imagePart;
    QString fileName = QFileInfo(m_filepath).fileName();
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data; name=\"image\"; filename=\"" + fileName + "\""));

    QFile *file = new QFile(m_filepath);
    if(file->open(QIODevice::ReadOnly)) {
        imagePart.setBodyDevice(file);
        file->setParent(multiPart);
        multiPart->append(imagePart);
    } else {
        emit UploadFailed("Failed to open File");
        file->deleteLater();
        multiPart->deleteLater();
        return;
    }

    QString fullEndpoint = m_url + "?key=" + m_apikey;
    QUrl url(fullEndpoint);
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.post(request, multiPart);
    multiPart->setParent(reply);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray response = reply->readAll();
    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(reply->error() != QNetworkReply::NoError) {
        qWarning() << "ImgBB HTTP Error Code:" << httpStatusCode;
        qWarning() << "ImgBB Raw Response:" << response;

        QJsonDocument errorDoc = QJsonDocument::fromJson(response);
        QString errorMsg = errorDoc.object().value("error").toObject().value("message").toString();

        if (errorMsg.isEmpty()) {
            errorMsg = reply->errorString();
        }

        emit UploadFailed(errorMsg);
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(response);
    QString webUrl = doc.object().value("data").toObject()["url"].toString();
    bool success = doc.object().value("success").toBool();

    if (!webUrl.isEmpty() && success) {
        qInfo() << "ImgBB Upload Success:" << webUrl;
        emit UploadFinished(webUrl);
    } else {
        qWarning() << "ImgBB Success but no URL found. Response:" << response;
        emit UploadFailed("Parsed JSON but found no URL or failed");
    }

    file->deleteLater();
    multiPart->deleteLater();
}


