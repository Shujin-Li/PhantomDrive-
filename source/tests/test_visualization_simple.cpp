#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QtMath>
#include <QRandomGenerator>

class SimpleGameView : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleGameView(QWidget* parent = nullptr)
        : QWidget(parent)
        , m_playerX(0)
        , m_playerY(0)
        , m_playerAngle(0)
        , m_tileSize(20)
    {
        setMinimumSize(800, 600);
        setAutoFillBackground(true);
        setPalette(QPalette(QColor(30, 30, 30)));

        m_simulationTimer = new QTimer(this);
        connect(m_simulationTimer, &QTimer::timeout, this, [this]() {
            m_playerAngle += 2.0;
            if (m_playerAngle >= 360.0) m_playerAngle -= 360.0;

            qreal radius = 100.0;
            m_playerX = qCos(m_playerAngle * M_PI / 180.0) * radius;
            m_playerY = qSin(m_playerAngle * M_PI / 180.0) * radius;

            update();
        });
        m_simulationTimer->start(50);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int w = width();
        int h = height();
        int centerX = w / 2;
        int centerY = h / 2;

        painter.translate(centerX, centerY);

        drawTrack(painter);
        drawPowerupBox(painter, 150, 50, "Shield");
        drawTrafficLight(painter, 200, 150, "green");
        drawPlayerCar(painter, m_playerX, m_playerY, m_playerAngle);
    }

private:
    void drawTrack(QPainter& painter)
    {
        int tileSize = m_tileSize;
        int rows = 26;
        int cols = 11;

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                int x = col * tileSize - (cols * tileSize) / 2;
                int y = row * tileSize - (rows * tileSize) / 2;

                bool isRoad = (col >= 4 && col <= 6);
                bool isIntersection = (row >= 10 && row <= 12 && col >= 3 && col <= 7);

                QColor color;
                if (isIntersection) color = QColor(44, 62, 80);
                else if (isRoad) color = QColor(52, 58, 64);
                else color = QColor(39, 174, 96);

                painter.fillRect(x, y, tileSize, tileSize, color);
            }
        }
    }

    void drawPlayerCar(QPainter& painter, qreal x, qreal y, qreal angle)
    {
        painter.save();
        painter.translate(x, y);
        painter.rotate(angle);

        painter.fillRect(-15, -10, 30, 20, QColor(231, 76, 60));
        painter.fillRect(-10, -8, 20, 16, QColor(192, 57, 43));

        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 8));
        painter.drawText(-15, -10, 30, 20, Qt::AlignCenter, "P");

        painter.restore();
    }

    void drawPowerupBox(QPainter& painter, qreal x, qreal y, const QString& type)
    {
        painter.save();
        painter.translate(x, y);

        painter.fillRect(-10, -10, 20, 20, QColor(241, 196, 15));
        painter.setPen(QColor(243, 156, 18));
        painter.drawRect(-10, -10, 20, 20);

        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 8));
        painter.drawText(-10, -10, 20, 20, Qt::AlignCenter, "?");

        painter.restore();
    }

    void drawTrafficLight(QPainter& painter, qreal x, qreal y, const QString& state)
    {
        painter.save();
        painter.translate(x, y);

        painter.fillRect(-8, -20, 16, 40, QColor(44, 62, 80));
        painter.setPen(QColor(52, 73, 94));
        painter.drawRect(-8, -20, 16, 40);

        QColor lightColor = (state == "green") ? QColor(39, 174, 96) :
                           (state == "yellow") ? QColor(241, 196, 15) :
                           QColor(231, 76, 60);

        painter.setBrush(lightColor);
        painter.drawEllipse(-5, -15, 10, 10);

        painter.restore();
    }

    QTimer* m_simulationTimer;
    qreal m_playerX;
    qreal m_playerY;
    qreal m_playerAngle;
    int m_tileSize;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("PhantomDrive E-A Visualization Test");
    app.setApplicationVersion("1.0.0");

    QWidget window;
    window.setWindowTitle("PhantomDrive - E-A Visualization Test");
    window.resize(900, 700);

    QVBoxLayout* layout = new QVBoxLayout(&window);

    SimpleGameView* gameView = new SimpleGameView(&window);
    layout->addWidget(gameView);

    QLabel* statusLabel = new QLabel("Status: Animation running | Player car orbiting center", &window);
    statusLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(statusLabel);

    window.show();
    return app.exec();
}

#include "test_visualization_simple.moc"
