#include <QApplication>

#include "AeroMap.h"        // application header
#include "MainWindow.h"

int main(int argc, char* argv[])
{
	Q_INIT_RESOURCE(AeroMap);

	AeroMap app(argc, argv);

	MainWindow mainWin;
	mainWin.show();

	return app.exec();
}
