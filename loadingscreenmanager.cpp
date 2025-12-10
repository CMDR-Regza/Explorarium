#include "loadingscreenmanager.h"
#include <QDebug>
#include <QTimer>

LoadingScreenManager::LoadingScreenManager(QObject *parent)
    : QObject{parent}
{
    qInfo() << this << "Constructed";
}

void LoadingScreenManager::journalManagerComplete()
{
    this->m_done++;
    this->isDone();
}

void LoadingScreenManager::supabaseClientCompleted()
{
    this->m_done++;
    this->isDone();
}

void LoadingScreenManager::isDone()
{
    if(this->m_done == this->m_bayNum) {
        qInfo() << this << "Loading complete";
        emit loadApp();
    }
}
