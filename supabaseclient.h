#ifndef SUPABASECLIENT_H
#define SUPABASECLIENT_H

#include <QObject>
#include "categorymodel.h"
#include "systemsmodel.h"
#include "journalmanager.h"
#include <QTimer>

class SupabaseClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SystemsModel *systemsModel READ systemsModel CONSTANT)
    Q_PROPERTY(CategoryModel *categoryModel READ categoryModel CONSTANT)
    Q_PROPERTY(QVariantMap categoryImages READ categoryImages NOTIFY categoryImagesChanged)
    Q_PROPERTY(int NewThisWeek READ newthisweek NOTIFY dbDataChanged FINAL)
    Q_PROPERTY(QString lastUpdated READ lastUpdated NOTIFY TimeChanged FINAL)
    Q_PROPERTY(SortMode sortMode READ sort WRITE setSortMode NOTIFY SortModeChanged FINAL)
    Q_PROPERTY(int totalSystems READ totalSystems NOTIFY systemsLoaded FINAL)
public:
    enum SortMode {
        SortByClosestDistance,
        SortByFurthestDistance // more later
    };
    Q_ENUM(SortMode)
    explicit SupabaseClient(QObject *parent = nullptr, JournalManager *manager = nullptr);
    Q_INVOKABLE void fetchAllSystems();
    Q_INVOKABLE void claimSystem(QString systemName, QString cmdrName);
    Q_INVOKABLE void unclaimSystem(QString systemName, QString cmdrName);
    Q_INVOKABLE void addContribution(QString systemName, QString cmdrName, QString title,
                                     QString desc, QString imageUrl);
    Q_INVOKABLE void uploadScreenshot(QString systemName, QString cmdrName, QString imageUrl);
    Q_INVOKABLE void fetchCategoryImages();
    Q_INVOKABLE void fetchDbData();
    Q_INVOKABLE void fetchContributions();
    Q_INVOKABLE void saveSystemImage(QString systemName, QString cmdrName, QString imageUrl);
    Q_INVOKABLE void fetchSystemImages();
    Q_INVOKABLE void removeScreenshot(QString systemName, QString cmdrName, QString imageUrl);
    Q_INVOKABLE void refresh();
    Q_INVOKABLE QVariantMap getSystem(QString systemName);
    SystemsModel* systemsModel() const { return m_systemsModel; }
    CategoryModel* categoryModel() const { return m_categoryModel; }
    int newthisweek() const { return m_newthisweek; }
    int totalSystems() const { return m_allSystems.size(); }
    QVariantMap categoryImages() const { return m_categoryImages; }
    SortMode sort() const { return m_sortMode; }
    QString lastUpdated() const { return m_time; }

    Q_INVOKABLE double calculateDistance(double x, double y, double z, double cmdrX, double cmdrY, double cmdrZ);
public slots:
    void onSystemsLoaded(QVariantList systems, QVariantList claims, QVariantList category);
    void onCategoryLoaded(QVariantMap categoryImages);
    void onClaimSuccess(QString systemName, QString cmdrName);
    void onContributionsLoaded(QVariantList contributions);
    void onImagesLoaded(QVariantList images);
    void onUnclaimSuccess(QString systemName);
    void onImageRemoved(QString url);
    void onInterval();
    void setSortMode(SupabaseClient::SortMode mode);
    void MergeAndUpdateModel();
    void onDbLoaded(QVariantList data);
    void onError(QString operation, QString title, QString error);
    void onContributionsAdded(QVariantMap data);
    void onImgbbSuccess(QString link);
    void onImageSaved(QVariantMap confirmedData);
signals:
    void categoryImagesChanged();
    void errorOccurred(QString error, QString title, QString operation);
    void dbDataChanged();
    void TimeChanged();
    void SortModeChanged();
    void claimUpdated(bool success, QString message);
    void systemsLoaded();
    void supabaseClientComplete();
    void contributionUpdated(QString systemName);
    void screenshotReady(QString url);
    void imagesChanged(QString url, QString system);
private:
    SortMode m_sortMode = SortMode::SortByClosestDistance; // default value
    JournalManager *m_manager;
    QTimer *m_updateTimer;
    SystemsModel *m_systemsModel;
    CategoryModel *m_categoryModel;
    bool m_initialLoadComplete = false;
    const QString m_supabaseUrl = "https://oduelomkzdlxvenwjeui.supabase.co";
    const QString m_anonKey = "sb_publishable_wRxCE9xKgOLmkVx5cpR0Tw_w5tuc3yq";
    QVariantMap m_categoryImages;
    QVariantMap m_systemImages;
    QVariantMap m_contributions;
    QHash<QString, QVariantMap> m_mergedCache;
    QString m_pendingSystem;
    QString m_pendingCmdr;
    QList<QVariantMap> m_allSystems;
    QList<QVariantMap> m_allClaims;
    QMap<QString, QVariantList> m_systemBodyDetails;
    QMap<QString, QStringList> m_systemCategory;
    int m_newthisweek = 0;
    QString m_lastupdated = "";
    QString m_time = "";
    bool m_sysred = false;
    bool m_catred = false;
    bool m_contred = false;
    bool m_imagred = false;
    bool m_isnew = true;
    QMap<QString, QString> m_claimsMap;
};

#endif // SUPABASECLIENT_H
