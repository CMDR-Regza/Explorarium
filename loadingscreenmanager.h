#ifndef LOADINGSCREENMANAGER_H
#define LOADINGSCREENMANAGER_H

#include <QObject>

class LoadingScreenManager : public QObject
{
    Q_OBJECT
public:
    explicit LoadingScreenManager(QObject *parent = nullptr);
signals:
    void loadApp();
public slots:
    void journalManagerComplete();
    void supabaseClientCompleted();
private:
    void isDone();
    const int m_bayNum = 2;
    int m_done = 0;
};

#endif // LOADINGSCREENMANAGER_H
