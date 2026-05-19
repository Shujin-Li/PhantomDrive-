#include "UI/mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("PhantomDrive"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));

    MainWindow window;
    window.setWindowTitle(QStringLiteral("PhantomDrive"));
    window.resize(1200, 800);
    window.show();

    return app.exec();
}
