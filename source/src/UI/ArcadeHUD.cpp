#include "UI/ArcadeHUD.h"

#include <QApplication>
#include <QDateTime>
#include <QScreen>
#include <QTimer>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QtMath>
#include <QGraphicsDropShadowEffect>

using namespace PhantomDrive;

namespace {

QColor speedStateColor(qreal speed, qreal speedLimit)
{
    if (speedLimit > 0.0) {
        if (speed <= speedLimit * 0.75) {
            return QColor(QStringLiteral("#2EF27A"));
        }
        if (speed <= speedLimit) {
            return QColor(QStringLiteral("#F6C343"));
        }
        return QColor(QStringLiteral("#FF3366"));
    }

    if (speed < 60.0) {
        return QColor(QStringLiteral("#2EF27A"));
    }
    if (speed < 100.0) {
        return QColor(QStringLiteral("#F6C343"));
    }
    return QColor(QStringLiteral("#FF3366"));
}

} // namespace

// ============================================================
//  SpeedometerWidget — cyberpunk neon ring gauge (200px)
// ============================================================

SpeedometerWidget::SpeedometerWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(160, 160);
}

void SpeedometerWidget::setSpeed(qreal speed)
{
    m_speed = qBound(0.0, speed, m_maxSpeed);
    update();
}

void SpeedometerWidget::setMaxSpeed(qreal maxSpeed)
{
    m_maxSpeed = qMax(1.0, maxSpeed);
    update();
}

void SpeedometerWidget::setRedLight(bool red)
{
    m_redLight = red;
    update();
}

void SpeedometerWidget::setSpeedLimit(qreal limit)
{
    m_speedLimit = qMax(0.0, limit);
    update();
}

void SpeedometerWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int W = width();
    const int H = height();
    const QPointF centre(W / 2.0, H / 2.0);
    const qreal radius = qMin(W, H) * 0.44;

    const qreal startDeg = 225.0;
    const qreal spanDeg  = 270.0;

    const QColor stateColor = speedStateColor(m_speed, m_speedLimit);

    // ---- Outer neon glow ----
    {
        QColor glowCol = stateColor;
        glowCol.setAlpha(42);
        QPen glowPen(glowCol, 18, Qt::SolidLine, Qt::RoundCap);
        p.setPen(glowPen);
        p.setBrush(Qt::NoBrush);
        QRectF gr(centre.x() - radius, centre.y() - radius, radius * 2, radius * 2);
        p.drawArc(gr, static_cast<int>((180.0 - startDeg) * 16),
                  static_cast<int>(-spanDeg * 16));
    }

    // ---- Background arc ----
    {
        QPen bgPen(QColor(15, 22, 45, 200), 8, Qt::SolidLine, Qt::RoundCap);
        p.setPen(bgPen);
        QRectF ar(centre.x() - radius, centre.y() - radius, radius * 2, radius * 2);
        p.drawArc(ar, static_cast<int>((180.0 - startDeg) * 16),
                  static_cast<int>(-spanDeg * 16));
    }

    // ---- Speed fill arc ----
    const qreal fraction = m_speed / m_maxSpeed;
    {
        QPen arcPen(stateColor, 8, Qt::SolidLine, Qt::RoundCap);
        p.setPen(arcPen);
        QRectF ar(centre.x() - radius, centre.y() - radius, radius * 2, radius * 2);
        p.drawArc(ar, static_cast<int>((180.0 - startDeg) * 16),
                  static_cast<int>(-spanDeg * fraction * 16));
    }

    // ---- Tick marks ----
    {
        const int maxVal = static_cast<int>(m_maxSpeed);
        for (int v = 0; v <= maxVal; v += 10) {
            const bool isMajor = (v % 30 == 0);
            const qreal frac = static_cast<qreal>(v) / m_maxSpeed;
            const qreal angleDeg = startDeg - frac * spanDeg;
            const qreal angleRad = qDegreesToRadians(angleDeg);
            const qreal r1 = radius * (isMajor ? 0.80 : 0.86);
            const qreal r2 = radius * 0.94;
            p.setPen(QPen(isMajor ? QColor(200, 230, 255, 200) : QColor(100, 130, 180, 120),
                          isMajor ? 2.0 : 1.0));
            p.drawLine(QPointF(centre.x() + r1 * qCos(angleRad), centre.y() - r1 * qSin(angleRad)),
                       QPointF(centre.x() + r2 * qCos(angleRad), centre.y() - r2 * qSin(angleRad)));
        }
    }

    // ---- Speed labels: 0, 60, 120, 180, 240 ----
    {
        const int maxVal = static_cast<int>(m_maxSpeed);
        const int fs = qMax(6, static_cast<int>(radius * 0.11));
        QFont lf("Segoe UI", fs, QFont::Bold);
        p.setFont(lf);
        p.setPen(QColor(180, 210, 255));
        const QList<int> labelVals = {0, 60, 120, 180, 240};
        for (int v : labelVals) {
            if (v > maxVal) continue;
            const qreal frac = static_cast<qreal>(v) / m_maxSpeed;
            const qreal angleDeg = startDeg - frac * spanDeg;
            const qreal angleRad = qDegreesToRadians(angleDeg);
            const qreal lr = radius * 0.62;
            const QPointF lp(centre.x() + lr * qCos(angleRad),
                             centre.y() - lr * qSin(angleRad));
            const qreal hw = radius * 0.22;
            const qreal hh = radius * 0.16;
            QRectF tr(lp.x() - hw, lp.y() - hh, hw * 2, hh * 2);
            p.drawText(tr, Qt::AlignCenter, QString::number(v));
        }
    }

    // ---- Needle ----
    {
        const qreal needleDeg = startDeg - fraction * spanDeg;
        const qreal needleRad = qDegreesToRadians(needleDeg);
        const qreal nLen = radius * 0.75;
        const qreal nBase = radius * 0.12;
        QPointF tip(centre.x() + nLen * qCos(needleRad),
                    centre.y() - nLen * qSin(needleRad));
        QPointF base(centre.x() - nBase * qCos(needleRad),
                     centre.y() + nBase * qSin(needleRad));

        QPen shadowPen(QColor(0, 0, 0, 100), 4, Qt::SolidLine, Qt::RoundCap);
        p.setPen(shadowPen);
        p.drawLine(base + QPointF(2, 2), tip + QPointF(2, 2));

        QColor nCol = stateColor;
        QPen needlePen(nCol, 2.5, Qt::SolidLine, Qt::RoundCap);
        p.setPen(needlePen);
        p.drawLine(base, tip);

        p.setBrush(QColor(10, 15, 35));
        p.setPen(QPen(nCol, 1.5));
        p.drawEllipse(centre, radius * 0.09, radius * 0.09);
        p.setBrush(nCol);
        p.setPen(Qt::NoPen);
        p.drawEllipse(centre, radius * 0.05, radius * 0.05);
    }

    // ---- Centre speed text ----
    {
        const int sfSize = qMax(10, static_cast<int>(radius * 0.30));
        QFont sf("Segoe UI", sfSize, QFont::Bold);
        p.setFont(sf);
        p.setPen(stateColor);
        QRectF tr(centre.x() - radius * 0.4, centre.y() + radius * 0.08,
                  radius * 0.8, radius * 0.38);
        p.drawText(tr, Qt::AlignCenter, QString::number(static_cast<int>(m_speed)));

        const int ufSize = qMax(7, static_cast<int>(radius * 0.11));
        QFont uf("Segoe UI", ufSize);
        p.setFont(uf);
        p.setPen(QColor(140, 170, 220));
        QRectF ur(centre.x() - radius * 0.3, centre.y() + radius * 0.40,
                  radius * 0.6, radius * 0.18);
        p.drawText(ur, Qt::AlignCenter, "km/h");
    }
}

// ============================================================
//  ArcadeHUD — cyberpunk racing HUD (right panel)
// ============================================================

ArcadeHUD::ArcadeHUD(QWidget* parent)
    : QWidget(parent)
    , m_modeLabel(nullptr)
    , m_speedBigLabel(nullptr)
    , m_speedUnitLabel(nullptr)
    , m_speedLimitLabel(nullptr)
    , m_trafficDot(nullptr)
    , m_trafficStateLabel(nullptr)
    , m_speedo(nullptr)
    , m_lapLabel(nullptr)
    , m_lapTimeLabel(nullptr)
    , m_totalTimeLabel(nullptr)
    , m_positionLabel(nullptr)
    , m_positionTotalLabel(nullptr)
    , m_ai1SpeedLabel(nullptr)
    , m_ai2SpeedLabel(nullptr)
    , m_boostBar(nullptr)
    , m_powerupSlot1(nullptr)
    , m_powerupTimer1(nullptr)
    , m_powerupSlot2(nullptr)
    , m_powerupTimer2(nullptr)
    , m_objectiveLabel(nullptr)
    , m_stopLabel(nullptr)
    , m_blinkTimer(new QTimer(this))
    , m_powerupTimer(new QTimer(this))
{
    setupUI();
    applyTheme();
    connect(m_blinkTimer,     &QTimer::timeout, this, &ArcadeHUD::onRedLightBlink);
    connect(m_powerupTimer,    &QTimer::timeout, this, &ArcadeHUD::onPowerupTimerTick);
    m_powerupTimer->setInterval(500);
}

ArcadeHUD::~ArcadeHUD() {}

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------
static QWidget* makeCard(QWidget* parent)
{
    QWidget* card = new QWidget(parent);
    card->setStyleSheet(R"(
        QWidget {
            background: rgba(8, 16, 40, 220);
            border: 1px solid rgba(0, 190, 255, 70);
            border-radius: 8px;
        }
    )");
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(15);
    shadow->setColor(QColor(0, 180, 255, 60));
    shadow->setOffset(0, 0);
    card->setGraphicsEffect(shadow);
    return card;
}

static QFrame* makeDivider(QWidget* parent)
{
    QFrame* d = new QFrame(parent);
    d->setFrameShape(QFrame::HLine);
    d->setStyleSheet("QFrame{color:rgba(0,190,255,35);}");
    d->setFixedHeight(1);
    return d;
}

// ---------------------------------------------------------------------------
//  setupUI — clean right-panel layout
// ---------------------------------------------------------------------------
void ArcadeHUD::setupUI()
{
    // Fixed-width right panel — never shrinks
    setFixedWidth(300);
    setMinimumHeight(510);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    setStyleSheet(R"(
        ArcadeHUD {
            background-color: rgba(6, 12, 28, 248);
            border: 2px solid rgba(0, 190, 255, 100);
            border-radius: 12px;
        }
    )");

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 10);
    root->setSpacing(4);

    // ========================================================================
    // MODE TITLE
    // ========================================================================
    m_modeLabel = new QLabel("  /  ARCADE MODE  /", this);
    m_modeLabel->setStyleSheet(
        "QLabel{color:#FF3366;font-size:11px;font-weight:bold;"
        "font-style:italic;letter-spacing:2px;}");
    m_modeLabel->setAlignment(Qt::AlignCenter);
    root->addWidget(m_modeLabel);

    root->addWidget(makeDivider(this));
    root->addSpacing(4);

    // ========================================================================
    // SPEED / LIMIT / LIGHT ROW
    // ========================================================================
    {
        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(10);

        // SPEED
        QVBoxLayout* speedCol = new QVBoxLayout();
        speedCol->setSpacing(0);
        QLabel* speedTitle = new QLabel("SPEED", this);
        speedTitle->setStyleSheet(
            "QLabel{color:rgba(0,190,255,180);font-size:8px;font-weight:bold;letter-spacing:2px;}");
        speedCol->addWidget(speedTitle);
        QHBoxLayout* snr = new QHBoxLayout();
        snr->setSpacing(2);
        m_speedBigLabel = new QLabel("0", this);
        m_speedBigLabel->setStyleSheet(
            "QLabel{color:#00FFA0;font-size:36px;font-weight:bold;"
            "font-family:'Segoe UI',sans-serif;}");
        snr->addWidget(m_speedBigLabel);
        m_speedUnitLabel = new QLabel("km/h", this);
        m_speedUnitLabel->setStyleSheet(
            "QLabel{color:rgba(0,190,255,160);font-size:9px;margin-top:18px;}");
        snr->addWidget(m_speedUnitLabel);
        snr->addStretch();
        speedCol->addLayout(snr);
        row->addLayout(speedCol, 2);

        // Vertical separator
        QFrame* vsep = new QFrame(this);
        vsep->setFrameShape(QFrame::VLine);
        vsep->setStyleSheet("QFrame{color:rgba(0,190,255,35);}");
        row->addWidget(vsep);

        // LIMIT
        QVBoxLayout* limitCol = new QVBoxLayout();
        limitCol->setSpacing(0);
        QLabel* limitTitle = new QLabel("LIMIT", this);
        limitTitle->setStyleSheet(
            "QLabel{color:rgba(255,255,255,100);font-size:8px;font-weight:bold;letter-spacing:2px;}");
        limitCol->addWidget(limitTitle);
        m_speedLimitLabel = new QLabel("--", this);
        m_speedLimitLabel->setStyleSheet(
            "QLabel{color:#FFAA00;font-size:22px;font-weight:bold;"
            "font-family:'Segoe UI',sans-serif;}");
        limitCol->addWidget(m_speedLimitLabel);
        limitCol->addStretch();
        row->addLayout(limitCol, 1);

        // Vertical separator
        QFrame* vsep2 = new QFrame(this);
        vsep2->setFrameShape(QFrame::VLine);
        vsep2->setStyleSheet("QFrame{color:rgba(0,190,255,35);}");
        row->addWidget(vsep2);

        // LIGHT
        QVBoxLayout* lightCol = new QVBoxLayout();
        lightCol->setSpacing(2);
        lightCol->setAlignment(Qt::AlignHCenter);
        QLabel* lightTitle = new QLabel("LIGHT", this);
        lightTitle->setStyleSheet(
            "QLabel{color:rgba(255,255,255,100);font-size:8px;font-weight:bold;letter-spacing:2px;}");
        lightTitle->setAlignment(Qt::AlignHCenter);
        lightCol->addWidget(lightTitle);
        m_trafficDot = new QLabel(this);
        m_trafficDot->setFixedSize(22, 22);
        m_trafficDot->setStyleSheet(
            "QLabel{background:#00E060;border-radius:11px;"
            "border:2px solid rgba(255,255,255,60);}");
        lightCol->addWidget(m_trafficDot, 0, Qt::AlignHCenter);
        m_trafficStateLabel = new QLabel("GREEN", this);
        m_trafficStateLabel->setStyleSheet(
            "QLabel{color:#00E060;font-size:8px;font-weight:bold;}");
        m_trafficStateLabel->setAlignment(Qt::AlignHCenter);
        lightCol->addWidget(m_trafficStateLabel);
        row->addLayout(lightCol, 1);

        root->addLayout(row);
    }

    root->addWidget(makeDivider(this));
    root->addSpacing(4);

    // ========================================================================
    // SPEEDOMETER (200x200px — compact)
    // ========================================================================
    m_speedo = new SpeedometerWidget(this);
    m_speedo->setMaxSpeed(240.0);
    m_speedo->setFixedSize(200, 200);
    root->addWidget(m_speedo, 0, Qt::AlignHCenter);

    root->addWidget(makeDivider(this));
    root->addSpacing(6);

    // ========================================================================
    // INFO CARDS — each card: title + value, fixed height ~60px
    // ========================================================================

    // ---- LAP ----
    {
        QWidget* card = makeCard(this);
        QHBoxLayout* cl = new QHBoxLayout(card);
        cl->setContentsMargins(12, 8, 12, 8);
        cl->setSpacing(8);
        QLabel* t = new QLabel("LAP", card);
        t->setStyleSheet("QLabel{color:rgba(0,190,255,180);font-size:8px;font-weight:bold;letter-spacing:2px;}");
        cl->addWidget(t);
        m_lapLabel = new QLabel("1 / 3", card);
        m_lapLabel->setStyleSheet(
            "QLabel{color:#00E5FF;font-size:15px;font-weight:bold;"
            "font-family:'Segoe UI',sans-serif;}");
        m_lapLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        cl->addWidget(m_lapLabel);
        root->addWidget(card);
    }

    // ---- LAP TIME + TOTAL TIME (side by side) ----
    {
        QHBoxLayout* timeRow = new QHBoxLayout();
        timeRow->setSpacing(6);

        QWidget* lapCard = makeCard(this);
        QHBoxLayout* lapCl = new QHBoxLayout(lapCard);
        lapCl->setContentsMargins(10, 8, 10, 8);
        lapCl->setSpacing(4);
        QLabel* lapT = new QLabel("LAP TIME", lapCard);
        lapT->setStyleSheet("QLabel{color:rgba(0,190,255,180);font-size:7px;font-weight:bold;letter-spacing:1px;}");
        lapCl->addWidget(lapT, 0, Qt::AlignTop);
        m_lapTimeLabel = new QLabel("00:00.000", lapCard);
        m_lapTimeLabel->setStyleSheet(
            "QLabel{color:#00E5FF;font-size:12px;font-weight:bold;"
            "font-family:'Segoe UI',sans-serif;}");
        m_lapTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lapCl->addWidget(m_lapTimeLabel, 1);
        timeRow->addWidget(lapCard, 1);

        QWidget* totalCard = makeCard(this);
        QHBoxLayout* totalCl = new QHBoxLayout(totalCard);
        totalCl->setContentsMargins(10, 8, 10, 8);
        totalCl->setSpacing(4);
        QLabel* totalT = new QLabel("TOTAL", totalCard);
        totalT->setStyleSheet("QLabel{color:rgba(180,100,255,180);font-size:7px;font-weight:bold;letter-spacing:1px;}");
        totalCl->addWidget(totalT, 0, Qt::AlignTop);
        m_totalTimeLabel = new QLabel("00:00.000", totalCard);
        m_totalTimeLabel->setStyleSheet(
            "QLabel{color:#CC66FF;font-size:12px;font-weight:bold;"
            "font-family:'Segoe UI',sans-serif;}");
        m_totalTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        totalCl->addWidget(m_totalTimeLabel, 1);
        timeRow->addWidget(totalCard, 1);

        root->addLayout(timeRow);
    }

    // ---- POSITION ----
    {
        QWidget* card = makeCard(this);
        QHBoxLayout* cl = new QHBoxLayout(card);
        cl->setContentsMargins(12, 8, 12, 8);
        cl->setSpacing(8);
        QLabel* t = new QLabel("POSITION", card);
        t->setStyleSheet("QLabel{color:rgba(255,220,0,180);font-size:8px;font-weight:bold;letter-spacing:2px;}");
        cl->addWidget(t);
        m_positionLabel = new QLabel("1ST", card);
        m_positionLabel->setStyleSheet(
            "QLabel{color:#FFE000;font-size:24px;font-weight:bold;"
            "font-family:'Segoe UI',sans-serif;}");
        m_positionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        cl->addWidget(m_positionLabel);
        m_positionTotalLabel = new QLabel("/ 1", card);
        m_positionTotalLabel->setStyleSheet(
            "QLabel{color:rgba(255,220,0,120);font-size:11px;font-weight:bold;"
            "font-family:'Segoe UI',sans-serif;}");
        m_positionTotalLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        cl->addWidget(m_positionTotalLabel);
        root->addWidget(card);
    }

    // ---- POWERUP STATE ----
    {
        QWidget* card = makeCard(this);
        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setContentsMargins(12, 8, 12, 8);
        cl->setSpacing(4);
        QLabel* t = new QLabel("POWERUP", card);
        t->setStyleSheet("QLabel{color:rgba(180,100,255,180);font-size:8px;font-weight:bold;letter-spacing:2px;}");
        cl->addWidget(t);

        // Slot 1
        QHBoxLayout* slot1 = new QHBoxLayout();
        slot1->setSpacing(4);
        m_powerupSlot1 = new QLabel("--", card);
        m_powerupSlot1->setStyleSheet(
            "QLabel{color:#CC88FF;font-size:13px;font-weight:bold;"
            "font-family:'Segoe UI',sans-serif;}");
        m_powerupSlot1->setFixedWidth(70);
        slot1->addWidget(m_powerupSlot1);
        m_powerupTimer1 = new QLabel("", card);
        m_powerupTimer1->setStyleSheet(
            "QLabel{color:#9966CC;font-size:11px;font-family:'Segoe UI',sans-serif;}");
        slot1->addWidget(m_powerupTimer1);
        slot1->addStretch();
        cl->addLayout(slot1);

        // Slot 2
        QHBoxLayout* slot2 = new QHBoxLayout();
        slot2->setSpacing(4);
        m_powerupSlot2 = new QLabel("--", card);
        m_powerupSlot2->setStyleSheet(
            "QLabel{color:#CC88FF;font-size:13px;font-weight:bold;"
            "font-family:'Segoe UI',sans-serif;}");
        m_powerupSlot2->setFixedWidth(70);
        slot2->addWidget(m_powerupSlot2);
        m_powerupTimer2 = new QLabel("", card);
        m_powerupTimer2->setStyleSheet(
            "QLabel{color:#9966CC;font-size:11px;font-family:'Segoe UI',sans-serif;}");
        slot2->addWidget(m_powerupTimer2);
        slot2->addStretch();
        cl->addLayout(slot2);

        root->addWidget(card);
    }

    // ---- CURRENT OBJECTIVE ----
    {
        QWidget* card = makeCard(this);
        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setContentsMargins(12, 8, 12, 8);
        cl->setSpacing(4);
        QLabel* t = new QLabel("OBJECTIVE", card);
        t->setStyleSheet("QLabel{color:rgba(0,230,180,180);font-size:8px;font-weight:bold;letter-spacing:2px;}");
        cl->addWidget(t);
        m_objectiveLabel = new QLabel("Next: CP1", card);
        m_objectiveLabel->setStyleSheet(
            "QLabel{color:#00E6B4;font-size:13px;font-weight:bold;"
            "font-family:'Segoe UI',sans-serif;}");
        cl->addWidget(m_objectiveLabel);
        root->addWidget(card);
    }

    // ---- BOOST ----
    {
        QWidget* card = makeCard(this);
        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setContentsMargins(12, 8, 12, 8);
        cl->setSpacing(5);

        QHBoxLayout* header = new QHBoxLayout();
        header->setSpacing(4);
        QLabel* t = new QLabel("BOOST", card);
        t->setStyleSheet("QLabel{color:rgba(0,190,255,180);font-size:8px;font-weight:bold;letter-spacing:2px;}");
        header->addWidget(t);
        header->addStretch();
        cl->addLayout(header);

        m_boostBar = new QProgressBar(card);
        m_boostBar->setRange(0, 100);
        m_boostBar->setValue(0);
        m_boostBar->setTextVisible(false);
        m_boostBar->setFixedHeight(10);
        m_boostBar->setStyleSheet(R"(
            QProgressBar {
                background: rgba(255,255,255,8);
                border-radius: 5px;
                border: 1px solid rgba(0,190,255,50);
            }
            QProgressBar::chunk {
                background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 #00FFA0, stop:0.5 #00BEFF, stop:1 #FF3366);
                border-radius: 5px;
            }
        )");
        cl->addWidget(m_boostBar);
        root->addWidget(card);
    }

    // ---- AI 1 SPEED ----
    {
        QWidget* card = makeCard(this);
        QHBoxLayout* cl = new QHBoxLayout(card);
        cl->setContentsMargins(12, 8, 12, 8);
        cl->setSpacing(8);
        QLabel* t = new QLabel("AI 1 SPEED", card);
        t->setStyleSheet("QLabel{color:rgba(255,140,0,180);font-size:8px;font-weight:bold;letter-spacing:2px;}");
        cl->addWidget(t);
        m_ai1SpeedLabel = new QLabel("--", card);
        m_ai1SpeedLabel->setStyleSheet(
            "QLabel{color:#FF8800;font-size:13px;font-weight:bold;"
            "font-family:'Segoe UI',sans-serif;}");
        m_ai1SpeedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        cl->addWidget(m_ai1SpeedLabel);
        root->addWidget(card);
    }

    // ---- AI 2 SPEED ----
    {
        QWidget* card = makeCard(this);
        QHBoxLayout* cl = new QHBoxLayout(card);
        cl->setContentsMargins(12, 8, 12, 8);
        cl->setSpacing(8);
        QLabel* t = new QLabel("AI 2 SPEED", card);
        t->setStyleSheet("QLabel{color:rgba(255,100,200,180);font-size:8px;font-weight:bold;letter-spacing:2px;}");
        cl->addWidget(t);
        m_ai2SpeedLabel = new QLabel("--", card);
        m_ai2SpeedLabel->setStyleSheet(
            "QLabel{color:#FF66CC;font-size:13px;font-weight:bold;"
            "font-family:'Segoe UI',sans-serif;}");
        m_ai2SpeedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        cl->addWidget(m_ai2SpeedLabel);
        root->addWidget(card);
    }

    // ========================================================================
    // STOP badge (absolute overlay)
    // ========================================================================
    m_stopLabel = new QLabel("STOP", this);
    m_stopLabel->setAlignment(Qt::AlignCenter);
    m_stopLabel->setStyleSheet(
        "QLabel{color:#FFFFFF;background:#FF0033;font-size:11px;font-weight:bold;"
        "border-radius:6px;padding:3px 8px;}");
    m_stopLabel->setVisible(false);
    m_stopLabel->setGeometry(6, 6, 55, 24);
    m_stopLabel->raise();
}

// ---------------------------------------------------------------------------
//  setGameMode — sets the title based on mode string ("Arcade","Learning","Custom Track")
// ---------------------------------------------------------------------------
void ArcadeHUD::setGameMode(const QString& mode)
{
    m_gameMode = mode;
    applyTheme();
}

// ---------------------------------------------------------------------------
//  applyTheme
// ---------------------------------------------------------------------------
void ArcadeHUD::applyTheme()
{
    if (!m_modeLabel) return;
    if (m_gameMode == "Learning") {
        m_modeLabel->setText("  /  LEARNING MODE  /");
        m_modeLabel->setStyleSheet(
            "QLabel{color:#00FFA0;font-size:11px;font-weight:bold;"
            "font-style:italic;letter-spacing:2px;}");
    } else if (m_gameMode == "Custom Track") {
        m_modeLabel->setText("  /  CUSTOM TRACK  /");
        m_modeLabel->setStyleSheet(
            "QLabel{color:#59F7FF;font-size:11px;font-weight:bold;"
            "font-style:italic;letter-spacing:2px;}");
    } else {
        m_modeLabel->setText("  /  ARCADE MODE  /");
        m_modeLabel->setStyleSheet(
            "QLabel{color:#FF3366;font-size:11px;font-weight:bold;"
            "font-style:italic;letter-spacing:2px;}");
    }
}

void ArcadeHUD::setFloatingStyle()     { setGameMode("Arcade"); }
void ArcadeHUD::setCustomTrackStyle() { setGameMode("Custom Track"); }
void ArcadeHUD::setCustomTrackVisualMode(bool enabled)
{
    if (enabled) setGameMode("Custom Track");
}

void ArcadeHUD::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
}

void ArcadeHUD::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

// ---------------------------------------------------------------------------
//  updateSpeedDisplay
// ---------------------------------------------------------------------------
void ArcadeHUD::updateSpeedDisplay()
{
    const QString color = speedStateColor(m_currentSpeed, m_currentSpeedLimit).name();
    if (m_speedBigLabel) {
        m_speedBigLabel->setStyleSheet(
            QString("QLabel{color:%1;font-size:36px;font-weight:bold;"
                    "font-family:'Segoe UI',sans-serif;}").arg(color));
    }
    if (m_speedo) {
        m_speedo->setSpeed(m_currentSpeed);
        m_speedo->setRedLight(false);
    }
}

// ---------------------------------------------------------------------------
//  Data update API
// ---------------------------------------------------------------------------
void ArcadeHUD::updateSpeed(qreal speed)
{
    m_currentSpeed = speed;
    if (m_speedBigLabel) m_speedBigLabel->setText(QString::number(static_cast<int>(speed)));
    updateSpeedDisplay();
}

void ArcadeHUD::updateLap(int lapsCompleted, int totalLaps)
{
    m_currentLap = lapsCompleted + 1;
    m_totalLaps  = totalLaps;
    if (m_lapLabel) {
        m_lapLabel->setText(QString("Lap %1 / %2").arg(lapsCompleted + 1).arg(totalLaps));
    }
    if (m_objectiveLabel) {
        const int nextLap = lapsCompleted + 1;
        m_objectiveLabel->setText(
            nextLap >= totalLaps
                ? QStringLiteral("Next: FINISH")
                : QStringLiteral("Lap %1 / %2").arg(nextLap).arg(totalLaps));
    }
}

void ArcadeHUD::updateRouteProgress(int passed, int total, const QString& nextTarget)
{
    // Don't overwrite lap label — put CP info in objective instead.
    if (m_objectiveLabel) {
        m_objectiveLabel->setText(QStringLiteral("CP %1/%2 | Next: %3")
                                      .arg(passed)
                                      .arg(total)
                                      .arg(nextTarget));
    }
}

void ArcadeHUD::updateLapTime(const QString& time)
{
    if (m_lapTimeLabel) m_lapTimeLabel->setText(time);
}

void ArcadeHUD::updateTotalTime(const QString& time)
{
    if (m_totalTimeLabel) m_totalTimeLabel->setText(time);
}

void ArcadeHUD::updatePosition(int position, int totalRacers)
{
    m_totalRacers = qMax(1, totalRacers);
    QString posText;
    switch (position) {
    case 1: posText = "1ST"; break;
    case 2: posText = "2ND"; break;
    case 3: posText = "3RD"; break;
    default: posText = QString("%1TH").arg(position); break;
    }
    if (m_positionLabel) m_positionLabel->setText(posText);
    if (m_positionTotalLabel) m_positionTotalLabel->setText(QStringLiteral(" / %1").arg(m_totalRacers));
}

void ArcadeHUD::updateBoost(qreal boostPercent)
{
    m_boostPercent = qBound(0.0, boostPercent, 100.0);
    if (m_boostBar) m_boostBar->setValue(static_cast<int>(m_boostPercent));
}

void ArcadeHUD::updateAISpeed1(qreal speedKmh, bool available)
{
    if (m_ai1SpeedLabel) {
        m_ai1SpeedLabel->setText(available
                                     ? QStringLiteral("%1 km/h").arg(static_cast<int>(speedKmh))
                                     : QStringLiteral("--"));
    }
}

void ArcadeHUD::updateAISpeed2(qreal speedKmh, bool available)
{
    if (m_ai2SpeedLabel) {
        m_ai2SpeedLabel->setText(available
                                     ? QStringLiteral("%1 km/h").arg(static_cast<int>(speedKmh))
                                     : QStringLiteral("--"));
    }
}

void ArcadeHUD::updateSpeedLimit(qreal limitKmh)
{
    m_currentSpeedLimit = qMax(0.0, limitKmh);
    if (m_speedLimitLabel) {
        m_speedLimitLabel->setText(limitKmh > 0 ? QString::number(static_cast<int>(limitKmh)) : "--");
    }
    if (m_speedo) {
        m_speedo->setSpeedLimit(m_currentSpeedLimit);
    }
    updateSpeedDisplay();
}

void ArcadeHUD::updateTrafficLight(const QString& state)
{
    m_trafficState = state.toLower();
    const bool red    = (m_trafficState == "red");
    const bool yellow = (m_trafficState == "yellow" || m_trafficState == "amber");

    if (m_trafficDot) {
        QString dotColor = red ? "#FF0033" : yellow ? "#FFAA00" : "#00E060";
        m_trafficDot->setStyleSheet(
            QString("QLabel{background:%1;border-radius:11px;"
                    "border:2px solid rgba(255,255,255,60);}").arg(dotColor));
    }
    if (m_trafficStateLabel) {
        QString stateText  = red ? "RED" : yellow ? "YELLOW" : "GREEN";
        QString stateColor = red ? "#FF0033" : yellow ? "#FFAA00" : "#00E060";
        m_trafficStateLabel->setText(stateText);
        m_trafficStateLabel->setStyleSheet(
            QString("QLabel{color:%1;font-size:8px;font-weight:bold;}").arg(stateColor));
    }
    if (m_stopLabel) m_stopLabel->setVisible(red);

    if (red) {
        m_blinkOn = true;
        m_blinkTimer->start(500);
    } else {
        m_blinkTimer->stop();
        m_blinkOn = false;
    }
    updateSpeedDisplay();
}

void ArcadeHUD::onRedLightBlink()
{
    m_blinkOn = !m_blinkOn;
    updateSpeedDisplay();
}

// ---------------------------------------------------------------------------
//  Race notifications
// ---------------------------------------------------------------------------
void ArcadeHUD::showRaceBanner(const QString& message)
{
    emit centerNotificationRequested(message, 3500);
}

void ArcadeHUD::showLapCompleted(int lapNumber)
{
    emit centerNotificationRequested(QStringLiteral("Lap %1 Complete!").arg(lapNumber), 3500);
}

void ArcadeHUD::showRaceFinished(int finalPosition, const QString& totalTime)
{
    QString posText;
    switch (finalPosition) {
    case 1: posText = "1ST PLACE!"; break;
    case 2: posText = "2ND PLACE";  break;
    case 3: posText = "3RD PLACE";  break;
    default: posText = QString("%1TH PLACE").arg(finalPosition); break;
    }
    emit centerNotificationRequested(
        QString("%1\nTotal: %2").arg(posText).arg(totalTime), 5000);
}

// ---------------------------------------------------------------------------
//  reset
// ---------------------------------------------------------------------------
void ArcadeHUD::reset()
{
    m_currentLap    = 1;
    m_totalLaps     = 3;
    m_currentSpeed  = 0;
    m_currentSpeedLimit = 0.0;
    m_boostPercent = 0;
    m_trafficState  = "green";
    m_blinkOn       = false;
    m_totalRacers  = 1;
    m_blinkTimer->stop();
    m_powerupTimer->stop();
    m_powerup1ExpiryMs = 0;
    m_powerup2ExpiryMs = 0;
    m_powerup1Type.clear();
    m_powerup2Type.clear();

    if (m_speedBigLabel)  m_speedBigLabel->setText("0");
    if (m_lapLabel)        m_lapLabel->setText("Lap 1 / 3");
    if (m_lapTimeLabel)    m_lapTimeLabel->setText("00:00.000");
    if (m_totalTimeLabel)  m_totalTimeLabel->setText("00:00.000");
    if (m_positionLabel)   m_positionLabel->setText("1ST");
    if (m_positionTotalLabel) m_positionTotalLabel->setText("/ 1");
    if (m_ai1SpeedLabel)   m_ai1SpeedLabel->setText("--");
    if (m_ai2SpeedLabel)   m_ai2SpeedLabel->setText("--");
    if (m_boostBar)       m_boostBar->setValue(0);
    if (m_speedLimitLabel) m_speedLimitLabel->setText("--");
    if (m_stopLabel)       m_stopLabel->setVisible(false);
    if (m_trafficDot)      m_trafficDot->setStyleSheet(
        "QLabel{background:#00E060;border-radius:11px;border:2px solid rgba(255,255,255,60);}");
    if (m_trafficStateLabel) {
        m_trafficStateLabel->setText("GREEN");
        m_trafficStateLabel->setStyleSheet("QLabel{color:#00E060;font-size:8px;font-weight:bold;}");
    }
    if (m_speedo) { m_speedo->setSpeed(0); m_speedo->setRedLight(false); }
    if (m_powerupSlot1) m_powerupSlot1->setText("--");
    if (m_powerupSlot2) m_powerupSlot2->setText("--");
    if (m_powerupTimer1) m_powerupTimer1->setText("");
    if (m_powerupTimer2) m_powerupTimer2->setText("");
    if (m_objectiveLabel) m_objectiveLabel->setText("Next: CP1");
    updateSpeedDisplay();
}

void ArcadeHUD::setVisible(bool visible)
{
    QWidget::setVisible(visible);
}

// ---------------------------------------------------------------------------
//  Powerup state
// ---------------------------------------------------------------------------
void ArcadeHUD::updatePowerupState(const QString& type, int remainingSecs)
{
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();

    // Use the slot that expires sooner (or is empty)
    bool useSlot1 = (m_powerup1ExpiryMs == 0)
                    || (m_powerup2ExpiryMs > 0 && m_powerup1ExpiryMs > m_powerup2ExpiryMs);
    QLabel* slotLabel = useSlot1 ? m_powerupSlot1 : m_powerupSlot2;
    QLabel* timerLabel = useSlot1 ? m_powerupTimer1 : m_powerupTimer2;

    if (!slotLabel || !timerLabel) return;

    QColor slotColor;
    if (type == "Boost")       slotColor = QColor("#00FF88");
    else if (type == "Shield")  slotColor = QColor("#44AAFF");
    else if (type == "EMP")     slotColor = QColor("#FF88FF");
    else if (type == "Repair")   slotColor = QColor("#FF6644");
    else if (type == "Missile") slotColor = QColor("#FF4444");
    else if (type == "Oil")      slotColor = QColor("#888888");
    else if (type == "Invis")   slotColor = QColor("#AAAAAA");
    else if (type == "Magnet")  slotColor = QColor("#FFAA44");
    else                          slotColor = QColor("#CC88FF");

    slotLabel->setText(type);
    slotLabel->setStyleSheet(
        QString("QLabel{color:%1;font-size:13px;font-weight:bold;"
                "font-family:'Segoe UI',sans-serif;}").arg(slotColor.name()));

    if (remainingSecs > 0) {
        timerLabel->setText(QStringLiteral("%1s").arg(remainingSecs));
        if (useSlot1) {
            m_powerup1ExpiryMs = nowMs + remainingSecs * 1000;
            m_powerup1Type = type;
        } else {
            m_powerup2ExpiryMs = nowMs + remainingSecs * 1000;
            m_powerup2Type = type;
        }
        if (!m_powerupTimer->isActive()) m_powerupTimer->start();
    } else {
        timerLabel->setText("");
        if (useSlot1) { m_powerup1ExpiryMs = 0; m_powerup1Type.clear(); }
        else           { m_powerup2ExpiryMs = 0; m_powerup2Type.clear(); }
    }
}

void ArcadeHUD::clearPowerupState()
{
    if (m_powerupSlot1) m_powerupSlot1->setText("--");
    if (m_powerupSlot2) m_powerupSlot2->setText("--");
    if (m_powerupTimer1) m_powerupTimer1->setText("");
    if (m_powerupTimer2) m_powerupTimer2->setText("");
    m_powerup1ExpiryMs = 0;
    m_powerup2ExpiryMs = 0;
    m_powerup1Type.clear();
    m_powerup2Type.clear();
    m_powerupTimer->stop();
}

void ArcadeHUD::onPowerupTimerTick()
{
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    bool changed = false;

    if (m_powerup1ExpiryMs > 0 && nowMs >= m_powerup1ExpiryMs) {
        if (m_powerupSlot1) m_powerupSlot1->setText("--");
        if (m_powerupTimer1) m_powerupTimer1->setText("");
        m_powerup1ExpiryMs = 0;
        m_powerup1Type.clear();
        changed = true;
    } else if (m_powerup1ExpiryMs > 0 && m_powerupTimer1) {
        int secs = qMax(0, static_cast<int>((m_powerup1ExpiryMs - nowMs) / 1000));
        m_powerupTimer1->setText(QStringLiteral("%1s").arg(secs));
    }

    if (m_powerup2ExpiryMs > 0 && nowMs >= m_powerup2ExpiryMs) {
        if (m_powerupSlot2) m_powerupSlot2->setText("--");
        if (m_powerupTimer2) m_powerupTimer2->setText("");
        m_powerup2ExpiryMs = 0;
        m_powerup2Type.clear();
        changed = true;
    } else if (m_powerup2ExpiryMs > 0 && m_powerupTimer2) {
        int secs = qMax(0, static_cast<int>((m_powerup2ExpiryMs - nowMs) / 1000));
        m_powerupTimer2->setText(QStringLiteral("%1s").arg(secs));
    }

    if (m_powerup1ExpiryMs == 0 && m_powerup2ExpiryMs == 0) {
        m_powerupTimer->stop();
    }
    Q_UNUSED(changed);
}

// ---------------------------------------------------------------------------
//  Current objective
// ---------------------------------------------------------------------------
void ArcadeHUD::updateCurrentObjective(const QString& objective)
{
    if (m_objectiveLabel) m_objectiveLabel->setText(objective);
}


