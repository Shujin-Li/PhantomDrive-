#include "mainwindow.h"
#include <QApplication>
#include "DrivingReportWidget.h"


/**
 * @brief Phantom Drive - AI 驾驶教练系统
 * @note Main 入口点：仅包含应用程序初始化与全局样式设定
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // ========================================================
    // 全局 QSS 样式表 (W5 美化预演)
    // 统一定义 UI 风格，避免在代码里到处去 setStyleSheet
    // ========================================================
    a.setStyleSheet(
        "QMainWindow { background-color: #2C3E50; }" // 深色科技感背景
        "QPushButton { "
        "   background-color: #34495E; "
        "   color: #ECF0F1; "
        "   border-radius: 6px; "
        "   padding: 12px; "
        "   font-size: 14px; "
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #1ABC9C; }" // 悬停变薄荷绿
        "QLabel#label_Logo { font-size: 28px; font-weight: 900; color: #E74C3C; letter-spacing: 2px; }" // 主标题
        "QLabel { color: #ECF0F1; font-family: 'Microsoft YaHei'; }" // 默认字体颜色
        );

    MainWindow w;
    w.setWindowTitle("Phantom Drive - U-A Build");
    w.resize(1200, 800); // 建议稍微调大尺寸以容纳图表
    w.show();
    return a.exec();

}