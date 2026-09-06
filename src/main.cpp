#include <QApplication>
#include <QFile>
#include <QTimer>
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    bool screenshotMode = false;
    QString screenshotPath;
    for (int i = 1; i < argc; ++i) {
        if (QString::fromLatin1(argv[i]) == QStringLiteral("--screenshot") && i + 1 < argc) {
            screenshotMode = true;
            screenshotPath = QString::fromLatin1(argv[++i]);
            qputenv("QT_QPA_PLATFORM", "offscreen");
            break;
        }
    }

    QApplication app(argc, argv);
    QApplication::setOrganizationName("taskboard");
    QApplication::setApplicationName("qt-taskboard");

    // Load global theme stylesheet from Qt resource bundle
    QFile styleFile(QStringLiteral(":/styles/theme.qss"));
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }

    MainWindow window;
    window.resize(1150, 720);
    window.show();

    if (screenshotMode) {
        QTimer::singleShot(250, [&window, screenshotPath, &app]() {
            QPixmap pixmap = window.grab();
            pixmap.save(screenshotPath);
            app.quit();
        });
    }

    return app.exec();
}
