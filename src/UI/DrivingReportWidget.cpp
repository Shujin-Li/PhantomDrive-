#include "UI/DrivingReportWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDateTime>
#include <QRandomGenerator>

DrivingReportWidget::DrivingReportWidget(QWidget *parent) : QWidget(parent), m_tick(0) {
    auto *mainLayout = new QVBoxLayout(this);

    // 顶部卡片
    m_avgSpeedLabel = new QLabel("平均时速: 0 km/h");
    m_avgSpeedLabel->setStyleSheet("background-color: #34495E; color: white; padding: 15px; border-radius: 8px; font-weight: bold;");
    mainLayout->addWidget(m_avgSpeedLabel);

    // 折线图
    m_series = new QSplineSeries();
    m_series->setName("实时车速");

    auto *chart = new QChart();
    chart->addSeries(m_series);
    chart->createDefaultAxes();
    chart->axisX()->setRange(0, 20);
    chart->axisY()->setRange(0, 120);
    chart->setBackgroundBrush(QBrush(QColor(44, 62, 80)));
    chart->setTitleBrush(QBrush(Qt::white));
    chart->setTitle("驾驶数据实时监控");

    m_chartView = new QChartView(chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(m_chartView);

    // 事件列表
    m_table = new QTableWidget(0, 2);
    m_table->setHorizontalHeaderLabels({"时间", "行驶状态"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setStyleSheet("QTableWidget { background-color: #2C3E50; color: white; }");
    mainLayout->addWidget(m_table);

    // 定时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DrivingReportWidget::updateMockData);
    m_timer->start(1000);
}

void DrivingReportWidget::updateMockData() {
    int speed = QRandomGenerator::global()->bounded(40, 90);
    m_series->append(m_tick, speed);

    if (m_tick > 20) {
        m_chartView->chart()->axisX()->setRange(m_tick - 20, m_tick);
    }

    m_avgSpeedLabel->setText(QString("当前模拟时速: %1 km/h").arg(speed));

    if (m_tick % 5 == 0) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("hh:mm:ss")));
        m_table->setItem(row, 1, new QTableWidgetItem("巡航中"));
        m_table->scrollToBottom();
    }
    m_tick++;
}