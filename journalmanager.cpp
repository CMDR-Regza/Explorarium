#include "journalmanager.h"
#include "journaltask.h"
#include <QDebug>
#include <QThreadPool>
#include <QFileSystemWatcher>
#include <QDir>
#include <QVariantMap>
#include <QSettings>
#include <QApplication>
#include <QTimer>

JournalManager::JournalManager(QObject *parent)
    : QObject{parent}
{
    qInfo() << this << "Constructed";
    m_watcher = new QFileSystemWatcher(this);
    m_watcher->addPath(m_journalPath);
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &JournalManager::onNewFile);
    connect(m_watcher, &QFileSystemWatcher::fileChanged, this, &JournalManager::onJournalUpdate);

    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    m_commanderName = settings.value("cmdrName", "Unknown").toString();
    m_location = settings.value("location", "Unknown").toString();
    QVariantList coordsVar = settings.value("coordinates", QVariantList()).toList();
    qInfo() << coordsVar;
    m_coordinates.clear();
    for (int i = 0; i < coordsVar.size(); ++i) {
        m_coordinates.append(coordsVar[i].toDouble());
    }
    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(1000);
    connect(m_debounceTimer, &QTimer::timeout, this, &JournalManager::contactJournalData);

    onNewFile();
}

QString JournalManager::findLatestJournal()
{
    QDir dir(m_journalPath);
    QFileInfoList files = dir.entryInfoList(QStringList() << "Journal.*.log", QDir::Files, QDir::Time);
    if (!files.isEmpty()) {
        return files.first().absoluteFilePath();
    }
    return QString();
}

void JournalManager::onNewFile()
{
    QString oldfile = m_currentJournalFile;
    QString newfile = findLatestJournal();

    if(newfile != oldfile) {
        if (!oldfile.isEmpty()) {
            m_watcher->removePath(oldfile);
        }
        m_watcher->addPath(newfile);
        m_currentJournalFile = newfile;
        m_fileposition = 0;

        m_debounceTimer->start();
    }
}

void JournalManager::onJournalUpdate()
{
    m_debounceTimer->start();
}

void JournalManager::contactJournalData()
{
    JournalTask *task = new JournalTask(this, m_currentJournalFile, m_fileposition);
    QThreadPool::globalInstance()->start(task);
}

void JournalManager::onJournalDataLoaded(const QVariantMap &data)
{
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    bool hadCmdr = !m_commanderName.isEmpty();
    bool hadLocation = !m_location.isEmpty();
    if (data.contains("cmdrName")) {
        QString name = data["cmdrName"].toString();
        if (!name.isEmpty()) {
            m_commanderName = name;
            settings.setValue("cmdrName", name);
            qInfo() << "Retrieved name" << name;
            emit CmdrChanged();
            emit loadingComplete();
        }
    }

    if (data.contains("location")) {
        m_location = data["location"].toString();
        settings.setValue("location", m_location);
        m_coordinates.clear();
        m_coordinates.append(data["x"].toDouble());
        m_coordinates.append(data["y"].toDouble());
        m_coordinates.append(data["z"].toDouble());
        QVariantList coordsVar;
        for (int i = 0; i < m_coordinates.size(); ++i) {
            coordsVar.append(m_coordinates[i]);
        }
        settings.setValue("coordinates", coordsVar);
        qInfo() << "Retrieved location" << m_location << m_coordinates;
        emit locationChanged();
    }

    if(data.contains("pos")) {
        m_fileposition = data["pos"].toLongLong();
    }

    bool nowHasBoth = !m_commanderName.isEmpty() && !m_location.isEmpty();
    if (nowHasBoth && (!hadCmdr || !hadLocation)) {
        emit loadingComplete();
    }
}

QString JournalManager::commanderName()
{
    return m_commanderName;
}

QString JournalManager::location()
{
    return m_location;
}

QList<double> JournalManager::coordinates()
{
    return m_coordinates;
}


