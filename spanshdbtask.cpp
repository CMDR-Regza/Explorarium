#include "spanshdbtask.h"
#include "spanshplotter.h"
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
#include <QTimer>


SpanshDBTask::SpanshDBTask(SpanshPlotter *manager, QString id64, QString task, QString systemName)
{
    m_manager = manager;
    m_id64 = id64;
    m_task = task;
    m_systemName = systemName;
    setAutoDelete(true);
}

void SpanshDBTask::run()
{
    qInfo() << "SpanshDBTask running on" << QThread::currentThread() << " with task: " << m_task;
    if(m_task == "system" && !m_id64.isEmpty()) {
        systemTask();
    }
    if(m_task == "edsm" && !m_systemName.isEmpty()) {
        edsmTask();
    }
    if(m_task == "systemMap" && !m_systemName.isEmpty()) {
        mapTask();
    }
}

void SpanshDBTask::systemTask()
{
    QEventLoop loop;
    QTimer timer;
    QNetworkAccessManager manager;
    timer.setSingleShot(true);

    QString endpoint = m_systemLink + m_id64;
    QUrl url(endpoint);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Explorarium/1.0");
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    QNetworkReply *reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(10000);
    loop.exec();
    timer.stop();
    if (!reply->isFinished()) {
        reply->abort();
        return;
    }
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QNetworkReply::NetworkError netError = reply->error();
    if (netError != QNetworkReply::NoError && httpStatus != 404) {
        qWarning() << "Network Error:" << reply->errorString();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonObject obj = doc.object();
    QVariantMap result;

    if (obj.contains("record")) {
        obj = obj.value("record").toObject();
    }

    if(!obj.contains("error")) {
        if(obj.contains("name")) {
            result["name"] = obj.value("name").toVariant();
        } else {
            result["name"] = QVariant("Unknown System");
        }

        if(obj.contains("updated_at")) {
            result["updated_at"] = obj.value("updated_at").toVariant();
        } else {
            result["updated_at"] = QVariant("Unknown Date");
        }

        if(obj.contains("body_count")) {
            result["full_body_count"] = obj.value("body_count").toVariant();
        } else {
            result["full_body_count"] = QVariant("??");
        }

        QJsonArray bodies = obj.value("bodies").toArray();
        int body_count = bodies.size();

        result["body_count"] = QVariant(body_count);
    } else {
        result["error"] = obj.value("error").toVariant();
        qInfo() << "Spansh Error found:" << result["error"].toString();
    }
    result["id"] = QVariant("spansh");
    qInfo() << "Fetched" << result.size() << "from Spansh";
    QMetaObject::invokeMethod(m_manager, "targetLoadingBay",
                              Qt::QueuedConnection,
                              Q_ARG(QVariantMap, result));
}

void SpanshDBTask::edsmTask()
{
    QEventLoop loop;
    QTimer timer;
    QNetworkAccessManager manager;
    timer.setSingleShot(true);

    QString endpoint = m_edsmLink;
    QUrl url(endpoint);

    QUrlQuery query;
    query.addQueryItem("systemName", m_systemName);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Explorarium/1.0");
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    QNetworkReply *reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(10000);
    loop.exec();
    timer.stop();
    if (!reply->isFinished()) {
        reply->abort();
        return;
    }
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QNetworkReply::NetworkError netError = reply->error();
    if (netError != QNetworkReply::NoError && httpStatus != 404) {
        qWarning() << "Network Error:" << reply->errorString();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonObject obj = doc.object();
    QVariantMap result;

    if(obj.isEmpty()) {
        result["error"] = "empty";
    } else {
        if(obj.contains("name")) {
            result["name"] = obj.value("name").toVariant();
        } else {
            result["name"] = QVariant("Unknown System");
        }

        if(obj.contains("bodyCount")) {
            result["fullbodycount"] = obj.value("bodyCount").toVariant();
        } else {
            result["fullbodycount"] = QVariant("??");
        }

        if (obj.contains("bodies")) {
            QJsonArray bodies = obj.value("bodies").toArray();
            result["bodycount"] = !bodies.isEmpty() ? bodies.size() : 0;
            if (!bodies.isEmpty()) {
                QJsonObject star = bodies[0].toObject();
                if (!star.isEmpty()) {
                    if (star.contains("discovery")) {
                        QJsonObject discovery = star.value("discovery").toObject();
                        result["commander"] = discovery.value("commander").toVariant();
                        result["date"] = discovery.value("date").toVariant();
                    }
                }
            } else {
                result["commander"] = QVariant("Unknown CMDR");
                result["date"] = QVariant("Unknown Date");
            }
        } else {
            result["commander"] = QVariant("Unknown CMDR");
            result["date"] = QVariant("Unknown Date");
            result["bodycount"] = QVariant(0);
        }
    }
    result["id"] = QVariant("edsm");
    qInfo() << "Fetched" << result.size() << "from Spansh";
    // reply->deleteLater();
    QMetaObject::invokeMethod(m_manager, "targetLoadingBay",
                              Qt::QueuedConnection,
                              Q_ARG(QVariantMap, result));
}

void SpanshDBTask::mapTask()
{
    QEventLoop loop;
    QTimer timer;
    QNetworkAccessManager manager;
    timer.setSingleShot(true);

    QString endpoint = m_map1;
    QUrl url(endpoint);

    QUrlQuery query;
    query.addQueryItem("q", m_systemName);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Explorarium/1.0");
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    QNetworkReply *reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(10000);
    loop.exec();
    timer.stop();
    if (!reply->isFinished()) {
        reply->abort();
        return;
    }

    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QNetworkReply::NetworkError netError = reply->error();
    if (netError != QNetworkReply::NoError && httpStatus != 404) {
        qWarning() << "Network Error:" << reply->errorString();
        QMetaObject::invokeMethod(m_manager, "mapError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, "Spansh Lookup"),
                                  Q_ARG(QString, "Looking up data"),
                                  Q_ARG(QString, "Network Error."));
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QJsonObject obj = doc.object();
    int count = obj.value("count").toInt();
    if(count <= 0) {
        QMetaObject::invokeMethod(m_manager, "mapEmpty",
                                  Qt::QueuedConnection);
        return;
    }
    QJsonArray results = obj.value("results").toArray();
    qint64 id64 = 0;
    for(int i = 0; i < results.size(); i++) {
        QJsonObject result = results[i].toObject();
        if(result.value("type").toString() != "system") {
            continue;
        }

        QJsonObject record = result.value("record").toObject();
        if(record.value("name").toString().toLower() != m_systemName.toLower()) {
            continue;
        }
        id64 = record.value("id64").toInteger();
    }
    if(id64 == 0) {
        QMetaObject::invokeMethod(m_manager, "mapEmpty",
                                  Qt::QueuedConnection);
        return;
    }

    QEventLoop loop2;
    QTimer timer2;
    QNetworkAccessManager manager2;
    timer2.setSingleShot(true);

    QString endpoint2 = m_map2 + QString::number(id64);
    QUrl url2(endpoint2);

    QNetworkRequest request2(url2);
    request2.setRawHeader("User-Agent", "Explorarium/1.0");
    request2.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    QNetworkReply *reply2 = manager.get(request2);
    QObject::connect(reply2, &QNetworkReply::finished, &loop2, &QEventLoop::quit);
    QObject::connect(&timer2, &QTimer::timeout, &loop2, &QEventLoop::quit);
    timer2.start(10000);
    loop2.exec();
    timer2.stop();
    if (!reply2->isFinished()) {
        reply2->abort();
        return;
    }

    int httpStatus2 = reply2->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QNetworkReply::NetworkError netError2 = reply2->error();
    if (netError2 != QNetworkReply::NoError && httpStatus2 != 404) {
        qWarning() << "Network Error:" << reply2->errorString();
        QMetaObject::invokeMethod(m_manager, "mapError",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, "Spansh Lookup"),
                                  Q_ARG(QString, "Looking up data"),
                                  Q_ARG(QString, "Network Error."));
        return;
    }

    QByteArray responseData2 = reply2->readAll();
    QJsonDocument doc2 = QJsonDocument::fromJson(responseData2);
    QJsonObject package = doc2.object();

    if(package.isEmpty()) {
        QMetaObject::invokeMethod(m_manager, "mapEmpty",
                                  Qt::QueuedConnection);
        return;
    }

    QJsonObject system = package.value("system").toObject();
    QString systemName = system.value("name").toString();

    QJsonObject VirtualRoot;
    VirtualRoot["bodyId"] = -1;
    VirtualRoot["name"] = "System Anchor";
    VirtualRoot["type"] = "System Anchor";

    QMap<int, QJsonObject> BodyMap;
    BodyMap.insert(-1, VirtualRoot);
    QJsonArray bodies = system.value("bodies").toArray();

    for(int i = 0; i < bodies.size(); i++) {
        QJsonObject body = bodies[i].toObject();
        int bodyId = body.value("bodyId").toInt();
        BodyMap[bodyId] = body;
    }
    for(int i = 0; i < bodies.size(); i++) {
        QJsonObject body = bodies[i].toObject();
        int bodyId = body.value("bodyId").toInt();

        QJsonArray parents = body.value("parents").toArray();
        if(!parents.isEmpty()) {
            QJsonObject firstParent = parents.at(0).toObject();
            QString parentName = firstParent.keys().first();
            int parentId = firstParent.value(parentName).toInt();

            if(parentName.toLower() == "null") {
                if (!BodyMap.contains(parentId)) {
                    QJsonObject fakeBarycentre;
                    fakeBarycentre["bodyId"] = parentId;
                    fakeBarycentre["type"] = "Barycentre";
                    fakeBarycentre["name"] = systemName + " barycentre " + QString::number(parentId);
                    BodyMap[parentId] = fakeBarycentre;
                }

                if (parents.size() > 1) {
                    QJsonObject secondParentObj = parents.at(1).toObject();
                    QString secondParentName = secondParentObj.keys().first();
                    int secondParentId = secondParentObj.value(secondParentName).toInt();

                    if (!BodyMap.contains(secondParentId)) {
                        QJsonObject fakeGrandparent;
                        fakeGrandparent["bodyId"] = secondParentId;
                        fakeGrandparent["type"] = "Barycentre";
                        fakeGrandparent["name"] = systemName + " barycentre " + QString::number(secondParentId);
                        BodyMap[secondParentId] = fakeGrandparent;
                    }

                    QJsonArray newParents;
                    QJsonObject p;
                    p[secondParentName] = secondParentId;
                    newParents.append(p);

                    BodyMap[parentId]["parents"] = newParents;
                }
            }
        }
        if (!BodyMap.contains(bodyId)) {
            BodyMap[bodyId] = body;
        }
    }
    QMap<int, QJsonArray> BucketsMap;
    QMapIterator<int, QJsonObject> i(BodyMap);
    while (i.hasNext()) {
        i.next();
        int myId = i.key();
        if (myId == -1) continue;

        QJsonObject body = i.value();
        QJsonArray parents = body.value("parents").toArray();
        int parentId = -1;

        if (!parents.isEmpty()) {
            QJsonObject firstParent = parents.at(0).toObject();
            auto it = firstParent.constBegin();
            if (it != firstParent.constEnd()) {
                parentId = it.value().toInt();
            }
            if (!BodyMap.contains(parentId)) {
                parentId = -1;
            }
        }

        QJsonArray bucket = BucketsMap[parentId];
        bucket.append(body);
        BucketsMap[parentId] = bucket;
    }
    m_BodyMap.clear();
    m_BodyMap = BodyMap;
    QVariantMap rootNode = buildRecursive(-1, BucketsMap, 0);
    QVariantMap finalSystemPackage = system.toVariantMap();
    finalSystemPackage["bodies"] = rootNode;
    QMetaObject::invokeMethod(m_manager, "gotMapData",
                              Qt::QueuedConnection,
                              Q_ARG(QVariantMap, finalSystemPackage));
}

QVariantMap SpanshDBTask::buildRecursive(int currentId, QMap<int, QJsonArray> &BucketsMap, int currentTier)
{
    QJsonObject myData = m_BodyMap[currentId];
    QString originalType = myData.value("type").toString();

    QString superType;
    if (currentId == -1) superType = "Anchor";
    else if (currentTier == 1) superType = "Star";
    else if (currentTier == 2) superType = "Planet";
    else if (currentTier == 3) superType = "Moon";
    else if (currentTier >= 4) superType = "Nested Moon";
    else superType = "Unknown";

    myData["superType"] = superType;
    QVariantMap node = myData.toVariantMap();

    int defaultNextTier = currentTier;
    if (originalType != "Barycentre" && originalType != "System Anchor") {
        defaultNextTier++;
    } else if (currentId == -1) {
        defaultNextTier = 1;
    }

    QJsonArray myChildrenIDs = BucketsMap[currentId];
    QVariantList childrenList;

    if (originalType == "Barycentre") {
        if (myChildrenIDs.size() < 2) return QVariantMap();

        for(int i = 0; i < myChildrenIDs.size(); i++) {
            QJsonObject childObj = myChildrenIDs[i].toObject();
            int specificTier = (i >= 2) ? currentTier + 1 : currentTier;
            QVariantMap childNode = buildRecursive(childObj["bodyId"].toInt(), BucketsMap, specificTier);
            if (!childNode.isEmpty()) childrenList.append(childNode);
        }
    }
    else {
        for(int i = 0; i < myChildrenIDs.size(); i++) {
            QJsonObject childObj = myChildrenIDs[i].toObject();
            QVariantMap childNode = buildRecursive(childObj["bodyId"].toInt(), BucketsMap, defaultNextTier);
            if(!childNode.isEmpty()) childrenList.append(childNode);
        }
    }

    if (currentId == -1) {
        node["layoutType"] = "VerticalFlow";
    }
    else {
        bool isVertical = true;

        if (superType == "Star") {
            isVertical = false;
        }
        else if (superType == "Planet") {
            isVertical = true;
        }
        else if (superType == "Moon") {
            isVertical = false;
        }

        if (originalType == "Barycentre") {
            isVertical = !isVertical;
        }

        node["layoutType"] = isVertical ? "VerticalFlow" : "HorizontalFlow";
    }

    node["children"] = childrenList;
    return node;
}

