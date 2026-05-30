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

// ==================== 获取样式表 ====================
static QString getStyleSheet(const QString& theme = "dark") {
    if (theme == "light") {
        return lightTheme();
    } else if (theme == "racing") {
        return racingTheme();
    }
    return darkTheme();  // 默认深色主题
}

} // namespace ThemeManager
