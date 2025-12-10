#include "systemsmodel.h"
#include <QDebug>
#include <QByteArray>
#include <QHash>

SystemsModel::SystemsModel(QObject *parent)
    : QAbstractListModel{parent}
{
    qInfo() << this << "Constructed";
}

int SystemsModel::rowCount(const QModelIndex &parent) const
{
    return m_systems.size();
}

QVariant SystemsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_systems.size())
        return QVariant();

    QVariantMap system = m_systems[index.row()];
    switch(role) {
    case SystemNameRole: return system["system_name"];
    case CategoryRole: return system["category"];
    case DistanceRole: return system["distance"];
    case CategoryImageRole: return system["category_image"];
    case SystemDataRole: return system;
    default: return QVariant();
    }
}

QHash<int, QByteArray> SystemsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[SystemNameRole] = "system_name";
    roles[CategoryRole] = "category";
    roles[DistanceRole] = "distance";
    roles[CategoryImageRole] = "category_image";
    roles[SystemDataRole] = "systemData";
    return roles;
}

void SystemsModel::setSystemsData(QList<QVariantMap> systems)
{
    if(!systems.isEmpty()) {
        beginResetModel();
        m_systems = systems;
        endResetModel();
    }
}
