#include "supabasetask.h"
#include <QRunnable>
#include <QDebug>
#include <QThread>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <qobjectdefs.h>
#include <QUrlQuery>

SupabaseTask::SupabaseTask(SupabaseClient *client, Operation op, QVariantMap params, QString url, QString key)
{
    qInfo() << this << "Constructed for operation: " << op;
    m_operation = op;
    m_client = client;
    m_key = key;
    m_url = url;
    m_params = params;
}

void SupabaseTask::FetchSystems()
{
    QEventLoop loop;
    QNetworkAccessManager manager;

    QString endpoint = m_url + "/rest/v1/systems?select=*";
    QUrl url(endpoint);

    QNetworkRequest request(url);
    request.setRawHeader("apikey", m_key.toUtf8());
    request.setRawHeader("Authorization", ("Bearer " + m_key).toUtf8());
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    QNetworkReply *reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if(reply->error() != QNetworkReply::NoError) {
        QByteArray errorData = reply->readAll();
        QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
        QString detailedError = errorDoc.object().value("message").toString();
        QString errorMsg = detailedError.isEmpty() ? reply->errorString() : detailedError;
        qWarning() << "Error: " << errorMsg;
        QMetaObject::invokeMethod(m_client, "onError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, "Failed to Fetch Systems"),
                                  Q_ARG(QString, "Fetching Systems"),
                                  Q_ARG(QString, errorMsg));
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonArray array = doc.array();

    QVariantList systems = array.toVariantList();
    qInfo() << "Fetched" << systems.size() << "systems";
    reply->deleteLater();

    QString endpoint2 = m_url + "/rest/v1/claims?select=*";
    QUrl claimsUrl = QUrl(endpoint2);
    QNetworkRequest request2(claimsUrl);
    request2.setRawHeader("apikey", m_key.toUtf8());
    request2.setRawHeader("Authorization", ("Bearer " + m_key).toUtf8());
    request2.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    QNetworkReply *reply2 = manager.get(request2);
    QEventLoop loop2;
    QObject::connect(reply2, &QNetworkReply::finished, &loop2, &QEventLoop::quit);
    loop2.exec();

    if (reply2->error() != QNetworkReply::NoError) {
        QByteArray errorData2 = reply->readAll();
        QJsonDocument errorDoc2 = QJsonDocument::fromJson(errorData2);
        QString detailedError2 = errorDoc2.object().value("message").toString();
        QString errorMsg2 = detailedError2.isEmpty() ? reply->errorString() : detailedError2;
        qWarning() << "Error: " << errorMsg2;
        QMetaObject::invokeMethod(m_client, "onError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, "Failed to Fetch Claims"),
                                  Q_ARG(QString, "Fetching Claims"),
                                  Q_ARG(QString, errorMsg2));
        return;
    }

    QVariantList claims = QJsonDocument::fromJson(reply2->readAll()).array().toVariantList();
    qInfo() << "Fetched" << claims.size() << "claims";
    reply2->deleteLater();

    QString endpoint3 = m_url + "/rest/v1/system_tags?select=*";
    QUrl categoryUrl = QUrl(endpoint3);
    QNetworkRequest request3(categoryUrl);
    request3.setRawHeader("apikey", m_key.toUtf8());
    request3.setRawHeader("Authorization", ("Bearer " + m_key).toUtf8());
    request3.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    QNetworkReply *reply3 = manager.get(request3);
    QEventLoop loop3;
    QObject::connect(reply3, &QNetworkReply::finished, &loop3, &QEventLoop::quit);
    loop3.exec();

    if (reply3->error() != QNetworkReply::NoError) {
        QByteArray errorData3 = reply->readAll();
        QJsonDocument errorDoc3 = QJsonDocument::fromJson(errorData3);
        QString detailedError3 = errorDoc3.object().value("message").toString();
        QString errorMsg3 = detailedError3.isEmpty() ? reply->errorString() : detailedError3;
        qWarning() << "Error: " << errorMsg3;
        QMetaObject::invokeMethod(m_client, "onError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, "Failed to Fetch Category Images"),
                                  Q_ARG(QString, "Fetching Category Images"),
                                  Q_ARG(QString, errorMsg3));
        return;
    }

    QVariantList categorys = QJsonDocument::fromJson(reply3->readAll()).array().toVariantList();
    qInfo() << "Fetched" << categorys.size() << "categorys";
    reply3->deleteLater();
    QMetaObject::invokeMethod(m_client, "onSystemsLoaded",
                              Qt::QueuedConnection,
                              Q_ARG(QVariantList, systems),
                              Q_ARG(QVariantList, claims),
                              Q_ARG(QVariantList, categorys));
}

void SupabaseTask::FetchCategoryImages()
{
    QEventLoop loop;
    QNetworkAccessManager manager;

    QString endpoint = m_url + "/rest/v1/category_images?select=*";
    QUrl url(endpoint);

    QNetworkRequest request(url);
    request.setRawHeader("apikey", m_key.toUtf8());
    request.setRawHeader("Authorization", ("Bearer " + m_key).toUtf8());
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    QNetworkReply *reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if(reply->error() != QNetworkReply::NoError) {
        QByteArray errorData = reply->readAll();
        QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
        QString detailedError = errorDoc.object().value("message").toString();
        QString errorMsg = detailedError.isEmpty() ? reply->errorString() : detailedError;
        qWarning() << "Error: " << errorMsg;
        QMetaObject::invokeMethod(m_client, "onError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, "Failed to Fetch Category Images"),
                                  Q_ARG(QString, "Fetching Category Images"),
                                  Q_ARG(QString, errorMsg));
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonArray array = doc.array();
    QVariantMap categoryImages;

    for(int i = 0; i < array.size(); i++) {
        QJsonObject obj = array[i].toObject();
        QString category = obj["tag"].toString();
        QString url = obj["background_url"].toString();
        categoryImages[category] = url;
    }

    qInfo() << "Fetched" << categoryImages.size() << "categoryImages";
    reply->deleteLater();
    QMetaObject::invokeMethod(m_client, "onCategoryLoaded",
                              Qt::QueuedConnection,
                              Q_ARG(QVariantMap, categoryImages));
}

void SupabaseTask::FetchDbMetaData()
{
    QEventLoop loop;
    QNetworkAccessManager manager;

    QString endpoint = m_url + "/rest/v1/db_metadata?select=*";
    QUrl url(endpoint);

    QNetworkRequest request(url);
    request.setRawHeader("apikey", m_key.toUtf8());
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    request.setRawHeader("Authorization", ("Bearer " + m_key).toUtf8());
    QNetworkReply *reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if(reply->error() != QNetworkReply::NoError) {
        QByteArray errorData = reply->readAll();
        QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
        QString detailedError = errorDoc.object().value("message").toString();
        QString errorMsg = detailedError.isEmpty() ? reply->errorString() : detailedError;
        qWarning() << "Error: " << errorMsg;
        QMetaObject::invokeMethod(m_client, "onError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, "Failed to Fetch Database Status"),
                                  Q_ARG(QString, "Fetching Database Status"),
                                  Q_ARG(QString, errorMsg));
        reply->deleteLater();
        return;
    }
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonArray array = doc.array();
    QJsonObject metadata = array[0].toObject();
    int newThisWeek = metadata["new_this_week"].toInt();
    QString lastUpdated = metadata["last_updated"].toString();
    QVariantList dbData;
    dbData.append(newThisWeek);
    dbData.append(lastUpdated);
    QMetaObject::invokeMethod(m_client, "onDbLoaded",
                              Qt::QueuedConnection,
                              Q_ARG(QVariantList, dbData));
}

void SupabaseTask::FetchContributions()
{
    QEventLoop loop;
    QNetworkAccessManager manager;

    QString endpoint = m_url + "/rest/v1/user_contributions?select=*";
    QUrl url = QUrl(endpoint);

    QNetworkRequest request(url);
    request.setRawHeader("apikey", m_key.toUtf8());
    request.setRawHeader("Authorization", ("Bearer " + m_key).toUtf8());
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    QNetworkReply *reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if(reply->error() != QNetworkReply::NoError) {
        QByteArray errorData = reply->readAll();
        QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
        QString detailedError = errorDoc.object().value("message").toString();
        QString errorMsg = detailedError.isEmpty() ? reply->errorString() : detailedError;
        qWarning() << "Error: " << errorMsg;
        QMetaObject::invokeMethod(m_client, "onError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, "Failed to Fetch Contributions"),
                                  Q_ARG(QString, "Fetching Contributions"),
                                  Q_ARG(QString, errorMsg));
        reply->deleteLater();
        return;
    }
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonArray array = doc.array();
    QVariantList fulldata = array.toVariantList();
    qInfo() << "Fetched" << fulldata.size() <<  "contributions";
    reply->deleteLater();
    QMetaObject::invokeMethod(m_client, "onContributionsLoaded",
                              Qt::QueuedConnection,
                              Q_ARG(QVariantList, fulldata));
}

void SupabaseTask::FetchSystemImages()
{
    QEventLoop loop;
    QNetworkAccessManager manager;

    QString endpoint = m_url + "/rest/v1/system_images?select=*";
    QUrl url = QUrl(endpoint);

    QNetworkRequest request(url);
    request.setRawHeader("apikey", m_key.toUtf8());
    request.setRawHeader("Authorization", ("Bearer " + m_key).toUtf8());
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    QNetworkReply *reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if(reply->error() != QNetworkReply::NoError) {
        QByteArray errorData = reply->readAll();
        QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
        QString detailedError = errorDoc.object().value("message").toString();
        QString errorMsg = detailedError.isEmpty() ? reply->errorString() : detailedError;
        qWarning() << "Error: " << errorMsg;
        QMetaObject::invokeMethod(m_client, "onError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, "Failed to Fetch System Images"),
                                  Q_ARG(QString, "Fetching System Images"),
                                  Q_ARG(QString, errorMsg));
        reply->deleteLater();
        return;
    }
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonArray array = doc.array();
    QVariantList fulldata = array.toVariantList();
    qInfo() << "Fetched" << fulldata.size() <<  "images";
    reply->deleteLater();
    QMetaObject::invokeMethod(m_client, "onImagesLoaded",
                              Qt::QueuedConnection,
                              Q_ARG(QVariantList, fulldata));
}

void SupabaseTask::ClaimSystem()
{
    QEventLoop loop;
    QNetworkAccessManager manager;

    QString endpoint = m_url + "/rest/v1/claims?select=*";
    QUrl url = QUrl(endpoint);

    QNetworkRequest request(url);
    request.setRawHeader("apikey", m_key.toUtf8());
    request.setRawHeader("Authorization", ("Bearer " + m_key).toUtf8());
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    request.setRawHeader("Prefer", "return=representation");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject requestParams = QJsonObject::fromVariantMap(m_params);
    QJsonDocument doc(requestParams);
    QByteArray data = doc.toJson();
    QNetworkReply *reply = manager.post(request, data);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if(reply->error() != QNetworkReply::NoError) {
        QByteArray errorData = reply->readAll();
        QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
        QString detailedError = errorDoc.object().value("message").toString().toUtf8();
        QString errorMsg = detailedError.isEmpty() ? reply->errorString() : detailedError;
        qWarning() << "Error: " << errorMsg;
        QMetaObject::invokeMethod(m_client, "onError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, "Could Not Claim"),
                                  Q_ARG(QString, "Claiming a system"),
                                  Q_ARG(QString, errorMsg));
        reply->deleteLater();
        return;
    }
    QByteArray responseData = reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(responseData);
    QJsonArray array = document.array();
    QVariantList fulldata = array.toVariantList();
    QString systemName;
    QString cmdrName;
    for(int i = 0; i < fulldata.size(); i++) {
        QVariantMap minidata = fulldata[i].toMap();
        if(minidata["system_name"].toString() == m_params["system_name"].toString()) {
            systemName = minidata["system_name"].toString();
            cmdrName = minidata["cmdr_name"].toString();
        }
    }
    qInfo() << "Fetched" << fulldata.size() <<  "images";
    reply->deleteLater();
    QMetaObject::invokeMethod(m_client, "onClaimSuccess",
                              Qt::QueuedConnection,
                              Q_ARG(QString, systemName),
                              Q_ARG(QString, cmdrName));
}

void SupabaseTask::UnclaimSystem()
{
    QEventLoop loop;
    QNetworkAccessManager manager;

    QString endpoint = m_url + "/rest/v1/claims?system_name=eq." + QUrl::toPercentEncoding(m_params["system_name"].toString())
                       + "&cmdr_name=eq." + QUrl::toPercentEncoding(m_params["cmdr_name"].toString());
    QUrl url = QUrl(endpoint);

    QNetworkRequest request(url);
    request.setRawHeader("apikey", m_key.toUtf8());
    request.setRawHeader("Authorization", ("Bearer " + m_key).toUtf8());
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    QNetworkReply *reply = manager.deleteResource(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if(reply->error() != QNetworkReply::NoError) {
        QByteArray errorData = reply->readAll();
        QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
        QString detailedError = errorDoc.object().value("message").toString();
        QString errorMsg = detailedError.isEmpty() ? reply->errorString() : detailedError;

        qWarning() << "Error: " << errorMsg;
        QMetaObject::invokeMethod(m_client, "onError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, "Could Not Unclaim"),
                                  Q_ARG(QString, "Unclaiming a System"),
                                  Q_ARG(QString, errorMsg));
        reply->deleteLater();
        return;
    }
    reply->deleteLater();
    QMetaObject::invokeMethod(m_client, "onUnclaimSuccess",
                              Qt::QueuedConnection,
                              Q_ARG(QString, m_params["system_name"].toString()));
}

void SupabaseTask::AddContributions()
{
    QEventLoop loop;
    QNetworkAccessManager manager;

    QString endpoint = m_url + "/rest/v1/user_contributions?on_conflict=system_name,cmdr_name";
    QUrl url = QUrl(endpoint);

    QNetworkRequest request(url);
    request.setRawHeader("apikey", m_key.toUtf8());
    request.setRawHeader("Authorization", ("Bearer " + m_key).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Prefer", "resolution=merge-duplicates,return=representation");
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

    QJsonObject requestParams = QJsonObject::fromVariantMap(m_params);
    QJsonDocument doc(requestParams);
    QByteArray data = doc.toJson();

    QNetworkReply *reply = manager.post(request, data);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if(reply->error() != QNetworkReply::NoError) {
        QByteArray errorData = reply->readAll();
        QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
        QString detailedError = errorDoc.object().value("message").toString();
        QString errorMsg = detailedError.isEmpty() ? reply->errorString() : detailedError;

        qWarning() << "Error: " << errorMsg;
        QMetaObject::invokeMethod(m_client, "onError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, "Could Not Add Contribution"),
                                  Q_ARG(QString, "Adding a contribution"),
                                  Q_ARG(QString, errorMsg));
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
    QVariantList resultList = responseDoc.array().toVariantList();
    QVariantMap confirmedData;
    if(!resultList.isEmpty()) {
        confirmedData = resultList.first().toMap();
    }

    reply->deleteLater();

    QMetaObject::invokeMethod(m_client, "onContributionsAdded",
                              Qt::QueuedConnection,
                              Q_ARG(QVariantMap, confirmedData));
}

void SupabaseTask::UploadImage()
{
    QEventLoop loop;
    QNetworkAccessManager manager;

    QString endpoint = m_url + "/rest/v1/system_images";
    QUrl url = QUrl(endpoint);

    QNetworkRequest request(url);
    request.setRawHeader("apikey", m_key.toUtf8());
    request.setRawHeader("Authorization", ("Bearer " + m_key).toUtf8());
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Prefer", "return=representation");

    QJsonObject requestParams = QJsonObject::fromVariantMap(m_params);
    QJsonDocument doc(requestParams);
    QByteArray data = doc.toJson();

    QNetworkReply *reply = manager.post(request, data);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if(reply->error() != QNetworkReply::NoError) {
        QByteArray errorData = reply->readAll();
        QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
        QString detailedError = errorDoc.object().value("message").toString();
        QString errorMsg = detailedError.isEmpty() ? reply->errorString() : detailedError;
        qWarning() << "Error: " << errorMsg;
        QMetaObject::invokeMethod(m_client, "onError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, "Could Not Upload Image"),
                                  Q_ARG(QString, "Uploading an image"),
                                  Q_ARG(QString, errorMsg));
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
    QJsonArray resultArray = responseDoc.array();
    if (!resultArray.isEmpty()) {
        QJsonValue firstValue = resultArray.first();
        QVariantMap confirmedData = firstValue.toObject().toVariantMap();
        QMetaObject::invokeMethod(m_client, "onImageSaved",
                                  Qt::QueuedConnection,
                                  Q_ARG(QVariantMap, confirmedData));
    } else {
        qWarning() << "Database returned an empty array on successful image save.";
    }
    reply->deleteLater();
}

void SupabaseTask::RemoveImage()
{
    QEventLoop loop;
    QNetworkAccessManager manager;
    QString imageUrl = m_params["image_url"].toString().trimmed();
    QString cmdrName = m_params["cmdr_name"].toString().trimmed();

    QUrl url(m_url + "/rest/v1/system_images");
    QUrlQuery endpoint;

    endpoint.addQueryItem("image_url", "eq." + imageUrl);
    endpoint.addQueryItem("uploaded_by", "eq." + cmdrName);

    url.setQuery(endpoint);
    QNetworkRequest request(url);
    request.setRawHeader("apikey", m_key.toUtf8());
    request.setRawHeader("Authorization", ("Bearer " + m_key).toUtf8());
    request.setRawHeader("Prefer", "return=representation");
    request.setRawHeader("Cmdr-Name", cmdrName.toUtf8());
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    QNetworkReply *reply = manager.deleteResource(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if(reply->error() != QNetworkReply::NoError) {
        QByteArray errorData = reply->readAll();
        QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
        QString detailedError = errorDoc.object().value("message").toString();
        QString errorMsg = detailedError.isEmpty() ? reply->errorString() : detailedError;
        qWarning() << "Error: " << errorMsg;
        QMetaObject::invokeMethod(m_client, "onError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, "Could Not Remove Image"),
                                  Q_ARG(QString, "Removing an image"),
                                  Q_ARG(QString, errorMsg));
        reply->deleteLater();
        return;
    }
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonArray arr = doc.array();

    if (arr.isEmpty()) {
        qWarning() << "DELETE Request succeeded, but 0 rows were deleted. URL mismatch?";
        qWarning() << "Tried to delete:" << imageUrl;
    } else {
        qInfo() << "Successfully deleted row(s):" << arr.size();
        QMetaObject::invokeMethod(m_client, "onImageRemoved",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, imageUrl));
    }
    reply->deleteLater();
    QMetaObject::invokeMethod(m_client, "onImageRemoved",
                              Qt::QueuedConnection,
                              Q_ARG(QString, imageUrl));
}

void SupabaseTask::run()
{
    qInfo() << this << "Running on: " << QThread::currentThread();

    switch (m_operation) {
    case FETCH_SYSTEMS:
        FetchSystems();
        break;
    case FETCH_CATEGORY_IMAGES:
        FetchCategoryImages();
        break;
    case CLAIM_SYSTEM:
        ClaimSystem();
        break;
    case UNCLAIM_SYSTEM:
        UnclaimSystem();
        break;
    case FETCH_DB_METADATA:
        FetchDbMetaData();
        break;
    case FETCH_CONTRIBUTIONS:
        FetchContributions();
        break;
    case FETCH_SYSTEM_IMAGES:
        FetchSystemImages();
        break;
    case ADD_CONTRIBUTION:
        AddContributions();
        break;
    case UPLOAD_IMAGE:
        UploadImage();
        break;
    case REMOVE_IMAGE:
        RemoveImage();
        break;
    }
}
