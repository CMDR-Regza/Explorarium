#include "spanshplotter.h"
#include "spanshdbtask.h"
#include <QDebug>
#include "routetask.h"
#include <QThreadPool>
#include <QJsonObject>
#include <QDate>

SpanshPlotter::SpanshPlotter(QObject *parent)
    : QObject{parent}
{
    // GrabSystemData("Dryau Scraa AA-A h616");
}

void SpanshPlotter::GenerateRoute(QString source, QString dest, int cargo, int fuel, bool alr,
                                  bool neutron, bool injections, bool secondary, bool refuel, QString view, QJsonObject shipbuild)
{
    view = view.toLower().replace(" ", "-");
    qInfo() << "Generating route";
    QVariantMap params;
    params["cargo"] = cargo;
    params["fuel"] = fuel;
    params["alr"] = alr;
    params["neutron"] = neutron;
    params["injections"] = injections;
    params["secondary"] = secondary;
    params["refuel"] = refuel;
    params["view"] = view;

    RouteTask *task = new RouteTask(this,
                                    source,
                                    dest,
                                    shipbuild,
                                    params, m_shipData);
    QThreadPool::globalInstance()->start(task);
}

void SpanshPlotter::clearRoute()
{
    m_route.clear();
    emit routeChanged();
}

void SpanshPlotter::GrabSystemData(QString systemName)
{
    if(systemName.isEmpty()) return;
    qInfo() << "Asking spansh";
    SpanshDBTask *task = new SpanshDBTask(this, "", "systemMap", systemName);
    QThreadPool::globalInstance()->start(task);
}

void SpanshPlotter::error(QString operation, QString title, QString error)
{
    qInfo() << operation << title << error;
    emit fatal(operation, title, error);
}

void SpanshPlotter::mapError(QString operation, QString title, QString error)
{

}

void SpanshPlotter::mapEmpty()
{

}

void SpanshPlotter::gotMapData(QVariantMap data)
{
    qInfo() << "got map data";

    qDeleteAll(m_flattenedList);
    m_flattenedList.clear();

    QVariantMap rootMap = data["bodies"].toMap();
    if(rootMap.isEmpty()) {
        qWarning() << "Map is empty!";
        return;
    }

    SystemNode *rootNode = parseRecursive(rootMap, nullptr);
    calculateDimensions(rootNode);
    assignCoordinates(rootNode, 10000, 10000);
    if (rootNode->m_name == "System Anchor") {
        for (int i = 0; i < rootNode->m_children.size(); i++) {
            SystemNode *child = rootNode->m_children[i];
            flattenTree(child);
            debug(child, 0);
        }
    } else {
        flattenTree(rootNode);
        debug(rootNode, 0);
    }
    emit nodesListChanged();
    qInfo() << "Map generated with" << m_flattenedList.size() << "nodes.";
}

void SpanshPlotter::loadingbaydataplease(QJsonArray data)
{
    qInfo() << "Got yummy data thx plssss hehehehe hshahhahe ezezeze LETS GOOO";
    m_shipData = data;
}

void SpanshPlotter::gotSpanshReply(QByteArray data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "Invalid JSON:" << err.errorString();
        return;
    }

    QJsonObject root = doc.object();
    QJsonObject result = root["result"].toObject();
    QJsonArray jumps = result["jumps"].toArray();
    QVariantList model;
    for(int i = 0; i < jumps.size(); i++) {
        QJsonObject system = jumps[i].toObject();
        QVariantMap map = system.toVariantMap();
        model.append(map);
    }
    m_route = model;
    qInfo() << "Plot Complete fully!!";
    emit routeChanged();
    emit generatedRoute();
}

void SpanshPlotter::gotTargetEvent(QString id64, QString systemName)
{
    qInfo() << "Signals and slots okay";
    if(id64.isEmpty() || id64.isNull()) return;
    m_systemName = systemName;
    qInfo() << "Asking spansh";

    SpanshDBTask *task = new SpanshDBTask(this,
                                          id64, "system" , "");
    QThreadPool::globalInstance()->start(task);

    SpanshDBTask *othertask = new SpanshDBTask(this,
                                               "", "edsm" , m_systemName);
    QThreadPool::globalInstance()->start(othertask);
}

void SpanshPlotter::targetLoadingBay(QVariantMap data)
{
    if(data.isEmpty()) return;
    if(data["id"].toString().toLower() == "spansh") {
        m_spanshDone = true;
        if(data.contains("error")) {
            QString errorCode = data["error"].toString();
            if(errorCode.toLower().contains("could not find")) {
                m_spanshMessage = QString("%1 is not in Spansh").arg(m_systemName);
                if(m_spanshDone && m_edsmDone) {
                    combine();
                }
                return;
            }
        }
        QString name = data["name"].toString();
        QString rawTimestamp = data["updated_at"].toString();
        QString full_body_count = data["full_body_count"].toString();
        int body_count = data["body_count"].toInt();

        QDateTime dt = QDateTime::fromString(rawTimestamp, "yyyy-MM-dd HH:mm:ss+z");
        QString timestamp = "Unknown Date";
        if(dt.isValid()) {
            timestamp = dt.date().toString("yyyy-MM-dd");
        } else {
            if(rawTimestamp.toLower() != "unknown date") {
                timestamp = rawTimestamp.left(10);
            }
        }
        QString message = "";
        if(full_body_count.toInt() != body_count) {
            message = QString("%1 is in Spansh but incomplete [%2/%3] (%4)")
            .arg(name)
                .arg(body_count)
                .arg(full_body_count, timestamp);
            qInfo() << message;
        } else if(full_body_count.toInt() == body_count) {
            message = QString("%1 is in Spansh and complete [%2/%3] (%4)")
            .arg(name)
                .arg(body_count)
                .arg(full_body_count, timestamp);
            qInfo() << message;
        }
        m_spanshMessage = message;
        if(m_spanshDone && m_edsmDone) {
            combine();
            return;
        }
    } else if(data["id"].toString().toLower() == "edsm") {
        m_edsmDone = true;
        if(data.contains("error")) {
            QString errorCode = data["error"].toString();
            if(errorCode.toLower().contains("empty")) {
                m_edsmMessage = QString("%1 is not in EDSM").arg(m_systemName);
                if(m_spanshDone && m_edsmDone) {
                    combine();
                }
                return;
            }
        }

        QString name = data["name"].toString();
        QString fullbodycount = data["fullbodycount"].toString();
        int bodycount = data["bodycount"].toInt();
        QString commander = data["commander"].toString();
        QString rawTimestamp = data["date"].toString();

        QDateTime dt = QDateTime::fromString(rawTimestamp, "yyyy-MM-dd HH:mm:ss+z");
        QString timestamp = "Unknown Date";
        if(dt.isValid()) {
            timestamp = dt.date().toString("yyyy-MM-dd");
        } else {
            if(rawTimestamp.toLower() != "unknown date") {
                timestamp = rawTimestamp.left(10);
            }
        }
        QString message = "";
        if(fullbodycount.toInt() != bodycount) {
            message = QString("%1 is in EDSM but incomplete [%2/%3] (%4, %5)")
            .arg(name)
                .arg(bodycount)
                .arg(fullbodycount, timestamp, commander);
            qInfo() << message;
        } else if(fullbodycount.toInt() == bodycount) {
            message = QString("%1 is in EDSM and complete [%2/%3] (%4, %5)")
            .arg(name)
                .arg(bodycount)
                .arg(fullbodycount, timestamp, commander);
            qInfo() << message;
        }
        m_edsmMessage = message;
        if(m_spanshDone && m_edsmDone) {
            combine();
            return;
        }
    }
}

void SpanshPlotter::calculateDimensions(SystemNode *node)
{
    double spacing = 0;

    if (node->m_children.isEmpty()) {
        node->m_width = 400;
        node->m_height = 400;
        return;
    }

    for (int i = 0; i < node->m_children.size(); i++) {
        calculateDimensions(node->m_children[i]);
    }

    bool isVertical = shouldStackVertically(node);
    if (node->m_children.size() >= 2 && node->m_type == "Barycentre" && !isVertical) {
        SystemNode* left = node->m_children[0];
        SystemNode* right = node->m_children[1];

        double pairW = left->m_width + spacing + right->m_width;
        double pairH = qMax(left->m_height, right->m_height);

        double satsH = 0;
        double maxSatW = 0;
        for(int i=2; i<node->m_children.size(); i++) {
            satsH += node->m_children[i]->m_height + spacing;
            maxSatW = qMax(maxSatW, node->m_children[i]->m_width);
        }

        node->m_width = qMax(pairW, maxSatW);
        node->m_height = pairH + spacing + satsH;
    }
    else if (node->m_children.size() >= 2 && node->m_type == "Barycentre" && isVertical) {
        SystemNode* top = node->m_children[0];
        SystemNode* bottom = node->m_children[1];


        double pairH = top->m_height + spacing + bottom->m_height;
        double pairW = qMax(top->m_width, bottom->m_width);

        double satsW = 0;
        double maxSatH = 0;
        for(int i=2; i<node->m_children.size(); i++) {
            satsW += node->m_children[i]->m_width + spacing;
            maxSatH = qMax(maxSatH, node->m_children[i]->m_height);
        }

        node->m_width = pairW + spacing + satsW;
        node->m_height = qMax(pairH, maxSatH);
    }
    else {
        double dimMain = 400;
        double dimCross = 400;

        for (int i = 0; i < node->m_children.size(); i++) {
            auto *child = node->m_children[i];
            if (isVertical) {
                dimMain += child->m_height + spacing;
                dimCross = qMax(dimCross, child->m_width);
            } else {
                dimMain += child->m_width + spacing;
                dimCross = qMax(dimCross, child->m_height);
            }
        }
        if (isVertical) { node->m_height = dimMain; node->m_width = dimCross; }
        else { node->m_width = dimMain; node->m_height = dimCross; }
    }
}

void SpanshPlotter::assignCoordinates(SystemNode *node, double x, double y)
{
    node->m_x = x;
    node->m_y = y;

    if (node->m_children.isEmpty()) return;
    double spacing = 0;

    if (node->m_children.size() >= 2 && node->m_type == "Barycentre" && node->m_layoutType == "HorizontalFlow") {
        SystemNode* left = node->m_children[0];
        SystemNode* right = node->m_children[1];

        double pairHeight = qMax(left->m_height, right->m_height);
        double leftEdge = x - (left->m_width / 2.0);

        node->m_x = leftEdge + (node->m_width / 2.0);
        assignCoordinates(left, x, y);

        double rightX = x + (left->m_width / 2.0) + spacing + (right->m_width / 2.0);
        assignCoordinates(right, rightX, y);

        double pairCenterX = (x + rightX) / 2.0;

        double startSatY = y + (pairHeight / 2.0) + spacing;
        double currentSatY = startSatY;

        for (int i = 2; i < node->m_children.size(); i++) {
            SystemNode* sat = node->m_children[i];
            assignCoordinates(sat, pairCenterX, currentSatY);
            currentSatY += sat->m_height + spacing;
        }
    }
    else if (node->m_children.size() >= 2 && node->m_type == "Barycentre" && node->m_layoutType == "VerticalFlow") {
        SystemNode* top = node->m_children[0];
        SystemNode* bottom = node->m_children[1];

        double boxTop = y - (node->m_height / 2.0);
        double topEdge = boxTop - (top->m_height / 2.0);
        double largestNodeSize = 400;
        // for(int i = 0; i < node->m_children.size(); i++) {
        //     SystemNode *child = node->m_children[i];
        //     // do some stuff to grab node size or whatever. Planet size aswell.
        // }

        node->m_y = topEdge + (largestNodeSize / 2.0);

        assignCoordinates(top, x, topEdge);

        double barycentreLine = node->m_y;

        double bottomY = barycentreLine + node->m_height - (bottom->m_height / 2.0);
        assignCoordinates(bottom, x, bottomY);

        double startSatX = x + qMax(top->m_width, bottom->m_width) + spacing; // USE PLANET SIZE TOO!
        double currentSatX = startSatX;

        for (int i = 2; i < node->m_children.size(); i++) {
            SystemNode* sat = node->m_children[i];
            assignCoordinates(sat, currentSatX, barycentreLine);
            currentSatX += sat->m_width + spacing;
        }
    }
    else {
        bool vertical = (node->m_layoutType == "VerticalFlow");
        if (node->m_layoutType.isEmpty()) vertical = true;

        double currentX = x;
        double currentY = y;

        double parentSize = 400; // must change this to account for planet size.

        if (vertical) currentY = currentY + (parentSize / 2.0) + spacing;
        else currentX = currentX + (parentSize / 2.0) + spacing;

        for (int i = 0; i < node->m_children.size(); i++) {
            auto *child = node->m_children[i];

            qInfo() << "Placing child:" << child->m_name << "at Y:" << currentY << "Height:" << child->m_height;

            if (vertical) {
                double childCenterY = currentY + (child->m_height / 2.0);
                assignCoordinates(child, currentX, childCenterY);
                currentY += child->m_height + spacing;
            }
            else {
                double childCenterX = currentX + (child->m_width / 2.0);
                assignCoordinates(child, childCenterX, currentY);
                currentX += child->m_width + spacing;
            }
        }
    }
}


SystemNode* SpanshPlotter::parseRecursive(const QVariantMap& data, SystemNode* parent)
{
    SystemNode* node = new SystemNode(this);

    node->m_name = data["name"].toString();
    node->m_type = data["type"].toString();
    node->m_superType = data["superType"].toString();
    node->m_layoutType = data["layoutType"].toString();
    node->m_parentNode = parent;

    QVariantList childrenList = data["children"].toList();
    for (int i = 0; i < childrenList.size(); i++) {
        const QVariant &childVar = childrenList[i];
        SystemNode* childNode = parseRecursive(childVar.toMap(), node);
        node->m_children.append(childNode);
    }

    return node;
}

void SpanshPlotter::flattenTree(SystemNode *node)
{
    m_flattenedList.append(node);
    for (int i = 0; i < node->m_children.size(); i++) {
        auto* child = node->m_children[i];
        flattenTree(child);
    }
}

void SpanshPlotter::debug(SystemNode *node, int depth)
{
    QString indent = QString(" ").repeated(depth * 4);

    qDebug().noquote() << indent
                       << "[" << node->m_name << "]"
                       << "Type:" << node->m_type
                       << "SuperType:" << node->m_superType
                       << "x,y:" << node->xPos() << " " << node->yPos()
                       << "-> Layout:" << node->layoutType() << "width, height:" << node->m_width << " " << node->m_height;

    for (int i = 0; i < node->m_children.size(); i++) {
        auto *child = node->m_children[i];
        debug(child, depth + 1);
    }
}

bool SpanshPlotter::shouldStackVertically(SystemNode *node)
{
    if (node->m_layoutType == "VerticalFlow") return true;
    if (node->m_layoutType == "HorizontalFlow") return false;

    return true;
}

void SpanshPlotter::combine()
{
    m_spanshDone = false;
    m_edsmDone = false;

    QStringList output;
    if (!m_spanshMessage.isEmpty()) output << m_spanshMessage;
    if (!m_edsmMessage.isEmpty()) output << m_edsmMessage;

    if (!output.isEmpty()) {
        emit showWindow(output.join(" | "));
    }

    m_spanshMessage.clear();
    m_edsmMessage.clear();
}
