#include <QJsonObject>
//#include <QTranslator>
//#include <QLocale>
#include <QCommandLineParser>
#include <QCoreApplication>

#include "handlers/confighandler.h"
#include "handlers/systemhandler.h"
#include "handlers/ocshandler.h"
#include "handlers/itemhandler.h"

int main(int argc, char *argv[])
{
    // Init
    QCoreApplication app(argc, argv);

    ConfigHandler *configHandler = new ConfigHandler();
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
    clParser.process(app);

    return app.exec();
}
