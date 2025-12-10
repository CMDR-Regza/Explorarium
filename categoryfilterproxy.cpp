#include "categoryfilterproxy.h"

CategoryFilterProxy::CategoryFilterProxy(QObject *parent)
{
    qInfo() << this << "Constructed";
}

void CategoryFilterProxy::toggleCategory(QString category)
{
    if(category.isEmpty() || category.isNull()) {
        return;
    }

    if(m_selectedCategories.contains(category)) {
        m_selectedCategories.removeOne(category);
    } else {
        m_selectedCategories.append(category);
    }
    emit selectedCategoriesChanged();
    invalidateFilter();
}

QStringList CategoryFilterProxy::selectedCategories() const
{
    return m_selectedCategories;
}

void CategoryFilterProxy::setSelectedCategories(const QStringList &categories)
{
    if(categories.isEmpty()) return;
    m_selectedCategories = categories;
}

bool CategoryFilterProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if(m_selectedCategories.isEmpty()) {return true;}

    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    QVariant categoryData = sourceModel()->data(index, Qt::UserRole + 2);
    QStringList systemCategories = categoryData.toStringList();

    qDebug() << "Row" << source_row << "has categories:" << systemCategories;
    qDebug() << "Checking against selected:" << m_selectedCategories;

    for(int i = 0; i < m_selectedCategories.size(); i++) {
        if(systemCategories.contains(m_selectedCategories[i])) {
            return true;
        }
    }
    return false;
}
