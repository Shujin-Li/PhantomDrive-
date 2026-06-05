#include "UI/mainwindow.h"
#include "core/VehiclePhysics.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QDebug>

class KeyboardTestWidget : public QWidget
{
    Q_OBJECT

public:
    KeyboardTestWidget(PhantomDrive::VehiclePhysics* physics, QWidget* parent = nullptr)
        : QWidget(parent)
        , m_vehiclePhysics(physics)
        , m_statusLabel(new QLabel)
    {
        setLayout(new QVBoxLayout);
        layout()->addWidget(m_statusLabel);
        setFocusPolicy(Qt::StrongFocus);
        
        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &KeyboardTestWidget::updateStatus);
        timer->start(100);
    }

private slots:
    void updateStatus()
    {
        QString status = QString(
            "车辆状态:\n"
            "加速：%1\n"
            "刹车：%2\n"
            "左转：%3\n"
            "右转：%4\n"
            "手刹：%5\n\n"
            "速度：%6 km/h\n"
            "位置：(%7, %8)\n"
            "角度：%9°"
        )
        .arg(m_vehiclePhysics->getAcceleration() > 0 ? "是" : "否")
        .arg(m_vehiclePhysics->getAcceleration() < 0 ? "是" : "否")
        .arg(m_vehiclePhysics->getPosition().x() < 0 ? "否" : "是")
        .arg(m_vehiclePhysics->getPosition().x() > 0 ? "否" : "是")
        .arg(m_vehiclePhysics->getAcceleration() == 0 && m_vehiclePhysics->getSpeed() > 0 ? "否" : "是")
        .arg(m_vehiclePhysics->getSpeed(), 0, 'f', 1)
        .arg(m_vehiclePhysics->getPosition().x(), 0, 'f', 1)
        .arg(m_vehiclePhysics->getPosition().y(), 0, 'f', 1)
        .arg(m_vehiclePhysics->getRotation(), 0, 'f', 1);
        
        m_statusLabel->setText(status);
        m_statusLabel->adjustSize();
    }

protected:
    void keyPressEvent(QKeyEvent* event) override
    {
        qDebug() << "KeyboardTestWidget 收到按键按下:" << event->key();
        if (m_vehiclePhysics) {
            m_vehiclePhysics->handleKeyPress(event);
        }
        event->accept();
    }

    void keyReleaseEvent(QKeyEvent* event) override
    {
        qDebug() << "KeyboardTestWidget 收到按键释放:" << event->key();
        if (m_vehiclePhysics) {
            m_vehiclePhysics->handleKeyRelease(event);
        }
        event->accept();
    }

private:
    PhantomDrive::VehiclePhysics* m_vehiclePhysics;
    QLabel* m_statusLabel;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("PhantomDrive Keyboard Test");
    app.setApplicationVersion("1.0.0");

    QWidget window;
    window.setWindowTitle("键盘输入测试");
    window.resize(800, 600);

    auto* layout = new QVBoxLayout(&window);

    PhantomDrive::VehiclePhysics physics;
    KeyboardTestWidget testWidget(&physics);
    
    layout->addWidget(&testWidget);

    window.show();
    window.setFocus();
    testWidget.setFocus();

    qDebug() << "=== 键盘输入测试程序 ===";
    qDebug() << "请按 W/A/S/D 或方向键测试车辆控制";
    qDebug() << "按空格键测试手刹";
    qDebug() << "查看控制台输出和窗口状态";

    return app.exec();
}

#include "test_keyboard_input.moc"
