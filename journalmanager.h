#ifndef JOURNALMANAGER_H
#define JOURNALMANAGER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QDir>
#include <QTimer>

class JournalManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString cmdrName READ commanderName NOTIFY CmdrChanged FINAL)
    Q_PROPERTY(QString location READ location NOTIFY locationChanged FINAL)
public:
    explicit JournalManager(QObject *parent = nullptr);
    void contactJournalData();
    QString findLatestJournal();
    QString commanderName();
    QString location();
    QList<double> coordinates();
public slots:
    void onNewFile();
    void onJournalUpdate();
    void onJournalDataLoaded(const QVariantMap &data);
signals:
    void loadingComplete();
    void CmdrChanged();
    void locationChanged();
private:
    QString m_commanderName;
    QString m_location;
    QList<double> m_coordinates;
    QFileSystemWatcher *m_watcher;
    QString m_currentJournalFile;
    qint64 m_fileposition;
    const QString m_journalPath = QDir::homePath() + "/Saved Games/Frontier Developments/Elite Dangerous/";
    QTimer *m_debounceTimer;
};

#endif // JOURNALMANAGER_H
