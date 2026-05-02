#include <QApplication>
#include <QDir>
#include "mainwindow.h"
#include "globals.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Set app path to the directory containing the executable
    g_appPath = QCoreApplication::applicationDirPath() + "/";

    MainWindow w;
    w.show();

    return app.exec();
}
