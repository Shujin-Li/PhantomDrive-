#include "UI/CustomTrackEditorWidget.h"

#include "track/Checkpoint.h"
#include "track/TrackTile.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QLinearGradient>
#include <QSizePolicy>
#include <QUuid>

namespace PhantomDrive {

namespace {

constexpr qreal WorldTileSize = 64.0;
constexpr int ToolbarHeight = 188;
constexpr int ToolbarMargin = 12;
constexpr int ButtonHeight = 46;
constexpr int ButtonGap = 10;
constexpr int BottomStatusHeight = 42;

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
    , m_currentBrush(CustomTrackBrush::Road)
    , m_tileSize(32.0)
{
    setMinimumSize(1040, 700);
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
        button->setMinimumSize(86, ButtonHeight);
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

    m_playButton->setToolTip(QStringLiteral("Validate and race this custom track"));
    m_saveButton->setToolTip(QStringLiteral("Save .pdtrack"));
    m_loadButton->setToolTip(QStringLiteral("Load .pdtrack or .json"));
    m_exportButton->setToolTip(QStringLiteral("Export current track as JSON"));
    m_backButton->setToolTip(QStringLiteral("Back to main menu"));

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
    int y = 78;

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
    const int brushWidth = qBound(92, (contentWidth - ButtonGap * (brushes.size() - 1)) / brushes.size(), 150);
    for (int i = 0; i < brushes.size(); ++i) {
        QPushButton* button = m_brushButtons.value(brushes.at(i));
        if (button) {
            button->setGeometry(x, y, brushWidth, ButtonHeight);
            x += brushWidth + ButtonGap;
        }
    }

    x = contentLeft;
    y = 132;
    const QList<QPushButton*> actions = {m_playButton, m_saveButton, m_loadButton, m_exportButton, m_backButton};
    const int actionWidth = qBound(130, (contentWidth - ButtonGap * (actions.size() - 1)) / actions.size(), 250);
    for (int i = 0; i < actions.size(); ++i) {
        actions.at(i)->setGeometry(x, y, actionWidth, ButtonHeight);
        x += actionWidth + ButtonGap;
    }
}

QRectF CustomTrackEditorWidget::gridRect() const
{
    const qreal availableWidth = width() - ToolbarMargin * 2.0 - 18.0;
    const qreal availableHeight = height() - ToolbarHeight - BottomStatusHeight - ToolbarMargin;
    const qreal sizeByWidth = availableWidth / EditorCols;
    const qreal sizeByHeight = availableHeight / EditorRows;
    const qreal size = qMax<qreal>(8.0, qMin(sizeByWidth, sizeByHeight));
    const qreal gridWidth = size * EditorCols;
    const qreal gridHeight = size * EditorRows;
    return QRectF(ToolbarMargin + 9.0 + (availableWidth - gridWidth) / 2.0,
                  ToolbarHeight + (availableHeight - gridHeight) / 2.0,
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

    const QRectF topBar(ToolbarMargin, ToolbarMargin, width() - ToolbarMargin * 2, 42);
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

    const QRectF toolPanel(ToolbarMargin, 62, width() - ToolbarMargin * 2, 116);
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
                             height() - BottomStatusHeight + 8,
                             width() - ToolbarMargin * 2,
                             BottomStatusHeight - 18);
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

    painter.fillRect(grid, QColor(25, 104, 45));

    for (int row = 0; row < EditorRows; ++row) {
        for (int col = 0; col < EditorCols; ++col) {
            const QRectF cell(grid.left() + col * m_tileSize,
                              grid.top() + row * m_tileSize,
                              m_tileSize,
                              m_tileSize);
            const TrackTile* tile = m_track->getTileAt(row, col);
            const TileType tileType = tile ? tile->getType() : TileType::Unknown;
            QLinearGradient tileFill(cell.topLeft(), cell.bottomRight());
            const QColor baseTileColor = tileColor(tileType);
            if (tileType == TileType::Grass || tileType == TileType::Unknown) {
                const int variant = qAbs((row * 17 + col * 23) % 5);
                tileFill.setColorAt(0.0, QColor(54 + variant * 4, 128 + variant * 5, 48));
                tileFill.setColorAt(0.55, baseTileColor);
                tileFill.setColorAt(1.0, QColor(18, 72 + variant * 5, 34));
                painter.fillRect(cell, tileFill);
            } else if (tileType == TileType::Wall || tileType == TileType::Barrier) {
                tileFill.setColorAt(0.0, QColor(157, 160, 158));
                tileFill.setColorAt(0.52, baseTileColor);
                tileFill.setColorAt(1.0, QColor(78, 80, 82));
                painter.fillRect(cell, tileFill);
            } else if (tileType == TileType::Road || tileType == TileType::Asphalt) {
                tileFill.setColorAt(0.0, QColor(54, 56, 58));
                tileFill.setColorAt(0.55, baseTileColor);
                tileFill.setColorAt(1.0, QColor(24, 25, 27));
                painter.fillRect(cell, tileFill);
            } else {
                painter.fillRect(cell, baseTileColor);
            }

            if (tileType == TileType::Road || tileType == TileType::Asphalt) {
                painter.fillRect(cell.adjusted(2, 2, -2, -2), QColor(39, 42, 44, 210));
                painter.fillRect(cell.adjusted(cell.width() * 0.46, 0, -cell.width() * 0.46, 0),
                                 QColor(216, 214, 194, 30));
                painter.setPen(QPen(QColor(10, 11, 12, 58), 1));
                painter.drawLine(QPointF(cell.left(), cell.top() + cell.height() * 0.18),
                                 QPointF(cell.right(), cell.top() + cell.height() * 0.18));
                painter.drawLine(QPointF(cell.left(), cell.bottom() - cell.height() * 0.16),
                                 QPointF(cell.right(), cell.bottom() - cell.height() * 0.16));
                painter.setPen(QPen(QColor(188, 190, 178, 54), 1));
                for (int i = 0; i < 3; ++i) {
                    const int seed = row * 31 + col * 47 + i * 13;
                    painter.drawPoint(QPointF(cell.left() + ((seed % 78) + 11) / 100.0 * cell.width(),
                                              cell.top() + (((seed * 3) % 72) + 14) / 100.0 * cell.height()));
                }
            } else if (tileType == TileType::Wall || tileType == TileType::Barrier) {
                painter.fillRect(cell.adjusted(2, 2, -2, -2), QColor(172, 174, 170, 84));
                painter.setPen(QPen(QColor(47, 50, 52, 130), 1));
                const qreal blockH = cell.height() / 3.0;
                painter.drawLine(QPointF(cell.left(), cell.top() + blockH), QPointF(cell.right(), cell.top() + blockH));
                painter.drawLine(QPointF(cell.left(), cell.top() + blockH * 2.0), QPointF(cell.right(), cell.top() + blockH * 2.0));
                painter.drawLine(QPointF(cell.center().x(), cell.top()), QPointF(cell.center().x(), cell.top() + blockH));
                painter.drawLine(QPointF(cell.left() + cell.width() * 0.34, cell.top() + blockH),
                                 QPointF(cell.left() + cell.width() * 0.34, cell.top() + blockH * 2.0));
                painter.drawLine(QPointF(cell.left() + cell.width() * 0.66, cell.top() + blockH * 2.0),
                                 QPointF(cell.left() + cell.width() * 0.66, cell.bottom()));
                painter.setPen(QPen(QColor(238, 233, 205, 78), 1));
                painter.drawLine(cell.topLeft() + QPointF(3, 3), cell.topRight() + QPointF(-3, 3));
                painter.setPen(QPen(QColor(33, 35, 36, 125), 1));
                painter.drawLine(QPointF(cell.left() + cell.width() * 0.22, cell.top() + cell.height() * 0.30),
                                 QPointF(cell.left() + cell.width() * 0.38, cell.top() + cell.height() * 0.48));
            } else if (tileType == TileType::Grass || tileType == TileType::Unknown) {
                painter.setPen(QPen(QColor(20, 74, 30, 92), 1));
                for (int i = 0; i < 5; ++i) {
                    const int seed = row * 37 + col * 41 + i * 11;
                    const qreal x = cell.left() + ((seed % 72) + 14) / 100.0 * cell.width();
                    const qreal y = cell.top() + (((seed * 5) % 68) + 16) / 100.0 * cell.height();
                    const qreal blade = cell.height() * (0.08 + (i % 2) * 0.03);
                    painter.drawLine(QPointF(x, y + blade), QPointF(x + ((i % 2) ? -blade * 0.34 : blade * 0.30), y - blade));
                }
                painter.setPen(QPen(QColor(154, 190, 83, 82), 1));
                painter.drawArc(cell.adjusted(cell.width() * 0.18, cell.height() * 0.24,
                                              -cell.width() * 0.48, -cell.height() * 0.48),
                                15 * 16, 130 * 16);
            }
            painter.setPen(QPen(QColor(50, 205, 220, 42), 1));
            painter.drawRect(cell);

            if (hasStartAt(row, col)) {
                painter.save();
                const qreal radius = qMax<qreal>(8.0, qMin(cell.width(), cell.height()) * 0.28);
                const QRectF marker(cell.center().x() - radius,
                                    cell.center().y() - radius,
                                    radius * 2.0,
                                    radius * 2.0);
                painter.setBrush(QColor(0, 210, 255, 225));
                painter.setPen(QPen(QColor(226, 255, 255), 2));
                painter.drawEllipse(marker);
                painter.setPen(Qt::white);
                painter.setFont(QFont(QStringLiteral("Arial"), 8, QFont::Bold));
                painter.drawText(marker, Qt::AlignCenter, QStringLiteral("S"));
                painter.restore();
            }

            if (isFinishAt(row, col)) {
                painter.save();
                const QRectF marker = cell.adjusted(cell.width() * 0.18,
                                                    cell.height() * 0.18,
                                                    -cell.width() * 0.18,
                                                    -cell.height() * 0.18);
                painter.setBrush(QColor(255, 255, 255, 235));
                painter.setPen(QPen(QColor(255, 77, 109), 2));
                painter.drawRoundedRect(marker, 4, 4);
                painter.setPen(QColor(255, 60, 90));
                painter.setFont(QFont(QStringLiteral("Arial"), 8, QFont::Bold));
                painter.drawText(marker, Qt::AlignCenter, QStringLiteral("FIN"));
                painter.restore();
            }

            if (hasItemBoxAt(row, col)) {
                painter.save();
                painter.setBrush(QColor(156, 92, 255, 230));
                painter.setPen(QPen(QColor(236, 225, 255), 2));
                painter.drawRoundedRect(cell.adjusted(8, 8, -8, -8), 4, 4);
                painter.setPen(Qt::white);
                painter.setFont(QFont(QStringLiteral("Arial"), 10, QFont::Bold));
                painter.drawText(cell, Qt::AlignCenter, QStringLiteral("?"));
                painter.restore();
            }

            if (Checkpoint* cp = checkpointAt(row, col)) {
                painter.save();
                painter.setBrush(QColor(255, 214, 64, 72));
                painter.setPen(QPen(QColor(255, 214, 64), 3));
                painter.drawRoundedRect(cell.adjusted(5, 5, -5, -5), 5, 5);
                painter.setPen(Qt::white);
                painter.setFont(QFont(QStringLiteral("Arial"), 9, QFont::Bold));
                painter.drawText(cell, Qt::AlignCenter, QStringLiteral("CP%1").arg(cp->getIndexInRoute() + 1));
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

    return tile->getType() == TileType::FinishLine || tile->getType() == TileType::StartLine;
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
        return QColor(37, 39, 42);
    case TileType::Grass:
        return QColor(39, 121, 51);
    case TileType::Sand:
        return QColor(217, 167, 63);
    case TileType::Asphalt:
        return QColor(34, 37, 40);
    case TileType::StartLine:
        return QColor(0, 204, 255);
    case TileType::FinishLine:
        return QColor(255, 242, 250);
    case TileType::Wall:
    case TileType::Barrier:
        return QColor(120, 123, 124);
    default:
        return QColor(32, 107, 47);
    }
}

} // namespace PhantomDrive
