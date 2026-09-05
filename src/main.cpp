#include <QApplication>
#include "main_window.h"

int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    app.setApplicationName("AdjustDisplay");
    app.setApplicationDisplayName("Ubuntu ディスプレイ調整ツール");
    app.setOrganizationName("DisplayTools");
    app.setApplicationVersion("1.0.0");

    MainWindow window;
    window.show();

    return app.exec();
}
