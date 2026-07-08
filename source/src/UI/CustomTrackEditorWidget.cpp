#include "UI/CustomTrackEditorWidget.h"

#include "track/Checkpoint.h"
#include "track/TrackTile.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QLinearGradient>
#include <QFontMetrics>
#include <QSizePolicy>
#include <QUuid>

namespace PhantomDrive {

namespace {

constexpr qreal WorldTileSize = 64.0;
constexpr int ToolbarHeight = 178;
constexpr int ToolbarMargin = 10;
constexpr int ButtonHeight = 36;
constexpr int ButtonGap = 8;
constexpr int BottomStatusHeight = 34;
constexpr int ToolPanelTop = 54;
constexpr int ToolPanelHeight = 112;
constexpr int BrushButtonTop = 76;
constexpr int ActionButtonTop = 120;

QString brushLabel(CustomTrackBrush brush)
{
    switch (brush) {
    case CustomTrackBrush::Road:
        return QStringLiteral("Road");
    case CustomTrackBrush::Grass:
        return QStringLiteral("Grass");
    case CustomTrackBrush::Barrier:
        return QStringLiteral("Wall");
    case CustomTrackBrush::Start:
        return QStringLiteral("Start");
    case CustomTrackBrush::Finish:
        return QStringLiteral("Finish");
    case CustomTrackBrush::Checkpoint:
        return QStringLiteral("CP");
    case CustomTrackBrush::ItemBox:
        return QStringLiteral("Item");
    case CustomTrackBrush::Erase:
        return QStringLiteral("Erase");
    }

    return QString();
}

bool isPaintBrush(CustomTrackBrush brush)
{
    return brush == CustomTrackBrush::Road
        || brush == CustomTrackBrush::Grass
        || brush == CustomTrackBrush::Barrier
        || brush == CustomTrackBrush::Erase;
}

QColor tileBorderColor(TileType type)
{
    switch (type) {
    case TileType::Wall:
        return QColor(255, 238, 88, 255);
    case TileType::Barrier:
        return QColor(255, 202, 40, 255);
    case TileType::StartLine:
        return QColor(QStringLiteral("#BFFFFF"));
    case TileType::FinishLine:
        return QColor(QStringLiteral("#FFE4E6"));
    case TileType::Road:
    case TileType::Asphalt:
        return QColor(0, 229, 255, 255);
    case TileType::Grass:
        return QColor(126, 236, 164, 150);
    default:
        return QColor(QStringLiteral("#31506B"));
    }
}

void drawTileLabel(QPainter& painter, const QRectF& rect, const QString& text, int maxPointSize)
{
    int pointSize = maxPointSize;
    QFont font(QStringLiteral("Arial"), pointSize, QFont::Black);
    while (pointSize > 7) {
        font.setPointSize(pointSize);
        const QFontMetrics metrics(font);
        if (metrics.horizontalAdvance(text) <= rect.width() - 6.0
            && metrics.height() <= rect.height() - 4.0) {
            break;
        }
        --pointSize;
    }

    painter.setFont(font);
    const QColor shadow(0, 0, 0, 245);
    painter.setPen(shadow);
    painter.drawText(rect.translated(-1, 0), Qt::AlignCenter, text);
    painter.drawText(rect.translated(1, 0), Qt::AlignCenter, text);
    painter.drawText(rect.translated(0, -1), Qt::AlignCenter, text);
    painter.drawText(rect.translated(0, 1), Qt::AlignCenter, text);
    painter.setPen(QColor(255, 255, 255, 255));
    painter.drawText(rect, Qt::AlignCenter, text);
}

} // namespace

CustomTrackEditorWidget::CustomTrackEditorWidget(QWidget* parent)
    : QWidget(parent)
    , m_track(new TrackData(QStringLiteral("Custom Track"), this))
    , m_brushGroup(new QButtonGroup(this))
    , m_playButton(nullptr)
    , m_saveButton(nullptr)
    , m_loadButton(nullptr)
    , m_exportButton(nullptr)
    , m_backButton(nullptr)
    , m_playersLabel(nullptr)
    , m_aiLabel(nullptr)
    , m_playersCombo(nullptr)
    , m_aiDifficultyCombo(nullptr)
    , m_currentBrush(CustomTrackBrush::Road)
    , m_tileSize(32.0)
{
    setMinimumSize(900, 620);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral(
        "PhantomDrive--CustomTrackEditorWidget, CustomTrackEditorWidget {"
        "  background: #040813;"
        "  color: #E1FCFF;"
        "}"
        "QToolTip {"
        "  color: #EAFBFF;"
        "  background-color: #071220;"
        "  border: 1px solid #2FF0FF;"
        "}"));
    resetTrack();
    setupButtons();
}

CustomTrackEditorWidget::~CustomTrackEditorWidget()
{
}

void CustomTrackEditorWidget::setTrackData(TrackData* track)
{
    if (!track) {
        resetTrack();
        return;
    }

    if (m_track && m_track != track && m_track->parent() == this) {
        m_track->deleteLater();
    }

    m_track = track;
    if (!m_track->parent()) {
        m_track->setParent(this);
    }
    m_track->setMaxLaps(1);
    emit trackChanged(m_track);
    update();
}

void CustomTrackEditorWidget::resetTrack()
{
    if (!m_track) {
        m_track = new TrackData(QStringLiteral("Custom Track"), this);
    }

    m_track->clear();
    m_track->setId(QUuid::createUuid().toString(QUuid::WithoutBraces));
    m_track->setName(QStringLiteral("Custom Track"));
    m_track->setAuthor(QStringLiteral("Player"));
    m_track->setDescription(QStringLiteral("Tile-based custom track"));
    m_track->setDifficulty(QStringLiteral("Custom"));
    m_track->setMaxLaps(1);
    m_track->setSize(EditorRows, EditorCols);
    emit trackChanged(m_track);
    update();
}

int CustomTrackEditorWidget::selectedPlayerCount() const
{
    return m_playersCombo ? m_playersCombo->currentData().toInt() : 1;
}

QString CustomTrackEditorWidget::selectedAiDifficulty() const
{
    const QString value = m_aiDifficultyCombo ? m_aiDifficultyCombo->currentData().toString() : QString();
    return value.isEmpty() ? QStringLiteral("medium") : value;
}

void CustomTrackEditorWidget::setupButtons()
{
    const QList<CustomTrackBrush> brushes = {
        CustomTrackBrush::Road,
        CustomTrackBrush::Grass,
        CustomTrackBrush::Barrier,
        CustomTrackBrush::Start,
        CustomTrackBrush::Finish,
        CustomTrackBrush::Checkpoint,
        CustomTrackBrush::ItemBox,
        CustomTrackBrush::Erase
    };

    int id = 0;
    for (CustomTrackBrush brush : brushes) {
        auto* button = new QPushButton(brushLabel(brush), this);
        button->setCheckable(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setMinimumSize(76, ButtonHeight);
        button->setToolTip(brush == CustomTrackBrush::Finish ? QStringLiteral("Finish / StartLine")
                           : brush == CustomTrackBrush::Checkpoint ? QStringLiteral("Checkpoint")
                           : brush == CustomTrackBrush::ItemBox ? QStringLiteral("Item Box")
                           : brushLabel(brush));
        button->setStyleSheet(QStringLiteral(
            "QPushButton {"
            "  color: #EAFBFF;"
            "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 rgba(17, 39, 74, 240), stop:1 rgba(6, 16, 36, 245));"
            "  border: 1px solid rgba(44, 226, 255, 145);"
            "  border-radius: 7px;"
            "  padding: 6px 10px;"
            "  font-weight: 900;"
            "}"
            "QPushButton:hover {"
            "  background: rgba(14, 68, 105, 245);"
            "  border: 1px solid #22F6FF;"
            "}"
            "QPushButton:checked {"
            "  color: #FFFFFF;"
            "  background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #06D9FF, stop:1 #6C5CE7);"
            "  border: 2px solid #E9FFFF;"
            "}"));
        m_brushGroup->addButton(button, id++);
        m_brushButtons.insert(brush, button);
    }

    m_brushButtons.value(CustomTrackBrush::Road)->setChecked(true);
    connect(m_brushGroup, &QButtonGroup::idClicked, this, [this, brushes](int clickedId) {
        if (clickedId >= 0 && clickedId < brushes.size()) {
            m_currentBrush = brushes.at(clickedId);
        }
    });

    m_playButton = new QPushButton(QStringLiteral("Play This Track"), this);
    m_saveButton = new QPushButton(QStringLiteral("Save Track"), this);
    m_loadButton = new QPushButton(QStringLiteral("Load Track"), this);
    m_exportButton = new QPushButton(QStringLiteral("Export JSON"), this);
    m_backButton = new QPushButton(QStringLiteral("Back"), this);
    m_playersLabel = new QLabel(QStringLiteral("Players:"), this);
    m_aiLabel = new QLabel(QStringLiteral("AI:"), this);
    m_playersCombo = new QComboBox(this);
    m_aiDifficultyCombo = new QComboBox(this);

    m_playersCombo->addItem(QStringLiteral("Single Player"), 1);
    m_playersCombo->addItem(QStringLiteral("Two-Player"), 2);
    m_playersCombo->setCurrentIndex(0);
    m_aiDifficultyCombo->addItem(QStringLiteral("Easy"), QStringLiteral("easy"));
    m_aiDifficultyCombo->addItem(QStringLiteral("Normal"), QStringLiteral("medium"));
    m_aiDifficultyCombo->addItem(QStringLiteral("Hard"), QStringLiteral("hard"));
    m_aiDifficultyCombo->addItem(QStringLiteral("Adaptive"), QStringLiteral("adaptive"));
    m_aiDifficultyCombo->setCurrentIndex(1);

    m_playButton->setToolTip(QStringLiteral("Validate and race this custom track"));
    m_saveButton->setToolTip(QStringLiteral("Save .pdtrack"));
    m_loadButton->setToolTip(QStringLiteral("Load .pdtrack or .json"));
    m_exportButton->setToolTip(QStringLiteral("Export current track as JSON"));
    m_backButton->setToolTip(QStringLiteral("Back to main menu"));
    m_playersCombo->setToolTip(QStringLiteral("Choose one or two local players for this custom track run"));
    m_aiDifficultyCombo->setToolTip(QStringLiteral("Choose AI opponent difficulty for this custom track run"));

    const QString labelStyle = QStringLiteral(
        "QLabel {"
        "  color: #2FF0FF;"
        "  font-size: 11px;"
        "  font-weight: 900;"
        "}");
    m_playersLabel->setStyleSheet(labelStyle);
    m_aiLabel->setStyleSheet(labelStyle);
    const QString comboStyle = QStringLiteral(
        "QComboBox {"
        "  color: #EAFBFF;"
        "  background: rgba(7, 20, 42, 245);"
        "  border: 1px solid rgba(44, 226, 255, 150);"
        "  border-radius: 7px;"
        "  padding: 5px 28px 5px 10px;"
        "  font-weight: 800;"
        "}"
        "QComboBox:hover { border-color: #32F6FF; background: rgba(12, 40, 68, 245); }"
        "QComboBox::drop-down { width: 24px; border-left: 1px solid rgba(44, 226, 255, 80); }"
        "QComboBox QAbstractItemView {"
        "  color: #EAFBFF;"
        "  background: #071220;"
        "  selection-background-color: #0A8DA8;"
        "  border: 1px solid #2FF0FF;"
        "}");
    m_playersCombo->setStyleSheet(comboStyle);
    m_aiDifficultyCombo->setStyleSheet(comboStyle);
    m_playersCombo->setFocusPolicy(Qt::NoFocus);
    m_aiDifficultyCombo->setFocusPolicy(Qt::NoFocus);
    m_playersCombo->setMinimumHeight(ButtonHeight);
    m_aiDifficultyCombo->setMinimumHeight(ButtonHeight);

    const QList<QPushButton*> actionButtons = {m_playButton, m_saveButton, m_loadButton, m_exportButton, m_backButton};
    for (QPushButton* button : actionButtons) {
        button->setFocusPolicy(Qt::NoFocus);
        button->setMinimumHeight(ButtonHeight);
        button->setStyleSheet(QStringLiteral(
            "QPushButton {"
            "  color: #DDF7FF;"
            "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 rgba(16, 35, 68, 240), stop:1 rgba(5, 14, 34, 245));"
            "  border: 1px solid rgba(44, 226, 255, 130);"
            "  border-radius: 7px;"
            "  padding: 6px 14px;"
            "  font-weight: 900;"
            "}"
            "QPushButton:hover { background: rgba(22, 55, 82, 245); border-color: #32F6FF; }"
            "QPushButton:pressed { background: rgba(0, 170, 220, 220); }"));
    }
    m_playButton->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  color: #06121F;"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #34F5FF, stop:1 #4DFFB8);"
        "  border: 2px solid #BFFFFF;"
        "  border-radius: 9px;"
        "  padding: 6px 16px;"
        "  font-weight: 900;"
        "}"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #7AFFFF, stop:1 #78FFD2); }"
        "QPushButton:pressed { background: #16BCD6; }"));

    connect(m_playButton, &QPushButton::clicked, this, [this]() { emit playRequested(m_track); });
    connect(m_saveButton, &QPushButton::clicked, this, [this]() { emit saveRequested(m_track); });
    connect(m_loadButton, &QPushButton::clicked, this, &CustomTrackEditorWidget::loadRequested);
    connect(m_exportButton, &QPushButton::clicked, this, [this]() { emit exportJsonRequested(m_track); });
    connect(m_backButton, &QPushButton::clicked, this, &CustomTrackEditorWidget::backRequested);

    layoutButtons();
}

void CustomTrackEditorWidget::layoutButtons()
{
    const int contentLeft = ToolbarMargin + 18;
    const int contentWidth = qMax(360, width() - (ToolbarMargin + 18) * 2);
    int x = contentLeft;
    int y = BrushButtonTop;

    const QList<CustomTrackBrush> brushes = {
        CustomTrackBrush::Road,
        CustomTrackBrush::Grass,
        CustomTrackBrush::Barrier,
        CustomTrackBrush::Start,
        CustomTrackBrush::Finish,
        CustomTrackBrush::Checkpoint,
        CustomTrackBrush::ItemBox,
        CustomTrackBrush::Erase
    };
    const int brushWidth = qBound(78, (contentWidth - ButtonGap * (brushes.size() - 1)) / brushes.size(), 150);
    for (int i = 0; i < brushes.size(); ++i) {
        QPushButton* button = m_brushButtons.value(brushes.at(i));
        if (button) {
            button->setGeometry(x, y, brushWidth, ButtonHeight);
            button->raise();
            x += brushWidth + ButtonGap;
        }
    }

    x = contentLeft;
    y = ActionButtonTop;
    const QList<QPushButton*> actions = {m_playButton, m_saveButton, m_loadButton, m_exportButton, m_backButton};
    const int settingsWidth = qBound(330, contentWidth / 4, 430);
    const bool settingsFitOnActionRow = contentWidth >= 840;
    const int actionAreaWidth = settingsFitOnActionRow ? contentWidth - settingsWidth - ButtonGap * 2 : contentWidth;
    const int actionWidth = qBound(90, (actionAreaWidth - ButtonGap * (actions.size() - 1)) / actions.size(), 250);
    for (int i = 0; i < actions.size(); ++i) {
        actions.at(i)->setGeometry(x, y, actionWidth, ButtonHeight);
        actions.at(i)->raise();
        x += actionWidth + ButtonGap;
    }

    const int labelWidth = 58;
    const int comboWidth = settingsFitOnActionRow ? qMax(95, (settingsWidth - labelWidth * 2 - ButtonGap * 3) / 2) : 150;
    const int settingsY = settingsFitOnActionRow ? ActionButtonTop : ActionButtonTop;
    int settingsX = settingsFitOnActionRow ? contentLeft + actionAreaWidth + ButtonGap * 2
                                           : contentLeft + contentWidth - (labelWidth + comboWidth) * 2 - ButtonGap * 3;
    settingsX = qMax(contentLeft, settingsX);

    m_playersLabel->setGeometry(settingsX, settingsY, labelWidth, ButtonHeight);
    m_playersCombo->setGeometry(settingsX + labelWidth, settingsY, comboWidth, ButtonHeight);
    m_aiLabel->setGeometry(settingsX + labelWidth + comboWidth + ButtonGap, settingsY, labelWidth / 2, ButtonHeight);
    m_aiDifficultyCombo->setGeometry(settingsX + labelWidth + comboWidth + ButtonGap + labelWidth / 2,
                                     settingsY,
                                     comboWidth,
                                     ButtonHeight);
    m_playersLabel->raise();
    m_playersCombo->raise();
    m_aiLabel->raise();
    m_aiDifficultyCombo->raise();
}

QRectF CustomTrackEditorWidget::gridRect() const
{
    const qreal canvasLeft = ToolbarMargin + 8.0;
    const qreal canvasTop = ToolbarHeight + 8.0;
    const qreal availableWidth = qMax<qreal>(240.0, width() - canvasLeft * 2.0);
    const qreal availableHeight = qMax<qreal>(180.0, height() - canvasTop - BottomStatusHeight - ToolbarMargin * 2.0);
    const qreal sizeByWidth = availableWidth / EditorCols;
    const qreal sizeByHeight = availableHeight / EditorRows;
    const qreal size = qMax<qreal>(8.0, qMin(sizeByWidth, sizeByHeight));
    const qreal gridWidth = size * EditorCols;
    const qreal gridHeight = size * EditorRows;
    return QRectF(canvasLeft + (availableWidth - gridWidth) / 2.0,
                  canvasTop + (availableHeight - gridHeight) / 2.0,
                  gridWidth,
                  gridHeight);
}

bool CustomTrackEditorWidget::cellAt(const QPointF& pos, int& row, int& col) const
{
    const QRectF rect = gridRect();
    if (!rect.contains(pos)) {
        return false;
    }

    const qreal cellSize = rect.width() / EditorCols;
    col = static_cast<int>((pos.x() - rect.left()) / cellSize);
    row = static_cast<int>((pos.y() - rect.top()) / cellSize);
    return row >= 0 && row < EditorRows && col >= 0 && col < EditorCols;
}

QVector2D CustomTrackEditorWidget::cellCenter(int row, int col) const
{
    return TrackData::tileToWorldCenter(row, col, WorldTileSize);
}

void CustomTrackEditorWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QColor panelColor(7, 18, 32, 232);
    const QColor panelBorder(0, 220, 255, 145);
    const QColor primaryText(225, 252, 255);
    const QColor secondaryText(148, 214, 224);
    const QColor accentText(47, 240, 255);

    QLinearGradient bg(0, 0, width(), height());
    bg.setColorAt(0.0, QColor(4, 8, 19));
    bg.setColorAt(0.52, QColor(7, 21, 37));
    bg.setColorAt(1.0, QColor(19, 8, 38));
    painter.fillRect(rect(), bg);

    const QRectF topBar(ToolbarMargin, ToolbarMargin, width() - ToolbarMargin * 2, 38);
    painter.setPen(QPen(panelBorder, 1));
    painter.setBrush(panelColor);
    painter.drawRoundedRect(topBar, 12, 12);
    painter.setPen(primaryText);
    painter.setFont(QFont(QStringLiteral("Arial"), 11, QFont::Bold));
    painter.drawText(topBar.adjusted(16, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("PHANTOMDRIVE"));
    painter.setPen(accentText);
    painter.setFont(QFont(QStringLiteral("Arial"), 10, QFont::Bold));
    painter.drawText(topBar, Qt::AlignCenter, QStringLiteral("CUSTOM TRACK MODE"));
    painter.setPen(secondaryText);
    painter.setFont(QFont(QStringLiteral("Consolas"), 9));
    painter.drawText(topBar.adjusted(0, 0, -16, 0), Qt::AlignRight | Qt::AlignVCenter,
                     QStringLiteral("Speed 0 km/h  |  Limit 60 km/h  |  Light READY"));

    const QRectF toolPanel(ToolbarMargin, ToolPanelTop, width() - ToolbarMargin * 2, ToolPanelHeight);
    painter.setPen(QPen(panelBorder, 1));
    painter.setBrush(panelColor);
    painter.drawRoundedRect(toolPanel, 14, 14);
    painter.setPen(accentText);
    painter.setFont(QFont(QStringLiteral("Arial"), 8, QFont::Bold));
    painter.drawText(toolPanel.adjusted(14, 4, 0, 0), Qt::AlignLeft | Qt::AlignTop, QStringLiteral("TRACK TOOLS"));

    const QRectF grid = gridRect();
    m_tileSize = grid.width() / EditorCols;

    const QRectF canvasPanel = grid.adjusted(-14, -14, 14, 14);
    painter.setBrush(QColor(6, 18, 31, 244));
    painter.setPen(QPen(QColor(33, 235, 255, 170), 2));
    painter.drawRoundedRect(canvasPanel, 16, 16);
    painter.setPen(QPen(QColor(255, 45, 126, 78), 1));
    painter.drawRoundedRect(canvasPanel.adjusted(5, 5, -5, -5), 12, 12);

    const QRectF statusPanel(ToolbarMargin,
                             height() - BottomStatusHeight + 6,
                             width() - ToolbarMargin * 2,
                             BottomStatusHeight - 14);
    painter.setBrush(panelColor);
    painter.setPen(QPen(panelBorder, 1));
    painter.drawRoundedRect(statusPanel, 10, 10);
    painter.setPen(secondaryText);
    painter.setFont(QFont(QStringLiteral("Arial"), 10, QFont::Bold));
    painter.drawText(statusPanel.adjusted(16, 0, -16, 0),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("INFO  Draw a road, place Start, Finish, at least 2 CPs, then Play."));

    if (!m_track) {
        return;
    }

    painter.fillRect(grid, QColor(5, 12, 24, 255));
    painter.setOpacity(1.0);

    for (int row = 0; row < EditorRows; ++row) {
        for (int col = 0; col < EditorCols; ++col) {
            const QRectF cell(grid.left() + col * m_tileSize,
                              grid.top() + row * m_tileSize,
                              m_tileSize,
                              m_tileSize);
            const TrackTile* tile = m_track->getTileAt(row, col);
            const TileType tileType = tile ? tile->getType() : TileType::Unknown;
            const QColor baseTileColor = tileColor(tileType);
            const QRectF solidCell = cell.adjusted(0.0, 0.0, -0.6, -0.6);
            painter.fillRect(solidCell, baseTileColor);
            painter.setPen(QPen(QColor(7, 24, 31, 230), 1.0));
            painter.drawRect(solidCell);
            if (tileType == TileType::Grass) {
                painter.setPen(QPen(tileBorderColor(tileType), 1.0));
                painter.drawRect(solidCell.adjusted(1.2, 1.2, -1.2, -1.2));
            }
            if (tileType == TileType::Road || tileType == TileType::Asphalt
                || tileType == TileType::Wall || tileType == TileType::Barrier) {
                painter.setPen(QPen(tileBorderColor(tileType), 2.0));
                painter.drawRect(solidCell.adjusted(1.4, 1.4, -1.4, -1.4));
            }

            if (hasStartAt(row, col)) {
                painter.save();
                painter.fillRect(solidCell, QColor(8, 57, 72, 255));
                painter.setPen(QPen(QColor(0, 245, 255, 230), 1.6));
                painter.drawRect(solidCell);
                drawTileLabel(painter, solidCell, QStringLiteral("S"), 13);
                painter.restore();
            }

            if (isFinishAt(row, col)) {
                painter.save();
                painter.fillRect(solidCell, QColor(53, 18, 42, 255));
                painter.setPen(QPen(QColor(255, 79, 122, 230), 1.6));
                painter.drawRect(solidCell);
                drawTileLabel(painter, solidCell, QStringLiteral("F"), 13);
                painter.restore();
            }

            if (hasItemBoxAt(row, col)) {
                painter.save();
                painter.fillRect(solidCell, QColor(44, 32, 82, 255));
                painter.setPen(QPen(QColor(245, 208, 254, 230), 1.6));
                painter.drawRect(solidCell);
                drawTileLabel(painter, solidCell, QStringLiteral("?"), 15);
                painter.restore();
            }

            if (Checkpoint* cp = checkpointAt(row, col)) {
                Q_UNUSED(cp);
                painter.save();
                painter.fillRect(solidCell, QColor(45, 42, 28, 255));
                painter.setPen(QPen(QColor(255, 216, 80, 230), 1.6));
                painter.drawRect(solidCell);
                drawTileLabel(painter, solidCell, QStringLiteral("CP"), 11);
                painter.restore();
            }
        }
    }
}

void CustomTrackEditorWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    layoutButtons();
}

void CustomTrackEditorWidget::mousePressEvent(QMouseEvent* event)
{
    int row = -1;
    int col = -1;
    if (cellAt(event->position(), row, col)) {
        applyBrushAt(row, col);
    }
}

void CustomTrackEditorWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton) || !isPaintBrush(m_currentBrush)) {
        return;
    }

    int row = -1;
    int col = -1;
    if (cellAt(event->position(), row, col)) {
        applyBrushAt(row, col);
    }
}

void CustomTrackEditorWidget::applyBrushAt(int row, int col)
{
    switch (m_currentBrush) {
    case CustomTrackBrush::Road:
        setTileType(row, col, TileType::Road);
        break;
    case CustomTrackBrush::Grass:
        setTileType(row, col, TileType::Grass);
        break;
    case CustomTrackBrush::Barrier:
        setTileType(row, col, TileType::Barrier);
        break;
    case CustomTrackBrush::Start:
        placeStart(row, col);
        break;
    case CustomTrackBrush::Finish:
        placeFinish(row, col);
        break;
    case CustomTrackBrush::Checkpoint:
        placeCheckpoint(row, col);
        break;
    case CustomTrackBrush::ItemBox:
        placeItemBox(row, col);
        break;
    case CustomTrackBrush::Erase:
        eraseCell(row, col);
        break;
    }

    emit trackChanged(m_track);
    update();
}

void CustomTrackEditorWidget::setTileType(int row, int col, TileType type)
{
    if (!m_track) {
        return;
    }

    TrackTile* tile = m_track->getTileAt(row, col);
    if (tile) {
        tile->setType(type);
    }
}

void CustomTrackEditorWidget::placeStart(int row, int col)
{
    setTileType(row, col, TileType::Road);
    const QVector2D center = cellCenter(row, col);
    m_track->clearStartPositions();
    m_track->setStartPosition(center);
    m_track->addStartPosition(center);
    m_track->setStartRotation(0.0);
}

void CustomTrackEditorWidget::placeFinish(int row, int col)
{
    removeFinishTiles();
    setTileType(row, col, TileType::FinishLine);
}

void CustomTrackEditorWidget::placeCheckpoint(int row, int col)
{
    if (checkpointAt(row, col)) {
        return;
    }

    setTileType(row, col, TileType::Road);
    const int index = m_track->getCheckpointsInOrder().size();
    auto* cp = new Checkpoint(index + 1, cellCenter(row, col), m_track);
    cp->setIndexInRoute(index);
    cp->setWidth(WorldTileSize);
    cp->setHeight(WorldTileSize);
    m_track->addCheckpoint(cp);
}

void CustomTrackEditorWidget::placeItemBox(int row, int col)
{
    if (hasItemBoxAt(row, col)) {
        return;
    }

    m_track->addItemBoxPosition(cellCenter(row, col));
}

void CustomTrackEditorWidget::eraseCell(int row, int col)
{
    const bool hadCheckpoint = checkpointAt(row, col) != nullptr;
    const bool hadItemBox = hasItemBoxAt(row, col);
    const bool hadStart = hasStartAt(row, col);
    const bool hadFinish = isFinishAt(row, col);

    if (hadCheckpoint) {
        removeCheckpointAt(row, col);
        setTileType(row, col, TileType::Road);
        return;
    }

    if (hadItemBox) {
        m_track->removeItemBoxAt(cellCenter(row, col), WorldTileSize / 2.0);
        setTileType(row, col, TileType::Road);
        return;
    }

    if (hadStart) {
        m_track->clearStartPositions();
        m_track->setStartPosition(QVector2D());
        setTileType(row, col, TileType::Grass);
        return;
    }

    if (hadFinish) {
        setTileType(row, col, TileType::Grass);
        return;
    }

    setTileType(row, col, TileType::Grass);
}

void CustomTrackEditorWidget::renumberCheckpoints()
{
    const QList<Checkpoint*> checkpoints = m_track->getCheckpointsInOrder();
    QList<QVector2D> positions;
    for (Checkpoint* cp : checkpoints) {
        if (cp) {
            positions.append(cp->getPosition());
        }
    }

    m_track->clearCheckpoints();
    for (int i = 0; i < positions.size(); ++i) {
        auto* cp = new Checkpoint(i + 1, positions.at(i), m_track);
        cp->setIndexInRoute(i);
        cp->setWidth(WorldTileSize);
        cp->setHeight(WorldTileSize);
        m_track->addCheckpoint(cp);
    }
}

void CustomTrackEditorWidget::removeCheckpointAt(int row, int col)
{
    Checkpoint* cp = checkpointAt(row, col);
    if (!cp) {
        return;
    }

    m_track->removeCheckpoint(cp->getId());
    renumberCheckpoints();
}

void CustomTrackEditorWidget::removeFinishTiles()
{
    for (int row = 0; row < m_track->getRowCount(); ++row) {
        for (int col = 0; col < m_track->getColCount(); ++col) {
            TrackTile* tile = m_track->getTileAt(row, col);
            if (!tile) {
                continue;
            }
            if (tile->getType() == TileType::FinishLine || tile->getType() == TileType::StartLine) {
                tile->setType(TileType::Road);
            }
        }
    }
}

bool CustomTrackEditorWidget::hasItemBoxAt(int row, int col) const
{
    const QVector2D center = cellCenter(row, col);
    for (const QVector2D& pos : m_track->getItemBoxPositions()) {
        if ((pos - center).length() <= WorldTileSize / 2.0) {
            return true;
        }
    }

    return false;
}

bool CustomTrackEditorWidget::hasStartAt(int row, int col) const
{
    if (m_track->getStartPositions().isEmpty()) {
        return false;
    }

    return (m_track->getStartPositions().first() - cellCenter(row, col)).length() <= WorldTileSize / 2.0;
}

bool CustomTrackEditorWidget::isFinishAt(int row, int col) const
{
    const TrackTile* tile = m_track ? m_track->getTileAt(row, col) : nullptr;
    if (!tile) {
        return false;
    }

    return tile->getType() == TileType::FinishLine;
}

Checkpoint* CustomTrackEditorWidget::checkpointAt(int row, int col) const
{
    const QVector2D center = cellCenter(row, col);
    const QList<Checkpoint*> checkpoints = m_track->getCheckpointsInOrder();
    for (Checkpoint* cp : checkpoints) {
        if (cp && (cp->getPosition() - center).length() <= WorldTileSize / 2.0) {
            return cp;
        }
    }

    return nullptr;
}

QColor CustomTrackEditorWidget::tileColor(TileType type) const
{
    switch (type) {
    case TileType::Road:
        return QColor(8, 35, 62, 255);
    case TileType::Grass:
        return QColor(8, 43, 44, 255);
    case TileType::Sand:
        return QColor(78, 52, 16, 255);
    case TileType::Asphalt:
        return QColor(12, 33, 54, 255);
    case TileType::StartLine:
        return QColor(8, 57, 72, 255);
    case TileType::FinishLine:
        return QColor(53, 18, 42, 255);
    case TileType::Wall:
        return QColor(31, 31, 29, 255);
    case TileType::Barrier:
        return QColor(42, 36, 22, 255);
    default:
        return QColor(6, 32, 34, 255);
    }
}

} // namespace PhantomDrive
