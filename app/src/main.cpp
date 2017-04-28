#include <QStringList>
#include <QJsonObject>
//#include <QTranslator>
//#include <QLocale>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QCoreApplication>

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
    QCommandLineOption clPortOption(QStringList() << "p" << "port", "Port for websocket server [default: 443].", "port", "443");
    clParser.addOption(clPortOption);
    clParser.process(app);

    int port = clParser.value(clPortOption).toInt();

    // Setup websocket server
    WebSocketServer *webSocketServer = new WebSocketServer(appConfigApplication["id"].toString(), port, &app);
    if (!webSocketServer->start()) {
        qFatal("Failed to start websocket server");
    }

    return app.exec();
}
