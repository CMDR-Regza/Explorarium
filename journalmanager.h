#ifndef JOURNALMANAGER_H
#define JOURNALMANAGER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QDir>
#include <QTimer>
#include <QJsonArray>
#include <QJsonObject>

class JournalManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString cmdrName READ commanderName NOTIFY CmdrChanged FINAL)
    Q_PROPERTY(QString location READ location NOTIFY locationChanged FINAL)
    Q_PROPERTY(QList<double> coordinates READ coordinates NOTIFY locationChanged FINAL)
    Q_PROPERTY(QString journalPath READ journalPath WRITE setJournalPath NOTIFY journalPathChanged FINAL)
    Q_PROPERTY(QJsonObject shipbuild READ shipbuild NOTIFY shipBuildChanged FINAL)
public:
    explicit JournalManager(QObject *parent = nullptr);
    void contactJournalData();
    QString findLatestJournal();
    QString commanderName();
    QString location();
    QJsonObject shipbuild() const { return m_shipbuild; }
    Q_INVOKABLE QString journalPath() const { return m_journalPath; };
    Q_INVOKABLE void setJournalPath(QString path);
    QList<double> coordinates() { return m_coordinates; }
public slots:
    void onNewFile();
    void onJournalUpdate();
    void onJournalDataLoaded(const QVariantMap &data);
signals:
    void loadingComplete();
    void CmdrChanged();
    void journalPathChanged();
    void locationChanged();
    void shipBuildChanged();
    void targetEvent(QString id64, QString systemName);
private:
    QString m_commanderName;
    QString m_location;
    QJsonObject m_shipbuild;
    QList<double> m_coordinates;
    QFileSystemWatcher *m_watcher;
    QString m_currentJournalFile;
    qint64 m_fileposition;
    QString m_journalPath = QDir::homePath() + "/Saved Games/Frontier Developments/Elite Dangerous/";
    QTimer *m_debounceTimer;
    bool m_windowPopupFirst = false;
};

#endif // JOURNALMANAGER_H
