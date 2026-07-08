#include "UI/GarageWidget.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace PhantomDrive {

namespace {

QColor accentColorForSkin(const QString& skinId)
{
    if (skinId == QStringLiteral("blue")) {
        return QColor(QStringLiteral("#2E74FF"));
    }
    if (skinId == QStringLiteral("aurora")) {
        return QColor(QStringLiteral("#33A8FF"));
    }
    if (skinId == QStringLiteral("neon")) {
        return QColor(QStringLiteral("#23F0D7"));
    }
    if (skinId == QStringLiteral("violet")) {
        return QColor(QStringLiteral("#6C5CFF"));
    }
    if (skinId == QStringLiteral("splitfire")) {
        return QColor(QStringLiteral("#FF614D"));
    }
    if (skinId == QStringLiteral("gold")) {
        return QColor(QStringLiteral("#F5C451"));
    }
    return QColor(QStringLiteral("#E4EAF2"));
}

QColor secondaryColorForSkin(const QString& skinId)
{
    if (skinId == QStringLiteral("blue")) {
        return QColor(QStringLiteral("#91D8FF"));
    }
    if (skinId == QStringLiteral("aurora")) {
        return QColor(QStringLiteral("#8852FF"));
    }
    if (skinId == QStringLiteral("neon")) {
        return QColor(QStringLiteral("#A94BFF"));
    }
    if (skinId == QStringLiteral("violet")) {
        return QColor(QStringLiteral("#1FD3FF"));
    }
    if (skinId == QStringLiteral("splitfire")) {
        return QColor(QStringLiteral("#111827"));
    }
    if (skinId == QStringLiteral("gold")) {
        return QColor(QStringLiteral("#FFF1A8"));
    }
    return QColor(QStringLiteral("#8E9BAB"));
}

QColor canopyColorForSkin(const QString& skinId)
{
    if (skinId == QStringLiteral("blue")) {
        return QColor(QStringLiteral("#D8F2FF"));
    }
    if (skinId == QStringLiteral("aurora")) {
        return QColor(QStringLiteral("#E0F3FF"));
    }
    if (skinId == QStringLiteral("neon")) {
        return QColor(QStringLiteral("#D8FFF8"));
    }
    if (skinId == QStringLiteral("violet")) {
        return QColor(QStringLiteral("#E8E4FF"));
    }
    if (skinId == QStringLiteral("splitfire")) {
        return QColor(QStringLiteral("#FFE4D5"));
    }
    if (skinId == QStringLiteral("gold")) {
        return QColor(QStringLiteral("#FFF8D7"));
    }
    return QColor(QStringLiteral("#F8FBFF"));
}

QString themeLabelForSkin(const QString& skinId)
{
    if (skinId == QStringLiteral("blue")) {
        return QStringLiteral("Cobalt Sprint");
    }
    if (skinId == QStringLiteral("aurora")) {
        return QStringLiteral("Aurora Flux");
    }
    if (skinId == QStringLiteral("neon")) {
        return QStringLiteral("Neon Pulse");
    }
    if (skinId == QStringLiteral("violet")) {
        return QStringLiteral("Violet Surge");
    }
    if (skinId == QStringLiteral("splitfire")) {
        return QStringLiteral("Splitfire GT");
    }
    if (skinId == QStringLiteral("gold")) {
        return QStringLiteral("Championship Trim");
    }
    return QStringLiteral("Factory Classic");
}

class SkinPreviewWidget : public QWidget
{
public:
    explicit SkinPreviewWidget(const QString& skinId,
                               bool selected,
                               bool locked,
                               QWidget* parent = nullptr)
        : QWidget(parent)
        , m_skinId(skinId)
        , m_selected(selected)
        , m_locked(locked)
    {
        setFixedSize(210, 124);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QColor accent = accentColorForSkin(m_skinId);
        const QColor secondary = secondaryColorForSkin(m_skinId);

        QLinearGradient bg(rect().topLeft(), rect().bottomRight());
        bg.setColorAt(0.0, QColor(9, 20, 32));
        bg.setColorAt(1.0, QColor(8, 36, 54));
        painter.setBrush(bg);
        painter.setPen(QPen(m_selected ? accent.lighter(120) : QColor(255, 255, 255, 28),
                            m_selected ? 2.0 : 1.0));
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 18, 18);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 46));
        painter.drawEllipse(QPointF(width() * 0.77, height() * 0.28), 44, 44);
        painter.setBrush(QColor(255, 255, 255, 10));
        painter.drawRoundedRect(QRectF(18, 18, width() - 36, height() - 36), 16, 16);
        painter.setPen(QPen(QColor(255, 255, 255, 20), 1.0));
        painter.drawLine(QPointF(28, 86), QPointF(width() - 30, 86));
        painter.drawLine(QPointF(34, 98), QPointF(width() - 36, 98));

        painter.save();
        if (m_locked) {
            painter.setOpacity(0.46);
        }

        const QRectF carRect(26, 26, 158, 68);
        const QColor body = accent;
        const QColor trim = secondary;
        const QColor canopy = canopyColorForSkin(m_skinId);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 24));
        painter.drawRoundedRect(QRectF(carRect.left() + 16, carRect.bottom() - 1, carRect.width() - 8, 11), 6, 6);

        QPainterPath bodyPath;
        bodyPath.moveTo(carRect.left() + 10, carRect.bottom() - 10);
        bodyPath.cubicTo(carRect.left() + 18, carRect.top() + 24,
                         carRect.left() + 44, carRect.top() + 10,
                         carRect.left() + 84, carRect.top() + 12);
        bodyPath.cubicTo(carRect.left() + 110, carRect.top() + 12,
                         carRect.right() - 18, carRect.top() + 20,
                         carRect.right() - 6, carRect.bottom() - 18);
        bodyPath.lineTo(carRect.right() - 3, carRect.bottom() - 10);
        bodyPath.lineTo(carRect.left() + 10, carRect.bottom() - 10);
        bodyPath.closeSubpath();

        QLinearGradient bodyGradient(carRect.topLeft(), carRect.bottomRight());
        bodyGradient.setColorAt(0.0, body.lighter(155));
        bodyGradient.setColorAt(0.45, body);
        bodyGradient.setColorAt(1.0, body.darker(180));
        painter.setBrush(bodyGradient);
        painter.drawPath(bodyPath);

        painter.setBrush(QColor(trim.red(), trim.green(), trim.blue(), 210));
        QPainterPath stripePath;
        stripePath.moveTo(carRect.center().x(), carRect.top() + 4);
        stripePath.lineTo(carRect.center().x() + 10, carRect.top() + 30);
        stripePath.lineTo(carRect.center().x() + 4, carRect.bottom() - 10);
        stripePath.lineTo(carRect.center().x() - 4, carRect.bottom() - 10);
        stripePath.lineTo(carRect.center().x() - 10, carRect.top() + 30);
        stripePath.closeSubpath();
        painter.drawPath(stripePath);

        painter.setPen(QPen(QColor(255, 255, 255, 165), 1.4));
        painter.drawLine(QPointF(carRect.center().x(), carRect.top() + 6),
                         QPointF(carRect.center().x(), carRect.bottom() - 10));
        painter.setPen(QPen(trim, 2.2));
        painter.drawLine(QPointF(carRect.left() + 34, carRect.bottom() - 19),
                         QPointF(carRect.right() - 26, carRect.bottom() - 19));
        painter.drawLine(QPointF(carRect.left() + 28, carRect.top() + 34),
                         QPointF(carRect.left() + 44, carRect.bottom() - 18));
        painter.drawLine(QPointF(carRect.right() - 28, carRect.top() + 34),
                         QPointF(carRect.right() - 44, carRect.bottom() - 18));

        painter.setPen(Qt::NoPen);
        painter.setBrush(canopy);
        painter.drawRoundedRect(QRectF(carRect.left() + 46, carRect.top() + 11, 54, 19), 9, 9);
        painter.setBrush(QColor(trim.red(), trim.green(), trim.blue(), 165));
        painter.drawRoundedRect(QRectF(carRect.left() + 102, carRect.top() + 15, 22, 12), 6, 6);

        painter.setBrush(QColor(18, 24, 30));
        painter.drawEllipse(QPointF(carRect.left() + 38, carRect.bottom() - 7), 13, 13);
        painter.drawEllipse(QPointF(carRect.right() - 38, carRect.bottom() - 7), 13, 13);
        painter.setBrush(QColor(106, 124, 140));
        painter.drawEllipse(QPointF(carRect.left() + 38, carRect.bottom() - 7), 6, 6);
        painter.drawEllipse(QPointF(carRect.right() - 38, carRect.bottom() - 7), 6, 6);

        painter.setBrush(QColor(255, 249, 190, 170));
        painter.drawRoundedRect(QRectF(carRect.right() - 10, carRect.top() + 27, 10, 8), 4, 4);
        painter.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 98));
        painter.drawRoundedRect(QRectF(carRect.left() - 10, carRect.top() + 28, 20, 4), 2, 2);

        painter.restore();

        if (m_locked) {
            painter.setBrush(QColor(3, 6, 12, 138));
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 18, 18);
        }

        painter.setPen(QColor(232, 245, 255, 180));
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::Bold));
        painter.drawText(QRectF(16, 10, width() - 32, 16),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         m_selected ? QStringLiteral("ACTIVE PREVIEW") : QStringLiteral("SKIN PREVIEW"));
    }

private:
    QString m_skinId;
    bool m_selected;
    bool m_locked;
};

} // namespace

GarageWidget::GarageWidget(QWidget* parent)
    : QWidget(parent)
    , m_titleLabel(new QLabel(QStringLiteral("Garage"), this))
    , m_coinLabel(new QLabel(this))
    , m_skinListLayout(new QVBoxLayout())
{
    setObjectName(QStringLiteral("pageGarage"));
    setStyleSheet(QStringLiteral(
        "QWidget#pageGarage{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #07111D,stop:1 #0B2031);color:#F3FBFF;}"
        "QFrame[garageCard='true']{background:rgba(10,24,36,232);border:1px solid rgba(83,162,186,110);border-radius:20px;}"
        "QFrame[garageCard='true'][selected='true']{border:2px solid rgba(245,196,81,220);background:rgba(17,30,42,240);}"
        "QFrame[garageCard='true'][locked='true']{background:rgba(8,16,24,215);border-color:rgba(116,138,150,70);}"
        "QLabel#garageTitle{color:#59F7FF;font-size:28px;font-weight:800;letter-spacing:4px;}"
        "QLabel#garageCoins{background:rgba(245,196,81,28);border:1px solid rgba(245,196,81,120);border-radius:18px;"
        "color:#FFD84D;font-size:18px;font-weight:800;padding:10px 18px;}"
        "QLabel[garageEyebrow='true']{color:#8FC5D8;font-size:11px;font-weight:700;letter-spacing:2px;}"
        "QLabel[garageName='true']{color:#F3FBFF;font-size:20px;font-weight:800;}"
        "QLabel[garageMeta='true']{color:#A8D7E7;font-size:13px;}"
        "QLabel[garageBadge='true']{background:rgba(89,247,255,24);border:1px solid rgba(89,247,255,110);border-radius:10px;"
        "color:#9CF7FF;font-size:11px;font-weight:800;padding:4px 10px;}"
        "QLabel[garageBadge='true'][selected='true']{background:rgba(245,196,81,28);border-color:rgba(245,196,81,130);color:#FFE39A;}"
        "QLabel[garageBadge='true'][locked='true']{background:rgba(255,255,255,18);border-color:rgba(255,255,255,60);color:#A5BBC6;}"
        "QPushButton{background:#0A2536;color:#F3FBFF;border:1px solid #2BA9C7;border-radius:10px;padding:10px 18px;font-weight:700;}"
        "QPushButton:hover:enabled{background:#103349;border-color:#59F7FF;}"
        "QPushButton:disabled{background:#0B1620;color:#5E7683;border-color:#29404C;}"
        "QScrollArea{border:none;background:transparent;}"
        "QScrollArea > QWidget > QWidget{background:transparent;}"));

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(30, 24, 30, 24);
    rootLayout->setSpacing(18);

    m_titleLabel->setObjectName(QStringLiteral("garageTitle"));
    m_titleLabel->setAlignment(Qt::AlignCenter);
    rootLayout->addWidget(m_titleLabel);

    m_coinLabel->setObjectName(QStringLiteral("garageCoins"));
    m_coinLabel->setAlignment(Qt::AlignCenter);
    rootLayout->addWidget(m_coinLabel, 0, Qt::AlignHCenter);

    auto* topBar = new QHBoxLayout();
    topBar->setContentsMargins(0, 0, 0, 0);
    topBar->setSpacing(12);

    auto* backButton = new QPushButton(QStringLiteral("Back"), this);
    connect(backButton, &QPushButton::clicked, this, &GarageWidget::backRequested);
    topBar->addWidget(backButton, 0, Qt::AlignLeft);
    topBar->addStretch(1);
    rootLayout->addLayout(topBar);

    auto* listWidget = new QWidget(this);
    listWidget->setLayout(m_skinListLayout);
    m_skinListLayout->setContentsMargins(0, 0, 0, 0);
    m_skinListLayout->setSpacing(16);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(listWidget);
    rootLayout->addWidget(scrollArea, 1);
}

void GarageWidget::setGarageState(const QList<SkinInfo>& skins,
                                  const QString& currentSkinId,
                                  int coins)
{
    m_coinLabel->setText(QStringLiteral("Coins Available: %1").arg(coins));

    while (QLayoutItem* item = m_skinListLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    for (const SkinInfo& skin : skins) {
        const bool isSelected = skin.id == currentSkinId;
        const bool isLocked = !skin.unlocked;

        auto* card = new QFrame(this);
        card->setProperty("garageCard", true);
        card->setProperty("selected", isSelected);
        card->setProperty("locked", isLocked);

        auto* cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(18, 18, 18, 18);
        cardLayout->setSpacing(18);

        auto* previewColumn = new QVBoxLayout();
        previewColumn->setSpacing(8);

        auto* previewLabel = new QLabel(QStringLiteral("Preview"), card);
        previewLabel->setProperty("garageEyebrow", true);
        previewColumn->addWidget(previewLabel, 0, Qt::AlignLeft);

        auto* preview = new SkinPreviewWidget(skin.id, isSelected, isLocked, card);
        previewColumn->addWidget(preview);
        cardLayout->addLayout(previewColumn, 0);

        auto* contentColumn = new QVBoxLayout();
        contentColumn->setSpacing(8);

        auto* eyebrow = new QLabel(QStringLiteral("VEHICLE SKIN"), card);
        eyebrow->setProperty("garageEyebrow", true);
        contentColumn->addWidget(eyebrow, 0, Qt::AlignLeft);

        auto* nameRow = new QHBoxLayout();
        nameRow->setContentsMargins(0, 0, 0, 0);
        nameRow->setSpacing(10);

        auto* nameLabel = new QLabel(skin.name, card);
        nameLabel->setProperty("garageName", true);
        nameRow->addWidget(nameLabel, 0, Qt::AlignLeft);

        const QString badgeText = isSelected
            ? QStringLiteral("SELECTED")
            : (skin.unlocked ? QStringLiteral("UNLOCKED") : QStringLiteral("LOCKED"));
        auto* badgeLabel = new QLabel(badgeText, card);
        badgeLabel->setProperty("garageBadge", true);
        badgeLabel->setProperty("selected", isSelected);
        badgeLabel->setProperty("locked", isLocked);
        nameRow->addWidget(badgeLabel, 0, Qt::AlignLeft);
        nameRow->addStretch(1);
        contentColumn->addLayout(nameRow);

        auto* metaLabel = new QLabel(
            QStringLiteral("ID: %1    Price: %2 coins")
                .arg(skin.id, QString::number(skin.price)),
            card);
        metaLabel->setProperty("garageMeta", true);
        contentColumn->addWidget(metaLabel);

        auto* themeLabel = new QLabel(
            QStringLiteral("Theme: %1")
                .arg(themeLabelForSkin(skin.id)),
            card);
        themeLabel->setProperty("garageMeta", true);
        contentColumn->addWidget(themeLabel);

        auto* hintLabel = new QLabel(
            isSelected
                ? QStringLiteral("This is your active car skin for the current profile.")
                : (skin.unlocked
                    ? QStringLiteral("Unlocked and ready. Switch to this skin instantly.")
                    : QStringLiteral("Earn more coins in Coin Challenge to unlock this skin.")),
            card);
        hintLabel->setProperty("garageMeta", true);
        hintLabel->setWordWrap(true);
        contentColumn->addWidget(hintLabel);

        cardLayout->addLayout(contentColumn, 1);

        auto* actionColumn = new QVBoxLayout();
        actionColumn->setSpacing(10);
        actionColumn->addStretch(1);

        auto* actionButton = new QPushButton(card);
        actionButton->setMinimumWidth(150);
        if (isSelected) {
            actionButton->setText(QStringLiteral("Selected"));
            actionButton->setEnabled(false);
        } else if (skin.unlocked) {
            actionButton->setText(QStringLiteral("Select"));
            connect(actionButton, &QPushButton::clicked, this, [this, skin]() {
                emit selectRequested(skin.id);
            });
        } else if (coins >= skin.price) {
            actionButton->setText(QStringLiteral("Buy"));
            connect(actionButton, &QPushButton::clicked, this, [this, skin]() {
                emit purchaseRequested(skin.id);
            });
        } else {
            actionButton->setText(QStringLiteral("Not Enough Coins"));
            actionButton->setEnabled(false);
        }
        actionColumn->addWidget(actionButton, 0, Qt::AlignRight);
        actionColumn->addStretch(1);
        cardLayout->addLayout(actionColumn);

        m_skinListLayout->addWidget(card);
    }

    m_skinListLayout->addStretch(1);
}

} // namespace PhantomDrive
