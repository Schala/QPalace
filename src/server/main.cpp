#include <cstdio>
#include <QByteArray>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>
#include <QtGlobal>

#include "../gitrev.h"
#include "server.hpp" 

void generateSettings(QJsonObject &data)
{
	data["name"] = "A QPalace Server";
	data["port"] = 9998;
	data["mediaUrl"] = "http://localhost:80/";
	data["allowInsecureClients"] = false;
	data["allowScripts"] = true;
	data["allowDrawing"] = true;
	data["allowCustomProps"] = true;
	data["allowWizards"] = true;
	data["wizardsMayKick"] = true;
	data["wizardsMayAuthor"] = true;
	data["usersMayKick"] = false;
	data["scriptsMayKick"] = false;
	data["enforceBans"] = true;
	data["purgeInactiveProps"] = true;
	data["antiSpamProtection"] = true;
	data["disableSpoofing"] = true;
	data["allowUserCreatedRooms"] = false;
	data["enforceLogonPassword"] = false;
	data["logChat"] = true;
	data["disableWhispering"] = false;
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	app.setApplicationName("QPalace Server");
	app.setApplicationVersion(QString("0.0 - git revision ").append(gitrev));
	app.setOrganizationDomain("https://github.com/Schala/QPalace");
	
	QFile confFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/qpserver.conf");
	
	QCommandLineParser argParser;
	argParser.addHelpOption();
	argParser.addVersionOption();
	argParser.process(app);
	
	if (!confFile.exists())
	{
		if (!confFile.open(QIODevice::WriteOnly))
		{
			qFatal("Unable to write %s\n", qPrintable(confFile.fileName()));
			return -1;
		}
		QJsonObject genConfData;
		generateSettings(genConfData);
		QJsonDocument genConfDoc(genConfData);
		confFile.write(genConfDoc.toJson());
		qDebug("New qpserver.conf has been written to %s. The server will now exit.",
			qPrintable(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)));
		return 0;
	}
	
	QPServer server;
	
	if (!confFile.open(QIODevice::ReadOnly))
	{
		qFatal("Unable to open %s\n", qPrintable(confFile.fileName()));
		return -1;
	}
	QByteArray confData = confFile.readAll();
	QJsonDocument confJson(QJsonDocument::fromJson(confData));
	if (!server.loadConf(confJson.object()))
	{
		qFatal("Unable to write %s\n", qPrintable(confFile.fileName()));
		return -1;
	}
	
	if (int err = !server.start())
	{
		qFatal("Unable to start server: error code %d\n", err);
	}
	return app.exec();
}
