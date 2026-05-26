#include <QApplication>
#include <QDebug>

#include "UBDemoWindow.h"
#include "UI/ThemeManager.h"
#include "UI/SoundGenerator.h"
#include "UI/SoundManager.h"
#include "UI/InteractiveFeedback.h"

using namespace PhantomDrive;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(ThemeManager::getStyleSheet("dark"));

    qDebug() << "=== U-B Module Test Starting ===";

    SoundGenerator::instance().generateAllSounds();
    SoundManager::instance(&app);

    qDebug() << "Sound files generated. Ready to play sounds.";

    UBDemoWindow demo;
    demo.show();

    InteractiveFeedback::instance(&demo);

    return app.exec();
}
