#include "UI/ABDrivingReportWidget.h"

#include <QApplication>
#include <QDateTime>
#include <QTimer>

using namespace PhantomDrive;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ABDrivingReportWidget widget;
    widget.setWindowTitle("AB Driving Report Widget Demo");
    widget.resize(1100, 900);

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (int i = 0; i < 35; ++i) {
        const qreal speed = 48.0 + (i % 9) * 4.5;
        widget.addSpeedData(speed, i);
    }

    ReportSummary summary;
    summary.totalScore = 86;
    summary.grade = "A";
    summary.averageSpeed = 66.4;
    summary.maxSpeed = 92.0;
    summary.coachAdvice = "Keep braking earlier before speed-limit zones. Lane control is stable, and the overall safety score is good.";
    summary.violations.append(ViolationItem{
        "SpeedOverLimit",
        "Exceeded the 60 km/h speed limit near the training zone.",
        78.5,
        60.0,
        5,
        now - 45000
    });
    summary.violations.append(ViolationItem{
        "RedLight",
        "Entered the junction while the traffic light was red.",
        32.0,
        0.0,
        8,
        now - 26000
    });
    widget.setReportSummary(summary);

    QList<ReportSummary> history;
    for (int i = 0; i < 8; ++i) {
        ReportSummary report;
        report.totalScore = 72 + i * 3;
        report.grade = report.totalScore >= 85 ? "A" : "B";
        report.averageSpeed = 58 + i;
        report.maxSpeed = 84 + i;
        history.append(report);
    }
    history.append(summary);
    widget.setHistoryReports(history);

    widget.show();

    QTimer liveTimer;
    int tick = 36;
    QObject::connect(&liveTimer, &QTimer::timeout, [&widget, &tick]() {
        const qreal speed = 55.0 + (tick % 12) * 3.0;
        widget.addSpeedData(speed, tick++);
    });
    liveTimer.start(1000);

    return app.exec();
}
