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

#include "server.hpp" 

void genConfig(QJsonObject &data)
{
	data["name"] = "A QPalace Server";
	data["port"] = 9998;
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	app.setApplicationName("QPalace Server");
	app.setApplicationVersion("0.0");
	app.setOrganizationDomain("https://github.com/Schala/QPalace");
	
	QString outMsg;
	QTextStream ts(&outMsg);
	
	QCommandLineParser argParser;
	argParser.addHelpOption();
	argParser.addVersionOption();
	ts << "Generate a new `qpserver.conf` in " << QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
	QCommandLineOption genConfigOption(QStringList() << "conf", outMsg);
	argParser.addOption(genConfigOption);
	argParser.process(app);
	
	outMsg.clear();
	ts << QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) << "/qpserver.conf";
	QString confLoc(outMsg);
	
	if (argParser.isSet(genConfigOption))
	{
		QFile genConfFile(confLoc);
		if (!genConfFile.open(QIODevice::WriteOnly))
		{
			qFatal("Unable to write %s\n", genConfFile.fileName().toLocal8Bit().data());
			return -1;
		}
		QJsonObject genConfData;
		genConfig(genConfData);
		QJsonDocument genConfDoc(genConfData);
		genConfFile.write(genConfDoc.toJson());
		return 0;
	}
	
	QPServer server;
	
	QFile confFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/qpserver.conf");
	if (!confFile.open(QIODevice::ReadOnly))
	{
		qFatal("Unable to open %s\n", confFile.fileName().toLocal8Bit().data());
		return -1;
	}
	QByteArray confData = confFile.readAll();
	QJsonDocument confJson(QJsonDocument::fromJson(confData));
	if (!server.loadConf(confJson.object()))
	{
		qFatal("Unable to write %s\n", confFile.fileName().toLocal8Bit().data());
		return -1;
	}
	
	if (!server.start())
	{
		qFatal("Unable to start server\n");
	}
	return app.exec();
}
