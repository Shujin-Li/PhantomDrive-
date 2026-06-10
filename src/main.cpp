#include "UI/mainwindow.h"
#include "UI/SoundGenerator.h"
#include "UI/SoundManager.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("PhantomDrive"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));

    PhantomDrive::SoundGenerator::instance().generateAllSounds();
    PhantomDrive::SoundManager::instance(&app);

    MainWindow window;
    window.setWindowTitle(QStringLiteral("PhantomDrive"));
    window.resize(1200, 800);
    window.show();

    return app.exec();
}
