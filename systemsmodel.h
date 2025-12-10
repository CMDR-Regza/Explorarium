#ifndef SYSTEMSMODEL_H
#define SYSTEMSMODEL_H

#include <QObject>
#include <QAbstractListModel>

class SystemsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SystemsModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
public slots:
    void setSystemsData(QList<QVariantMap> systems);
private:
    enum SystemRoles {
        SystemNameRole = Qt::UserRole + 1,
        CategoryRole,
        DistanceRole,
        CategoryImageRole,
        SystemDataRole
    };
    QList<QVariantMap> m_systems;
};

#endif // SYSTEMSMODEL_H
