#include <QApplication>
#include <QIcon>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    // Set application icon from embedded Qt resource (:/logo.png)
    // This affects the window title bar icon and taskbar icon on all platforms.
    QIcon appIcon(":/logo.png");
    app.setWindowIcon(appIcon);

    app.setApplicationName("NonsenseMusic");
    app.setApplicationDisplayName("Nonsense Music");
    app.setOrganizationName("NonsenseMusic");

    MainWindow w;
    w.setWindowIcon(appIcon); // also explicitly set on the main window
    w.show();

    return app.exec();
}
