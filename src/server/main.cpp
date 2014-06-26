#include <QByteArray>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QStringList>

#include "../crypt.hpp"
#include "../room.hpp"
#include "../connection.hpp"
#include "server.hpp" 

void genConfig(QJsonObject &data) {
	data["name"] = "A QPalace Server";
	data["port"] = 9998;
}

int main(int argc, char *argv[]) {
	QCoreApplication app(argc, argv);
	app.applicationName("QPalace Server");
	app.applicationVersion("0.0");
	app.organizationDomain("https://github.com/Schala/QPalace");
	
	QCommandLineParser argParser;
	argParser.addHelpOption();
	argParser.addVersionOption();
	QCommandLineOption genConfigOption(QStringList() << "conf" << "Generate a new `qpserver.conf` in " +
		QStandardPaths::displayName(QStandardPaths::ConfigLocation));
	argParser.addOption(genConfigOption);
	argParser.process(app);
	
	if (argParser.isSet(genConfigOption)) {
		QFile genConfFile(QStandardPaths::displayName(QStandardPaths::ConfigLocation)+"/qpserver.conf");
		if (!genConfFile.open(QIODevice::WriteOnly)) {
			qFatal("Unable to write "+genConfFile.fileName().toLocal8Bit().data());
			return -1;
		}
		QJsonObject genConfData;
		genConfig(genConfData);
		QJsonDocument genConfDoc(genConfData);
		genConfFile.write(genConfDoc.toJson());
		return 0;
	}
	
	QPServer server();
	
	QFile confFile(QStandardPaths::displayName(QStandardPaths::ConfigLocation)+"/qpserver.conf");
	if (!confFile.open(QIODevice::ReadOnly)) {
		qFatal("Unable to open "+confFile.fileName().toLocal8Bit().data());
		return -1;
	}
	QByteArray confData = confFile.readAll();
	QJsonDocument confJson(QJsonDocument::fromJson(confData));
	if (!server.load(confJson.object())) {
		qFatal("Unable to parse "+confFile.fileName().toLocal8Bit().data());
		return -1;
	}
	
	server.listen(QHostAddress::Any, server.port());
	return app.exec();
}
