#include "UI/GameTopBar.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>

using namespace PhantomDrive;

GameTopBar::GameTopBar(QWidget* parent)
    : QWidget(parent)
    , m_modeLabel(nullptr)
    , m_exitBtn(nullptr)
{
    setFixedHeight(42);

    setStyleSheet(R"(
        GameTopBar {
            background: rgba(6, 14, 32, 235);
            border-bottom: 1px solid rgba(0, 190, 255, 90);
            border-radius: 0px;
        }
    )");

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(16);
    shadow->setColor(QColor(0, 180, 255, 50));
    shadow->setOffset(0, 2);
    setGraphicsEffect(shadow);

    QHBoxLayout* root = new QHBoxLayout(this);
    root->setContentsMargins(16, 0, 16, 0);
    root->setSpacing(12);

    // EXIT GAME button on the left
    m_exitBtn = new QPushButton("EXIT GAME", this);
    m_exitBtn->setCursor(Qt::PointingHandCursor);
    m_exitBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(220, 30, 50, 180);
            color: #ffffff;
            border: 1px solid rgba(255, 80, 80, 150);
            border-radius: 6px;
            padding: 4px 14px;
            font-size: 11px;
            font-weight: bold;
            letter-spacing: 1px;
        }
        QPushButton:hover {
            background: rgba(255, 40, 60, 220);
        }
        QPushButton:pressed {
            background: rgba(180, 20, 40, 220);
        }
    )");
    root->addWidget(m_exitBtn);
    connect(m_exitBtn, &QPushButton::clicked, this, &GameTopBar::exitGameClicked);

    // Mode title label — centred
    m_modeLabel = new QLabel("ARCADE MODE", this);
    m_modeLabel->setStyleSheet(
        "QLabel{color:#FF3366;font-size:14px;font-weight:bold;letter-spacing:3px;}");
    m_modeLabel->setAlignment(Qt::AlignCenter);
    root->addWidget(m_modeLabel, 1);

    // Spacer on the right to balance layout
    QPushButton* spacer = new QPushButton(this);
    spacer->setFixedWidth(80);
    spacer->setFixedHeight(1);
    spacer->setAttribute(Qt::WA_TransparentForMouseEvents);
    spacer->setStyleSheet("QPushButton{background:transparent;border:none;}");
    root->addWidget(spacer);
}

void GameTopBar::setMode(const QString& mode)
{
    if (!m_modeLabel) return;
    if (mode == "Learning") {
        m_modeLabel->setText("LEARNING MODE");
        m_modeLabel->setStyleSheet(
            "QLabel{color:#00FFA0;font-size:14px;font-weight:bold;letter-spacing:3px;}");
    } else if (mode == "Custom Track") {
        m_modeLabel->setText("CUSTOM TRACK");
        m_modeLabel->setStyleSheet(
            "QLabel{color:#59F7FF;font-size:14px;font-weight:bold;letter-spacing:3px;}");
    } else {
        m_modeLabel->setText("ARCADE MODE");
        m_modeLabel->setStyleSheet(
            "QLabel{color:#FF3366;font-size:14px;font-weight:bold;letter-spacing:3px;}");
    }
}

void GameTopBar::showFinishButton(bool show)
{
    Q_UNUSED(show);
}
