#include "journaltask.h"
#include <QDebug>
#include <QThread>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QJsonArray>

JournalTask::JournalTask(JournalManager *manager, QString currentFile, qint64 filePosition)
{
    qInfo() << this << "Constructed";
    m_manager = manager;
    m_currentFile = currentFile;
    m_filePosition = filePosition;
}

void JournalTask::run() {
    qInfo() << this << "Running on: " << QThread::currentThread();
    QVariantMap locationName = this->ReadJournalData();
    QMetaObject::invokeMethod(m_manager, "onJournalDataLoaded",
                                Qt::QueuedConnection, Q_ARG(QVariantMap, locationName));
}

QVariantMap JournalTask::ReadJournalData()
{
    QVariantMap result;
    QString latestFile = m_currentFile;
    QFile file(latestFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        file.seek(m_filePosition);
        QTextStream in(&file);
        QString fileContent = in.readAll();
        QStringList lines = fileContent.split("\n");

        for(int i = 0; i < lines.size(); ++i) {
            QString line = lines[i];
            if (line.isEmpty()) continue;
            QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8());
            if(!doc.isNull() && doc.isObject()) {
                QJsonObject obj = doc.object();
                if(obj["event"].toString() == "Commander") {
                    result["cmdrName"] = obj["Name"].toString();
                    qInfo() << "Found name!" << result["cmdrName"];
                    break;
                }
            }
        }

        for(int i = lines.size() - 1; i >= 0; --i) {
            QString line = lines[i];
            if (line.isEmpty()) continue;
            QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8());
            if(!doc.isNull() && doc.isObject()) {
                QJsonObject obj = doc.object();
                if(obj["event"].toString() == "FSDJump"
                    || obj["event"].toString() == "Location"
                    || obj["event"].toString() == "CarrierJump") {
                    result["location"] = obj["StarSystem"].toString();
                    QJsonArray pos = obj["StarPos"].toArray();
                    result["x"] = pos[0].toDouble();
                    result["y"] = pos[1].toDouble();
                    result["z"] = pos[2].toDouble();
                    qInfo() << "Found location!" << result["location"];
                    break;
                }
            }
        }

        result["pos"] = file.pos();
    }
    return result;
}

