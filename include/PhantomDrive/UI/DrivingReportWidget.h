#ifndef DRIVINGREPORTWIDGET_H
#define DRIVINGREPORTWIDGET_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QTableWidget>
#include <QLabel>
#include <QTimer>

// Qt 6 必须明确命名空间
QT_USE_NAMESPACE

class DrivingReportWidget : public QWidget {
    Q_OBJECT
public:
    explicit DrivingReportWidget(QWidget *parent = nullptr);

private slots:
    void updateMockData();

private:
    QSplineSeries *m_series;
    QChartView *m_chartView;
    QTableWidget *m_table;
    QLabel *m_avgSpeedLabel;
    QTimer *m_timer;
    int m_tick;
};

#endif