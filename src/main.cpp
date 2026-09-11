#include <QApplication>
#include "main_window.h"
#include "i18n.h"

int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    app.setApplicationName("AdjustDisplay");
    app.setOrganizationName("DisplayTools");
    app.setApplicationVersion("1.0.0");

    // Initialize I18n
    auto *i18n = I18n::instance();
    app.setApplicationDisplayName(i18n->t("app_display_name"));

    MainWindow window;
    window.show();

    return app.exec();
}
