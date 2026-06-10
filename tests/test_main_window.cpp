#include "UI/mainwindow.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("PhantomDrive");
    app.setApplicationVersion("1.0.0");

    MainWindow window;
    window.setWindowTitle("PhantomDrive - 2D Racing Game");
    window.resize(1200, 800);
    window.show();

    return app.exec();
}
