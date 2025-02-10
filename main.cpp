#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "ballsmodel.h"

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);

	qmlRegisterType<BallsModel>("BallsModel",0,1,"BallsModel");
	QQmlApplicationEngine engine;
	// QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
	// 				 &app, []() { QCoreApplication::exit(-1); },
	// Qt::QueuedConnection);
	engine.load("../../Main.qml");

	return app.exec();
}
