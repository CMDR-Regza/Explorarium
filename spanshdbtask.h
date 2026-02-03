#ifndef SPANSHDBTASK_H
#define SPANSHDBTASK_H

#include <QObject>
#include <QRunnable>
#include <QVariantMap>
#include <QMap>
#include "spanshplotter.h"

class SpanshDBTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    SpanshDBTask(SpanshPlotter *manager, QString id64, QString task, QString systemName);
    void run() override;
    void systemTask();
    void edsmTask();
    void mapTask();
signals:
    void taskReady(QVariantMap result);
    void taskError(QString title, QString context, QString details);
private:
    SpanshPlotter *m_manager;
    QString m_id64;
    QString m_task;
    QString m_systemName;
    QVariantMap buildRecursive(int currentId, QMap<int, QJsonArray> &BucketsMap, int currentTier);
    const QString m_systemLink = "https://spansh.co.uk/api/system/";
    const QString m_edsmLink = "https://www.edsm.net/api-system-v1/bodies";
    const QString m_map1 = "https://spansh.co.uk/api/search";
    const QString m_map2 = "https://spansh.co.uk/api/dump/";
    QMap<int, QJsonObject> m_BodyMap;
};

#endif // SPANSHDBTASK_H
