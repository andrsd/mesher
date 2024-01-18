#include "mesherconfig.h"
#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>

int
main(int argc, char * argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(MESHER_APP_NAME);
    QCoreApplication::setApplicationVersion(MESHER_VERSION);

#ifdef __APPLE__
    app.setQuitOnLastWindowClosed(false);
#endif
    app.setWindowIcon(QIcon(":/resources/app-icon.png"));
    MainWindow w;
    w.show();

    return app.exec();
}
