#include "mesherconfig.h"
#include "mainwindow.h"
#include "init.h"
#include <QApplication>
#include <QCommandLineParser>

int
main(int argc, char * argv[])
{
    Init init(argc, argv);

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(MESHER_APP_NAME);
    QCoreApplication::setApplicationVersion(MESHER_VERSION);
    MainWindow::getSettings();

#ifdef __APPLE__
    app.setQuitOnLastWindowClosed(false);
#endif
    app.setWindowIcon(QIcon(":/app-icon"));
    MainWindow w;
    w.show();

    return app.exec();
}
