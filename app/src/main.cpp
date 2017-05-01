#include <QStringList>
#include <QJsonObject>
//#include <QTranslator>
//#include <QLocale>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QCoreApplication>
#include <QDebug>

#include "handlers/confighandler.h"
//#include "handlers/systemhandler.h"
//#include "handlers/ocshandler.h"
//#include "handlers/itemhandler.h"
#include "websockets/websocketserver.h"

int main(int argc, char *argv[])
{
    // Init
    QCoreApplication app(argc, argv);

    ConfigHandler *configHandler = new ConfigHandler(&app);
    QJsonObject appConfigApplication = configHandler->getAppConfigApplication();

    app.setApplicationName(appConfigApplication["name"].toString());
    app.setApplicationVersion(appConfigApplication["version"].toString());
    app.setOrganizationName(appConfigApplication["organization"].toString());
    app.setOrganizationDomain(appConfigApplication["domain"].toString());

    // Setup translator
    //QTranslator translator;
    //if (translator.load(QLocale(), "messages", ".", ":/i18n")) {
    //    app.installTranslator(&translator);
    //}

    // Setup CLI
    QCommandLineParser clParser;
    clParser.setApplicationDescription(appConfigApplication["description"].toString());
    clParser.addHelpOption();
    clParser.addVersionOption();

    // Port 49152-61000 will available as ephemeral port
    // https://en.wikipedia.org/wiki/Ephemeral_port
    QCommandLineOption clOptionPort(QStringList() << "p" << "port", "Port for websocket server [default: 49152].", "port", "49152");
    clParser.addOption(clOptionPort);

    clParser.process(app);

    int port = clParser.value(clOptionPort).toInt();

    // Setup websocket server
    WebSocketServer *wsServer = new WebSocketServer(appConfigApplication["id"].toString(), port, &app);
    QObject::connect(wsServer, &WebSocketServer::stopped, &app, &QCoreApplication::quit);

    if (wsServer->start()) {
        qDebug() << "Websocket server started at:" << wsServer->serverUrl().toString();
    }
    else {
        qCritical() << "Failed to start websocket server:" << wsServer->errorString();
        return 1;
    }

    return app.exec();
}
