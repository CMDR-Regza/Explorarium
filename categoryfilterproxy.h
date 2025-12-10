#ifndef CATEGORYFILTERPROXY_H
#define CATEGORYFILTERPROXY_H
#include <QSortFilterProxyModel>

class CategoryFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList selectedCategories READ selectedCategories WRITE setSelectedCategories NOTIFY selectedCategoriesChanged)
public:
    explicit CategoryFilterProxy(QObject *parent = nullptr);
    Q_INVOKABLE void toggleCategory(QString category);
    QStringList selectedCategories() const;
    void setSelectedCategories(const QStringList &categories);
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
signals:
    void selectedCategoriesChanged();
private:
    QStringList m_selectedCategories;
};

#endif // CATEGORYFILTERPROXY_H
