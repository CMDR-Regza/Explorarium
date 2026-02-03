#ifndef SPANSHPLOTTER_H
#define SPANSHPLOTTER_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <systemnode.h>

class SpanshPlotter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList route READ route NOTIFY routeChanged FINAL)
    Q_PROPERTY(QList<QObject*> nodesList READ nodesList NOTIFY nodesListChanged)
public:
    explicit SpanshPlotter(QObject *parent = nullptr);
    Q_INVOKABLE void GenerateRoute(QString source, QString dest, int cargo, int fuel, bool alr, bool neutron,
                                   bool injections, bool secondary, bool refuel, QString view, QJsonObject shipbuild);
    QVariantList route() const { return m_route; }
    Q_INVOKABLE void clearRoute();
    Q_INVOKABLE void GrabSystemData(QString systemName);
    QList<QObject*> nodesList() const { return m_flattenedList; }
signals:
    void generatedRoute();
    void routeChanged();
    void fatal(QString operation, QString title, QString error);
    void showWindow(QString message);
    void nodesListChanged();
public slots:
    void error(QString operation, QString title, QString error);
    void mapError(QString operation, QString title, QString error);
    void mapEmpty();
    void gotMapData(QVariantMap data);
    void loadingbaydataplease(QJsonArray data);
    void gotSpanshReply(QByteArray data);
    void gotTargetEvent(QString id64, QString systemName);
    void targetLoadingBay(QVariantMap data);
private:
    QList<QObject*> m_flattenedList;
    QString m_systemName;
    QJsonArray m_shipData;
    QVariantList m_route;
    bool m_spanshDone = false;
    QString m_spanshMessage = "";
    bool m_edsmDone = false;
    QString m_edsmMessage = "";

    void calculateDimensions(SystemNode* node);
    void assignCoordinates(SystemNode* node, double x, double y);
    bool shouldStackVertically(SystemNode* node);
    SystemNode* parseRecursive(const QVariantMap& data, SystemNode* parent);
    void flattenTree(SystemNode *node);
    void debug(SystemNode *node, int depth);

    void combine();
};

#endif // SPANSHPLOTTER_H
