#include "UI/TestVisualizationWindow.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("PhantomDrive E-A Visualization Test");
    app.setApplicationVersion("1.0.0");

    TestVisualizationWindow window;
    window.show();

    return app.exec();
}
