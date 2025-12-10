// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QSslConfiguration>
#include <QtGlobal>

#include "autogen/environment.h"
#include "loadingscreenmanager.h"
#include "supabaseclient.h"
#include "journalmanager.h"
#include "categoryfilterproxy.h"

int main(int argc, char *argv[])
{
    set_qt_environment();
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/qt/qml/ExplorariumContent/images/logo.png"));
    app.setOrganizationName("Explorarium");
    app.setApplicationName("ExplorariumApp");
    app.setApplicationVersion("0.0.1-Alpha");

    QQmlApplicationEngine engine;
    LoadingScreenManager *manager = new LoadingScreenManager(&app);
    JournalManager *jmanager = new JournalManager(&app);
    SupabaseClient *smanager = new SupabaseClient(&app, jmanager);
    CategoryFilterProxy *pmanager = new CategoryFilterProxy(&app);
    pmanager->setSourceModel(smanager->systemsModel());

    QObject::connect(jmanager, &JournalManager::loadingComplete, manager,
                     &LoadingScreenManager::journalManagerComplete);
    QObject::connect(smanager, &SupabaseClient::supabaseClientComplete, manager,
                     &LoadingScreenManager::supabaseClientCompleted);

    const QUrl url("qrc:/qt/qml/ExplorariumContent/App.qml");
    QObject::connect(
                &engine, &QQmlApplicationEngine::objectCreated, &app,
                [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.rootContext()->setContextProperty("loadingScreenManager", manager);
    engine.rootContext()->setContextProperty("JournalManager", jmanager);
    engine.rootContext()->setContextProperty("SupabaseClient", smanager);
    engine.rootContext()->setContextProperty("applicationVersion", app.applicationVersion());
    engine.rootContext()->setContextProperty("CategoryProxy", pmanager);

    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.addImportPath(":/");
    engine.load(url);
    engine.load("qrc:/qt/qml/ExplorariumContent/LoadingScreen.qml");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
