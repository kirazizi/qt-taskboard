#include <QApplication>

#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    // QApplication must be the first Qt object created.
    QApplication app(argc, argv);

    // Application metadata (used by QSettings in Tier 2)
    QApplication::setOrganizationName("taskboard");
    QApplication::setApplicationName("qt-taskboard");

    MainWindow window;
    window.show();

    return app.exec();
}
