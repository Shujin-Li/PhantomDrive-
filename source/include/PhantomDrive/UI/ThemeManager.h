/*
 * PhantomDrive 统一样式表 (QSS)
 * 版本: 1.0
 * 
 * 使用方法:
 *   qApp->setStyleSheet(ThemeManager::getStyleSheet());
 *   或者在 Qt Designer 中设置样式表为: url(:/styles/phantom_theme.qss);
 */

#pragma once

#include <QString>

namespace ThemeManager {

static const QString mainMenuNeonQss() {
    return R"(
        QStackedWidget {
            background-color: #050816;
        }

        QWidget#pageMenu {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #020613,
                stop:0.48 #06152a,
                stop:1 #10051e);
            border: none;
        }

        QWidget#pageMenu QLabel#label_MainLogo {
            color: #F7FBFF;
            font-size: 54px;
            font-weight: 900;
            letter-spacing: 2px;
            padding: 20px 0 4px 0;
            border-bottom: 2px solid rgba(0, 224, 255, 150);
        }

        QWidget#pageMenu QLabel#label_AIDifficulty {
            color: #00E6FF;
            font-size: 13px;
            font-weight: 800;
            letter-spacing: 3px;
            padding: 4px 0 0 0;
        }

        QWidget#pageMenu QComboBox {
            color: #EAFBFF;
            background-color: rgba(5, 16, 38, 230);
            border: 2px solid rgba(0, 224, 255, 150);
            border-radius: 8px;
            padding: 10px 18px;
            font-size: 18px;
            font-weight: 800;
            selection-background-color: rgba(0, 224, 255, 90);
        }

        QWidget#pageMenu QComboBox:hover {
            border-color: #1FF4FF;
            background-color: rgba(8, 30, 62, 240);
        }

        QWidget#pageMenu QComboBox::drop-down {
            width: 48px;
            border: none;
            border-left: 1px solid rgba(0, 224, 255, 80);
        }

        QWidget#pageMenu QComboBox QAbstractItemView {
            color: #EAFBFF;
            background-color: #07162D;
            border: 1px solid rgba(0, 224, 255, 160);
            selection-background-color: rgba(0, 224, 255, 80);
            outline: none;
        }

        QWidget#pageMenu QPushButton {
            color: #F1FCFF;
            background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(16, 40, 82, 245),
                stop:1 rgba(4, 13, 34, 245));
            border: 2px solid rgba(0, 224, 255, 125);
            border-radius: 8px;
            padding: 12px 26px;
            font-size: 18px;
            font-weight: 900;
            min-width: 420px;
        }

        QWidget#pageMenu QPushButton:hover {
            color: #FFFFFF;
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(0, 190, 255, 95),
                stop:1 rgba(116, 76, 255, 115));
            border: 2px solid #28F2FF;
        }

        QWidget#pageMenu QPushButton:pressed {
            background-color: rgba(0, 120, 170, 150);
            border-color: #8DFFFF;
        }

        QWidget#pageMenu QPushButton#btn_Arcade {
            border-color: rgba(255, 54, 101, 165);
            color: #FFEAF0;
        }

        QWidget#pageMenu QPushButton#btn_Arcade:hover {
            background-color: rgba(255, 54, 101, 120);
            border-color: #FF4F7E;
        }

        QWidget#pageMenu QPushButton#btn_Learn {
            border-color: rgba(0, 255, 170, 145);
            color: #E9FFF7;
        }

        QWidget#pageMenu QPushButton#btn_Learn:hover {
            background-color: rgba(0, 255, 170, 80);
            border-color: #39FFC6;
        }

        QWidget#pageMenu QPushButton#btn_CustomTrackMode,
        QWidget#pageMenu QPushButton#btn_LoadCustomTrack,
        QWidget#pageMenu QPushButton#btn_History {
            border-color: rgba(0, 180, 255, 150);
        }

        QWidget#pageMenu QPushButton#btn_Exit {
            color: #FF6B8A;
            border-color: rgba(255, 54, 101, 150);
            background-color: rgba(30, 5, 18, 230);
        }

        QWidget#pageMenu QPushButton#btn_Exit:hover {
            color: #FFFFFF;
            background-color: rgba(180, 20, 50, 170);
            border-color: #FF3366;
        }
    )";
}

// ==================== 深色主题（推荐用于赛车游戏） ====================
static const QString darkTheme() {
    return R"(
        /* ==================== 全局样式 ==================== */
        QWidget {
            font-family: 'Segoe UI', 'Microsoft YaHei', 'PingFang SC', sans-serif;
            font-size: 13px;
        }

        /* ==================== 主窗口 ==================== */
        QMainWindow {
            background-color: #1a1a2e;
        }

        /* ==================== 面板卡片 ==================== */
        QFrame#cardPanel {
            background-color: #16213e;
            border-radius: 12px;
            border: 1px solid #0f3460;
            padding: 10px;
        }

        QFrame#titlePanel {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #1a1a2e, stop:1 #16213e);
            border-radius: 8px;
            border-bottom: 2px solid #e94560;
            padding: 8px 15px;
        }

        /* ==================== 按钮 ==================== */
        QPushButton {
            background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3a506b, stop:1 #2c3e50);
            color: #ecf0f1;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 14px;
            font-weight: bold;
            min-width: 80px;
        }

        QPushButton:hover {
            background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #4a607b, stop:1 #3c4e60);
            border: 1px solid #5a708b;
        }

        QPushButton:pressed {
            background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2a405b, stop:1 #1c2e40);
        }

        QPushButton:disabled {
            background-color: #34495e;
            color: #7f8c8d;
        }

        /* 主要按钮（强调色） */
        QPushButton#primaryButton {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #e94560, stop:1 #c23a51);
        }

        QPushButton#primaryButton:hover {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #f95570, stop:1 #d24a61);
        }

        /* 次要按钮 */
        QPushButton#secondaryButton {
            background-color: transparent;
            border: 2px solid #3498db;
            color: #3498db;
        }

        QPushButton#secondaryButton:hover {
            background-color: rgba(52, 152, 219, 50);
        }

        /* 危险按钮 */
        QPushButton#dangerButton {
            background-color: #c0392b;
        }

        QPushButton#dangerButton:hover {
            background-color: #e74c3c;
        }

        /* ==================== 输入框 ==================== */
        QLineEdit {
            background-color: #0f3460;
            color: #ecf0f1;
            border: 2px solid #1a1a2e;
            border-radius: 6px;
            padding: 8px 12px;
            selection-background-color: #e94560;
        }

        QLineEdit:focus {
            border: 2px solid #e94560;
        }

        QLineEdit:disabled {
            background-color: #16213e;
            color: #7f8c8d;
        }

        /* ==================== 标签 ==================== */
        QLabel {
            color: #ecf0f1;
        }

        QLabel#titleLabel {
            font-size: 24px;
            font-weight: bold;
            color: #ffffff;
        }

        QLabel#subtitleLabel {
            font-size: 16px;
            color: #bdc3c7;
        }

        QLabel#valueLabel {
            font-size: 28px;
            font-weight: bold;
            color: #2ecc71;
        }

        QLabel#warningLabel {
            color: #f39c12;
        }

        QLabel#dangerLabel {
            color: #e74c3c;
        }

        /* ==================== 表格 ==================== */
        QTableWidget {
            background-color: #16213e;
            alternate-background-color: #1a1a2e;
            color: #ecf0f1;
            gridline-color: #0f3460;
            border: none;
            border-radius: 8px;
            padding: 5px;
        }

        QTableWidget::item {
            padding: 8px;
            border-bottom: 1px solid #0f3460;
        }

        QTableWidget::item:selected {
            background-color: #e94560;
            color: #ffffff;
        }

        QTableWidget::item:hover {
            background-color: rgba(233, 69, 96, 50);
        }

        QHeaderView::section {
            background-color: #0f3460;
            color: #ecf0f1;
            padding: 10px;
            font-weight: bold;
            font-size: 13px;
            border: none;
            border-bottom: 2px solid #e94560;
        }

        /* ==================== 滚动条 ==================== */
        QScrollBar:vertical {
            background-color: #1a1a2e;
            width: 12px;
            border-radius: 6px;
            margin: 0px;
        }

        QScrollBar::handle:vertical {
            background-color: #3a506b;
            border-radius: 6px;
            min-height: 30px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: #4a607b;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QScrollBar:horizontal {
            background-color: #1a1a2e;
            height: 12px;
            border-radius: 6px;
        }

        QScrollBar::handle:horizontal {
            background-color: #3a506b;
            border-radius: 6px;
            min-width: 30px;
        }

        /* ==================== Tab 组件 ==================== */
        QTabWidget::pane {
            background-color: #16213e;
            border: 1px solid #0f3460;
            border-radius: 8px;
            padding: 10px;
        }

        QTabBar::tab {
            background-color: #1a1a2e;
            color: #bdc3c7;
            padding: 10px 20px;
            margin-right: 2px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            font-weight: bold;
        }

        QTabBar::tab:selected {
            background-color: #e94560;
            color: #ffffff;
        }

        QTabBar::tab:hover:!selected {
            background-color: #2c3e50;
        }

        /* ==================== 进度条 ==================== */
        QProgressBar {
            background-color: #0f3460;
            color: #ecf0f1;
            border: none;
            border-radius: 8px;
            text-align: center;
            height: 20px;
        }

        QProgressBar::chunk {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2ecc71, stop:0.5 #27ae60, stop:1 #2ecc71);
            border-radius: 8px;
        }

        /* ==================== 滑块 ==================== */
        QSlider::groove:horizontal {
            border: 1px solid #0f3460;
            height: 8px;
            background-color: #16213e;
            border-radius: 4px;
        }

        QSlider::handle:horizontal {
            background-color: #e94560;
            border: none;
            width: 18px;
            margin: -5px 0;
            border-radius: 9px;
        }

        QSlider::handle:horizontal:hover {
            background-color: #f95570;
        }

        /* ==================== 复选框 ==================== */
        QCheckBox {
            color: #ecf0f1;
            spacing: 8px;
        }

        QCheckBox::indicator {
            width: 20px;
            height: 20px;
            border-radius: 4px;
            border: 2px solid #3498db;
            background-color: #16213e;
        }

        QCheckBox::indicator:checked {
            background-color: #3498db;
            image: none;
        }

        QCheckBox::indicator:hover {
            border-color: #5dade2;
        }

        /* ==================== 单选按钮 ==================== */
        QRadioButton {
            color: #ecf0f1;
            spacing: 8px;
        }

        QRadioButton::indicator {
            width: 20px;
            height: 20px;
            border-radius: 10px;
            border: 2px solid #3498db;
            background-color: #16213e;
        }

        QRadioButton::indicator:checked {
            background-color: #3498db;
        }

        /* ==================== 组合框 ==================== */
        QComboBox {
            background-color: #0f3460;
            color: #ecf0f1;
            border: 2px solid #1a1a2e;
            border-radius: 6px;
            padding: 8px 12px;
            min-width: 100px;
        }

        QComboBox:hover {
            border: 2px solid #3498db;
        }

        QComboBox::drop-down {
            border: none;
            width: 30px;
        }

        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 8px solid #ecf0f1;
            margin-right: 10px;
        }

        QComboBox QAbstractItemView {
            background-color: #16213e;
            color: #ecf0f1;
            border: 1px solid #0f3460;
            selection-background-color: #e94560;
            outline: 0;
        }

        /* ==================== 消息框 ==================== */
        QMessageBox {
            background-color: #16213e;
        }

        QMessageBox QLabel {
            color: #ecf0f1;
            font-size: 14px;
        }

        QMessageBox QPushButton {
            min-width: 80px;
        }

        /* ==================== 工具提示 ==================== */
        QToolTip {
            background-color: #0f3460;
            color: #ecf0f1;
            border: 1px solid #3498db;
            border-radius: 6px;
            padding: 6px 10px;
            font-size: 12px;
        }

        /* ==================== 菜单 ==================== */
        QMenu {
            background-color: #16213e;
            border: 1px solid #0f3460;
            border-radius: 6px;
            padding: 5px;
        }

        QMenu::item {
            color: #ecf0f1;
            padding: 8px 30px 8px 20px;
            border-radius: 4px;
        }

        QMenu::item:selected {
            background-color: #e94560;
        }

        QMenu::separator {
            height: 1px;
            background-color: #0f3460;
            margin: 5px 0;
        }

        /* ==================== 状态栏 ==================== */
        QStatusBar {
            background-color: #0f3460;
            color: #bdc3c7;
        }

        QStatusBar::item {
            border: none;
        }

        /* ==================== 工具栏 ==================== */
        QToolBar {
            background-color: #16213e;
            border: none;
            spacing: 5px;
            padding: 5px;
        }

        QToolBar::separator {
            background-color: #0f3460;
            width: 1px;
            margin: 5px;
        }

        /* ==================== Dock 窗口 ==================== */
        QDockWidget {
            background-color: #16213e;
            border: 1px solid #0f3460;
            titlebar-close-icon: url(none);
            titlebar-normal-icon: url(none);
        }

        QDockWidget::title {
            background-color: #1a1a2e;
            color: #ecf0f1;
            padding: 8px;
            border-bottom: 1px solid #0f3460;
        }

        /* ==================== 图表 ==================== */
        QChart {
            background-color: transparent;
        }
    )";
}

// ==================== 浅色主题（备用） ====================
static const QString lightTheme() {
    return R"(
        QWidget {
            font-family: 'Segoe UI', 'Microsoft YaHei', 'PingFang SC', sans-serif;
            font-size: 13px;
        }

        QMainWindow {
            background-color: #ecf0f1;
        }

        QPushButton {
            background-color: #3498db;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #2980b9;
        }

        QPushButton:disabled {
            background-color: #bdc3c7;
        }

        QLineEdit {
            background-color: white;
            border: 2px solid #bdc3c7;
            border-radius: 6px;
            padding: 8px;
        }

        QLineEdit:focus {
            border: 2px solid #3498db;
        }

        QLabel {
            color: #2c3e50;
        }

        QTableWidget {
            background-color: white;
            alternate-background-color: #f8f9fa;
            color: #2c3e50;
            gridline-color: #dee2e6;
        }

        QHeaderView::section {
            background-color: #34495e;
            color: white;
            padding: 10px;
            font-weight: bold;
        }

        QTabWidget::pane {
            background-color: white;
            border: 1px solid #dee2e6;
        }

        QTabBar::tab {
            background-color: #ecf0f1;
            color: #7f8c8d;
            padding: 10px 20px;
        }

        QTabBar::tab:selected {
            background-color: #3498db;
            color: white;
        }
    )";
}

// ==================== 赛车游戏专用主题（霓虹风格） ====================
static const QString racingTheme() {
    return R"(
        /* ==================== 全局 ==================== */
        QWidget {
            font-family: 'Segoe UI', 'Orbitron', 'Microsoft YaHei', sans-serif;
            font-size: 13px;
            color: #ffffff;
        }

        /* ==================== 速度面板样式 ==================== */
        #speedPanel {
            background-color: rgba(10, 10, 20, 240);
            border: 2px solid #00ffff;
            border-radius: 15px;
        }

        /* ==================== 按钮 ==================== */
        QPushButton {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ff0066, stop:1 #cc0052);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: bold;
            text-transform: uppercase;
            letter-spacing: 1px;
        }

        QPushButton:hover {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ff3377, stop:1 #dd1166);
            border: 1px solid #ff6699;
        }

        QPushButton:pressed {
            background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #aa0044, stop:1 #880033);
        }

        /* ==================== 标签（霓虹效果） ==================== */
        QLabel#neonTitle {
            font-size: 32px;
            font-weight: bold;
            color: #00ffff;
            text-shadow: 0 0 10px #00ffff, 0 0 20px #00ffff;
        }

        QLabel#speedValue {
            font-size: 64px;
            font-weight: bold;
            color: #00ff00;
            font-family: 'Orbitron', 'Consolas', monospace;
            text-shadow: 0 0 10px #00ff00, 0 0 30px #00ff00;
        }

        QLabel#warningText {
            font-size: 18px;
            font-weight: bold;
            color: #ff0000;
            text-shadow: 0 0 10px #ff0000;
        }

        /* ==================== 面板 ==================== */
        QFrame#neonPanel {
            background-color: rgba(0, 20, 40, 200);
            border: 2px solid #00ffff;
            border-radius: 10px;
        }

        QFrame#warningPanel {
            background-color: rgba(255, 0, 0, 50);
            border: 2px solid #ff0000;
            border-radius: 10px;
        }
    )";
}

// ==================== 霓虹玻璃态调色板（报告/历史面板专用） ====================
// 颜色常量，供 DrivingReportWidget 等 UI 组件直接引用
namespace Neon {
    // 背景层
    static constexpr const char* BG_BASE      = "#0a0e1a";   // 最深底色
    static constexpr const char* BG_CARD      = "rgba(14,22,44,220)";  // 卡片半透明
    static constexpr const char* BG_CARD_SOLID= "#0e1630";   // 卡片不透明版
    static constexpr const char* BG_INNER     = "rgba(8,14,32,180)";   // 内嵌区域
    // 边框/光效
    static constexpr const char* BORDER_BLUE  = "#00B4FF";   // 电光蓝边框
    static constexpr const char* BORDER_DIM   = "rgba(0,180,255,60)";  // 暗蓝边框
    static constexpr const char* BORDER_GREEN = "#00FFAA";   // 霓虹绿边框
    // 强调色
    static constexpr const char* ACCENT_BLUE  = "#00B4FF";
    static constexpr const char* ACCENT_GREEN = "#00FFAA";
    static constexpr const char* ACCENT_RED   = "#FF4466";
    static constexpr const char* ACCENT_YELLOW= "#FFD700";
    static constexpr const char* ACCENT_PURPLE= "#B44FFF";
    // 文字
    static constexpr const char* TEXT_PRIMARY  = "#E8F4FF";
    static constexpr const char* TEXT_SECONDARY= "rgba(200,220,255,160)";
    static constexpr const char* TEXT_DIM      = "rgba(160,190,220,100)";
    // 分项颜色
    static constexpr const char* SCORE_SAFETY    = "#FF4466";
    static constexpr const char* SCORE_RULE      = "#00B4FF";
    static constexpr const char* SCORE_SMOOTH    = "#00FFAA";
    static constexpr const char* SCORE_EFFICIENCY= "#FFD700";
} // namespace Neon

// ==================== 报告面板专用 QSS ====================
static const QString reportPanelQss() {
    return QStringLiteral(R"(
        /* ---- 整体背景 ---- */
        QWidget#reportRoot {
            background-color: #0a0e1a;
        }
        QScrollArea, QScrollArea > QWidget > QWidget {
            background-color: transparent;
            border: none;
        }

        /* ---- 卡片 ---- */
        QFrame#neonCard {
            background-color: rgba(14,22,44,220);
            border: 1px solid rgba(0,180,255,60);
            border-radius: 14px;
        }
        QFrame#neonCard:hover {
            border: 1px solid rgba(0,180,255,130);
        }
        QFrame#scoreSummaryCard {
            background-color: rgba(18,18,48,235);
            border: 2px solid rgba(255,54,101,150);
            border-radius: 14px;
        }
        QFrame#scoreSummaryCard:hover {
            border: 2px solid rgba(255,54,101,220);
        }

        /* ---- 卡片标题 ---- */
        QLabel#cardTitle {
            color: #00B4FF;
            font-size: 13px;
            font-weight: bold;
            letter-spacing: 1px;
            text-transform: uppercase;
        }

        /* ---- 大数字 ---- */
        QLabel#bigNumber {
            color: #E8F4FF;
            font-size: 38px;
            font-weight: bold;
            font-family: 'Consolas', 'Courier New', monospace;
        }
        QLabel#bigNumberGreen {
            color: #00FFAA;
            font-size: 38px;
            font-weight: bold;
            font-family: 'Consolas', 'Courier New', monospace;
        }
        QLabel#bigNumberRed {
            color: #FF4466;
            font-size: 38px;
            font-weight: bold;
            font-family: 'Consolas', 'Courier New', monospace;
        }
        QLabel#bigNumberYellow {
            color: #FFD700;
            font-size: 46px;
            font-weight: bold;
            font-family: 'Consolas', 'Courier New', monospace;
        }

        /* ---- 单位/副标签 ---- */
        QLabel#unitLabel {
            color: rgba(200,220,255,160);
            font-size: 11px;
        }

        /* ---- 进度条（分项得分） ---- */
        QProgressBar#scoreBar {
            background-color: rgba(8,14,32,180);
            border: none;
            border-radius: 5px;
            height: 10px;
            text-align: right;
        }
        QProgressBar#scoreBarSafety::chunk    { background-color: #FF4466; border-radius: 5px; }
        QProgressBar#scoreBarRule::chunk      { background-color: #00B4FF; border-radius: 5px; }
        QProgressBar#scoreBarSmooth::chunk    { background-color: #00FFAA; border-radius: 5px; }
        QProgressBar#scoreBarEfficiency::chunk{ background-color: #FFD700; border-radius: 5px; }

        /* ---- 违规表格 ---- */
        QTableWidget {
            background-color: rgba(8,14,32,180);
            alternate-background-color: rgba(14,22,44,180);
            color: #E8F4FF;
            gridline-color: rgba(0,180,255,30);
            border: none;
            border-radius: 8px;
            font-size: 12px;
        }
        QTableWidget::item { padding: 7px 10px; }
        QTableWidget::item:selected {
            background-color: rgba(0,180,255,60);
            color: #ffffff;
        }
        QHeaderView::section {
            background-color: rgba(0,180,255,25);
            color: #00B4FF;
            padding: 8px 10px;
            font-weight: bold;
            font-size: 11px;
            border: none;
            border-bottom: 1px solid rgba(0,180,255,80);
            letter-spacing: 1px;
        }

        /* ---- 滚动条 ---- */
        QScrollBar:vertical {
            background: transparent;
            width: 6px;
            border-radius: 3px;
        }
        QScrollBar::handle:vertical {
            background: rgba(0,180,255,80);
            border-radius: 3px;
            min-height: 24px;
        }
        QScrollBar::handle:vertical:hover { background: rgba(0,180,255,160); }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

        /* ---- AI 教练区 ---- */
        QLabel#coachText {
            color: rgba(200,220,255,200);
            font-size: 12px;
            line-height: 1.6;
            background-color: rgba(8,14,32,90);
            border-radius: 8px;
            padding: 10px 14px;
            border: 1px solid rgba(0,180,255,30);
        }

        /* ---- 占位符 ---- */
        QLabel#placeholder {
            color: rgba(160,190,220,100);
            font-size: 12px;
            font-style: italic;
        }
    )");
}

// ==================== 获取样式表 ====================
static QString getStyleSheet(const QString& theme = "dark") {
    if (theme == "light") {
        return lightTheme();
    } else if (theme == "racing") {
        return racingTheme();
    } else if (theme == "report") {
        return reportPanelQss();
    }
    return darkTheme();  // 默认深色主题
}

} // namespace ThemeManager
