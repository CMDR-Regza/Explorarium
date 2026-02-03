#ifndef SYSTEMNODE_H
#define SYSTEMNODE_H

#include <QObject>

class SystemNode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double xPos READ xPos CONSTANT)
    Q_PROPERTY(double yPos READ yPos CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString type READ type CONSTANT)

    Q_PROPERTY(double parentX READ parentX CONSTANT)
    Q_PROPERTY(double parentY READ parentY CONSTANT)
    Q_PROPERTY(bool hasParent READ hasParent CONSTANT)
    Q_PROPERTY(QString layoutType READ layoutType CONSTANT)
public:
    explicit SystemNode(QObject *parent = nullptr);
    QString m_name;
    QString m_type;
    QString m_superType;
    QString m_layoutType;

    double m_x = 0;
    double m_y = 0;
    double m_width = 0;
    double m_height = 0;

    SystemNode* m_parentNode = nullptr;
    QVector<SystemNode*> m_children;

    double xPos() const { return m_x; }
    double yPos() const { return m_y; }
    QString name() const { return m_name; }
    QString type() const { return m_type; }
    QString layoutType() const { return m_layoutType; }

    bool hasParent() const { return m_parentNode != nullptr; }
    double parentX() const { return m_parentNode ? m_parentNode->m_x : 0; }
    double parentY() const { return m_parentNode ? m_parentNode->m_y : 0; }
};

#endif // SYSTEMNODE_H
