#include "supabaseclient.h"
#include "supabasetask.h"
#include "imgbbtask.h"
#include <QDebug>
#include <QThreadPool>
#include <QDateTime>
#include <QTimeZone>

SupabaseClient::SupabaseClient(QObject *parent, JournalManager *manager)
    : QObject{parent}
{
    qInfo() << this << "Constructed";
    m_systemsModel = new SystemsModel(this);
    m_updateTimer = new QTimer(this);
    m_categoryModel = new CategoryModel(this);
    m_manager = manager;
    connect(m_manager, &JournalManager::locationChanged, this, &SupabaseClient::MergeAndUpdateModel);
    m_updateTimer->setInterval(60000);
    connect(m_updateTimer, &QTimer::timeout, this, &SupabaseClient::onInterval);
    refresh();
}

void SupabaseClient::fetchAllSystems()
{
    qInfo() << "Fetching all systems from Supabase...";

    SupabaseTask *task = new SupabaseTask(this,
                                          SupabaseTask::FETCH_SYSTEMS,
                                          QVariantMap(),
                                          m_supabaseUrl,
                                          m_anonKey);
    QThreadPool::globalInstance()->start(task);
}

void SupabaseClient::claimSystem(QString systemName, QString cmdrName)
{
    qInfo() << "Uploading: " << systemName << " Using params: " << cmdrName << " To supabase...";
    QVariantMap params;
    params["system_name"] = systemName;
    params["cmdr_name"] = cmdrName;

    SupabaseTask *task = new SupabaseTask(this,
                                          SupabaseTask::CLAIM_SYSTEM,
                                          params,
                                          m_supabaseUrl,
                                          m_anonKey);
    QThreadPool::globalInstance()->start(task);
}

void SupabaseClient::unclaimSystem(QString systemName, QString cmdrName)
{
    qInfo() << "Unclaiming: " << systemName << " Using params: " << cmdrName << " To supabase...";
    QVariantMap params;
    params["system_name"] = systemName;
    params["cmdr_name"] = cmdrName;

    SupabaseTask *task = new SupabaseTask(this,
                                          SupabaseTask::UNCLAIM_SYSTEM,
                                          params,
                                          m_supabaseUrl,
                                          m_anonKey);
    QThreadPool::globalInstance()->start(task);
}

void SupabaseClient::addContribution(QString systemName, QString cmdrName, QString title, QString desc, QString imageUrl)
{
    qInfo() << "Adding contribution for:" << systemName;

    QVariantMap params;
    params["system_name"] = systemName;
    params["cmdr_name"] = cmdrName;
    params["title"] = title;
    params["description"] = desc;
    params["main_image_url"] = imageUrl;

    SupabaseTask *task = new SupabaseTask(this,
                                          SupabaseTask::ADD_CONTRIBUTION,
                                          params,
                                          m_supabaseUrl,
                                          m_anonKey);
    QThreadPool::globalInstance()->start(task);
}

void SupabaseClient::uploadScreenshot(QString systemName, QString cmdrName, QString imageUrl)
{
    qInfo() << "Starting ImgBB upload for:" << imageUrl;
    m_pendingSystem = systemName;
    m_pendingCmdr = cmdrName;

    // Create the task
    ImgBBTask *task = new ImgBBTask(this, imageUrl);

    // Connect the signals (The Bridge)
    connect(task, &ImgBBTask::UploadFinished, this, &SupabaseClient::onImgbbSuccess);
    connect(task, &ImgBBTask::UploadFailed, this, [=](QString err){
        onError("Uploading Screenshot", "Upload Failed", err);
    });

    QThreadPool::globalInstance()->start(task);
}

void SupabaseClient::fetchCategoryImages()
{
    qInfo() << "Fetching category images from Supabase...";

    SupabaseTask *task = new SupabaseTask(this,
                                          SupabaseTask::FETCH_CATEGORY_IMAGES,
                                          QVariantMap(),
                                          m_supabaseUrl,
                                          m_anonKey);
    QThreadPool::globalInstance()->start(task);
}

void SupabaseClient::fetchDbData()
{
    qInfo() << "Fetching database data from Supabase...";

    SupabaseTask *task = new SupabaseTask(this,
                                          SupabaseTask::FETCH_DB_METADATA,
                                          QVariantMap(),
                                          m_supabaseUrl,
                                          m_anonKey);
    QThreadPool::globalInstance()->start(task);
}

void SupabaseClient::fetchContributions()
{
    qInfo() << "Fetching contribution data from Supabase...";
    SupabaseTask *task = new SupabaseTask(this,
                                          SupabaseTask::FETCH_CONTRIBUTIONS,
                                          QVariantMap(),
                                          m_supabaseUrl,
                                          m_anonKey);
    QThreadPool::globalInstance()->start(task);
}

void SupabaseClient::saveSystemImage(QString systemName, QString cmdrName, QString imageUrl)
{
    qInfo() << "Manually saving image to DB:" << imageUrl;
    QVariantMap params;
    params["system_name"] = systemName;
    params["uploaded_by"] = cmdrName;
    params["image_url"] = imageUrl;

    SupabaseTask *task = new SupabaseTask(this,
                                          SupabaseTask::UPLOAD_IMAGE,
                                          params,
                                          m_supabaseUrl,
                                          m_anonKey);
    QThreadPool::globalInstance()->start(task);
}

void SupabaseClient::fetchSystemImages()
{
    qInfo() << "Fetching image data from Supabase...";
    SupabaseTask *task = new SupabaseTask(this,
                                          SupabaseTask::FETCH_SYSTEM_IMAGES,
                                          QVariantMap(),
                                          m_supabaseUrl,
                                          m_anonKey);
    QThreadPool::globalInstance()->start(task);
}

void SupabaseClient::removeScreenshot(QString systemName, QString cmdrName, QString imageUrl)
{
    qInfo() << "Preparing to remove image link:" << imageUrl;

    QVariantMap params;
    params["system_name"] = systemName;
    params["cmdr_name"] = cmdrName;
    params["image_url"] = imageUrl;

    SupabaseTask *task = new SupabaseTask(this,
                                          SupabaseTask::REMOVE_IMAGE,
                                          params,
                                          m_supabaseUrl,
                                          m_anonKey);
    QThreadPool::globalInstance()->start(task);
}

void SupabaseClient::refresh()
{
    m_catred = false;
    m_sysred = false;
    m_contred = false;
    m_imagred = false;

    fetchAllSystems();
    fetchCategoryImages();
    fetchDbData();
    fetchContributions();
    fetchSystemImages();
}

QVariantMap SupabaseClient::getSystem(QString systemName)
{
    return m_mergedCache.value(systemName, QVariantMap());
}

double SupabaseClient::calculateDistance(double x, double y, double z, double cmdrX, double cmdrY, double cmdrZ)
{
    double dx = x - cmdrX;
    double dy = y - cmdrY;
    double dz = z - cmdrZ;
    return qSqrt(dx*dx + dy*dy + dz*dz);
}

void SupabaseClient::onSystemsLoaded(QVariantList systems, QVariantList claims, QVariantList category)
{
    m_allSystems.clear();
    m_systemCategory.clear();
    m_systemBodyDetails.clear();

    QSet<QString> processedSystems;

    for(int i = 0; i < category.size(); i++) {
        QVariantMap t = category[i].toMap();
        QString sysName = t["system_name"].toString();
        QString tag = t["tag"].toString();

        if (!tag.isEmpty() && !m_systemCategory[sysName].contains(tag)) {
            m_systemCategory[sysName].append(tag);
        }
    }

    QMap<QString, int> systemTagUsageCount;

    for(int i = 0; i < systems.size(); i++) {
        QVariantMap row = systems[i].toMap();
        QString name = row["system_name"].toString();

        QString tag;
        QVariant bodiesVar = row["bodies"];
        QString bodyText = "";

        if (!processedSystems.contains(name)) {
            m_allSystems.append(row);
            processedSystems.insert(name);
        }

        if (bodiesVar.userType() == QMetaType::QVariantList) {
            QVariantList list = bodiesVar.toList();

            for(int k = 0; k < list.size(); k++) {
                QVariantMap b = list[k].toMap();
                const QList<QString> keys = b.keys();

                for(int c = 0; c < b.size(); c++) {
                    QVariant key = keys[c];
                    QVariant value = b.value(key.toString());
                    QString line = "• " + key.toString() + " : " + value.toString() + "\n";
                    bodyText += line;
                }
            }
        } else {
            bodyText = bodiesVar.toString();
        }

        if (!bodyText.isEmpty()) {
            QVariantMap card;
            QString displayTag = "System Feature";
            if(m_systemCategory.contains(name)) {
                QStringList tags = m_systemCategory[name];
                int tagIndex = systemTagUsageCount.value(name, 0);
                if(!tags.isEmpty()) {
                    if (tagIndex < tags.size()) {
                        displayTag = tags[tagIndex];
                    } else {
                        displayTag = tags.last();
                    }
                }
            }
            card["tag"] = displayTag;
            card["body"] = bodyText.trimmed();

            QVariantList stack = m_systemBodyDetails[name].toList();
            stack.append(card);
            m_systemBodyDetails[name] = stack;
        }
    }

    QMap<QString, QString> claimsMap;
    for(int i = 0; i < claims.size(); i++) {
        QVariantMap c = claims[i].toMap();
        claimsMap[c["system_name"].toString()] = c["cmdr_name"].toString();
    }
    m_claimsMap = claimsMap;
    emit systemsLoaded();
    m_sysred = true;
    if (m_catred && m_sysred && m_contred && m_imagred) {
        MergeAndUpdateModel();
    }
}

void SupabaseClient::onCategoryLoaded(QVariantMap categoryImages)
{
    m_categoryImages = categoryImages;

    QList<QVariantMap> categoryList;
    for(auto it = categoryImages.begin(); it != categoryImages.end(); ++it) {
        QVariantMap item;
        item["category_name"] = it.key();
        item["category_image"] = it.value();
        categoryList.append(item);
    }
    m_categoryModel->setCategoryData(categoryList);

    qInfo() << "Category images loaded:" << m_categoryImages.size() << "categories";
    m_catred = true;
    if (m_catred && m_sysred && m_contred && m_imagred) {
        MergeAndUpdateModel();
    }
    emit categoryImagesChanged();
}

void SupabaseClient::MergeAndUpdateModel()
{
    m_mergedCache.clear();

    QList<QVariantMap> mergedSystems;
    for(int i = 0; i < m_allSystems.size(); i++) {
        QVariantMap system = m_allSystems[i];
        QString sysName = system["system_name"].toString();


        //category image


        system["category"] = m_systemCategory.value(sysName, QStringList());
        system["claimed_by"] = m_claimsMap.value(sysName, "");
        QStringList systemCategory = m_systemCategory.value(sysName, QStringList());
        if (!systemCategory.isEmpty()) {
            QString firstTag = systemCategory.first();
            system["category_image"] = m_categoryImages.value(firstTag, "images/recordsBg.png");
        } else {
            system["category_image"] = "images/recordsBg.png";
        }


        //contribs


        if(m_contributions.contains(sysName)) {
            QVariantMap contrib = m_contributions[sysName].toMap();

            QString contribTitle = contrib["title"].toString();
            if (contribTitle.isEmpty()) {
                system["title"] = sysName;
            } else {
                system["title"] = contribTitle;
            }

            QString contribDesc = contrib["description"].toString();
            if (contribDesc.isEmpty()) {
                system["description"] = "No description available.";
            } else {
                system["description"] = contribDesc;
            }

            QString contribImg = contrib["main_image_url"].toString();
            if (contribImg.isEmpty()) {
                system["main_image"] = system["category_image"];
            } else {
                system["main_image"] = contribImg;
            }

            QString cmdrname = contrib["cmdr_name"].toString();
            if(cmdrname.isEmpty()) {
                system["cmdr_name"] = "Unknown";
            } else {
                system["cmdr_name"] = cmdrname;
            }
        } else {
            system["title"] = sysName;
            system["description"] = "No description available for this system...";
            system["main_image"] = system["category_image"];
            system["cmdr_name"] = "Unknown";
        }


        // images


        QVariantList carouselList;
        QString mainImg = system["main_image"].toString();

        if (!mainImg.isEmpty()) {
            carouselList.append(mainImg);
        }

        if(m_systemImages.contains(sysName)) {
            QVariantList galleryList = m_systemImages[sysName].toList();

            for (int k = 0; k < galleryList.size(); k++) {
                QString imgUrl = galleryList[k].toString();
                if (imgUrl != mainImg) {
                    carouselList.append(imgUrl);
                }
            }
        }

        system["images"] = carouselList;

        //body details

        if (m_systemBodyDetails.contains(sysName)) {
            system["body_details"] = m_systemBodyDetails[sysName];
        } else {
            system["body_details"] = QVariantList();
        }

        QList<double> cmdrCoords = m_manager->coordinates();
        if (cmdrCoords.size() >= 3) {
            double sysX = system["x"].toDouble();
            double sysY = system["y"].toDouble();
            double sysZ = system["z"].toDouble();

            double distance = calculateDistance(sysX, sysY, sysZ,
                                                cmdrCoords[0], cmdrCoords[1], cmdrCoords[2]);

            system["distance"] = QString::number(distance, 'f', 1) + " LY";
        } else {
            system["distance"] = "Unknown";
        }
        m_mergedCache[sysName] = system;
        mergedSystems.append(system);
    }
    switch(m_sortMode) {
    case SortMode::SortByClosestDistance:
        std::sort(mergedSystems.begin(), mergedSystems.end(),
                  [](const QVariantMap &a, const QVariantMap &b) {
                      QString distA = a["distance"].toString();
                      QString distB = b["distance"].toString();

                      double numA = distA.split(" ")[0].toDouble();
                      double numB = distB.split(" ")[0].toDouble();

                      return numA < numB;
                  });
        break;

    case SortMode::SortByFurthestDistance:
        std::sort(mergedSystems.begin(), mergedSystems.end(),
                  [](const QVariantMap &a, const QVariantMap &b) {
                      QString distA = a["distance"].toString();
                      QString distB = b["distance"].toString();

                      double numA = distA.split(" ")[0].toDouble();
                      double numB = distB.split(" ")[0].toDouble();

                      return numA > numB;
                  });
        break;
    }

    m_systemsModel->setSystemsData(mergedSystems);
    emit systemsLoaded();
    if(!m_initialLoadComplete) {
        m_initialLoadComplete = true;
        emit supabaseClientComplete();
    }
}

void SupabaseClient::onClaimSuccess(QString systemName, QString cmdrName)
{
    qInfo() << "Claim is successful! " << systemName;
    emit claimUpdated(true, QString("claimed_" + systemName));
    m_claimsMap[systemName] = cmdrName;
    MergeAndUpdateModel();
}

void SupabaseClient::onContributionsLoaded(QVariantList contributions)
{
    m_contributions.clear();

    for(int i = 0; i < contributions.size(); i++) {
        QVariantMap row = contributions[i].toMap();
        QString sysName = row["system_name"].toString();

        m_contributions.insert(sysName, row);
    }

    qInfo() << "Mapped" << m_contributions.size() << "contributions";
    m_contred = true;
    if (m_catred && m_sysred && m_contred && m_imagred) {
        MergeAndUpdateModel();
    }
}

void SupabaseClient::onImagesLoaded(QVariantList images)
{
    m_systemImages.clear();

    for(int i = 0; i < images.size(); i++) {
        QVariantMap row = images[i].toMap();
        QString sysName = row["system_name"].toString();
        QString url = row["image_url"].toString();

        QVariantList currentList = m_systemImages[sysName].toList();
        currentList.append(url);
        m_systemImages[sysName] = currentList;
    }

    qInfo() << "Mapped images for" << m_systemImages.size() << "systems";
    m_imagred = true;
    if (m_catred && m_sysred && m_contred && m_imagred) {
        MergeAndUpdateModel();
    }
}

void SupabaseClient::onUnclaimSuccess(QString systemName)
{
    qInfo() << "Unclaim is successful! " << systemName;
    emit claimUpdated(true, QString("unclaimed_" + systemName));
    m_claimsMap.remove(systemName);
    MergeAndUpdateModel();
}

void SupabaseClient::onImageRemoved(QString url)
{
    qInfo() << "Image remove is successful!";
    for (auto it = m_systemImages.begin(); it != m_systemImages.end(); ++it) {
        QVariantList imageList = it.value().toList();
        if (imageList.contains(url)) {
            imageList.removeOne(url);
            m_systemImages.insert(it.key(), imageList);
            MergeAndUpdateModel();
            return;
        }
    }
}

void SupabaseClient::onInterval()
{
    QDateTime dbTime = QDateTime::fromString(m_lastupdated, Qt::ISODate);
    dbTime.setTimeZone(QTimeZone::utc());
    dbTime = dbTime.toLocalTime();
    QDateTime now = QDateTime::currentDateTime();
    qint64 seconds = dbTime.secsTo(now);

    if (seconds < 60) {
        m_time = QString::number(seconds) + " seconds ago";
    } else if (seconds < 3600) {
        int minutes = seconds / 60;
        m_time = QString::number(minutes) + " minutes ago";
    } else if (seconds < 86400) {
        int hours = seconds / 3600;
        m_time = QString::number(hours) + " hours ago";
    } else {
        int days = seconds / 86400;
        m_time = QString::number(days) + " days ago";
    }
    emit TimeChanged();
    if(m_isnew == false) {
        fetchDbData();
    } else {
        m_isnew = !m_isnew;
        return;
    }
}

void SupabaseClient::setSortMode(SupabaseClient::SortMode mode)
{
    if(mode != m_sortMode) {
        m_sortMode = mode;
        qInfo() << "Changed m_sortMode to " << mode;
        MergeAndUpdateModel();
        emit SortModeChanged();
    }
}

void SupabaseClient::onDbLoaded(QVariantList data)
{
    if(!data.isEmpty()) {
        m_newthisweek = data[0].toInt();
        QString lastupdated = data[1].toString();
        if (!m_lastupdated.isEmpty() && m_lastupdated != lastupdated) {
            refresh();
        }
        if(m_lastupdated.isEmpty()) {
            m_lastupdated = lastupdated;
            onInterval();
            m_updateTimer->start();
        }
        m_lastupdated = lastupdated;
        emit dbDataChanged();
    }
}

void SupabaseClient::onError(QString title, QString operation, QString error)
{
    qWarning() << "Error during: " << operation << " With error: " << error;
    emit errorOccurred(error, title, operation);
}

void SupabaseClient::onContributionsAdded(QVariantMap data)
{
    QString sysName = data["system_name"].toString();
    qInfo() << "Contribution added successfully for" << sysName;

    m_contributions.insert(sysName, data);
    MergeAndUpdateModel();
    emit contributionUpdated(sysName);
}

void SupabaseClient::onImgbbSuccess(QString link)
{
    qInfo() << "Image hosted successfully at:" << link << "Saving link to Supabase.";
    emit screenshotReady(link);
    m_pendingSystem.clear();
    m_pendingCmdr.clear();
}

void SupabaseClient::onImageSaved(QVariantMap confirmedData)
{
    QString sysName = confirmedData["system_name"].toString();
    QString newUrl = confirmedData["image_url"].toString();

    qInfo() << "Image link saved to DB. Updating local cache for:" << sysName;

    QVariantList currentList = m_systemImages.value(sysName).toList();
    currentList.append(newUrl);
    m_systemImages.insert(sysName, currentList);
    MergeAndUpdateModel();
    emit imagesChanged(newUrl, sysName);
}
