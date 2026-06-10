# A-B v2 Interface Completion

日期：2026-06-04

## 本轮补充

根据 `docs/a-b-v2-integration-guide.md`，本轮把 A-B v2 对接面补成更容易直接 signal-slot 连接的版本。

## 已补接口

### ScoreManager

文件：

- `include/PhantomDrive/scoring/ScoreManager.h`

补充内容：

- `recordViolation(const ViolationEvent& event)` 暴露为 Qt slot。
- `recordCollision(const QPointF& position, qreal speed, const QString& description)` 暴露为 Qt slot。
- `recordSafeDrivingTick(qint64 timestampMs, qreal speed)` 暴露为 Qt slot。
- `generateCoachReportAsync(const ScoreReport& report)` 暴露为 Qt slot。

这些函数原有实现保持不变，只是从普通 public API 调整为 public slots，方便 E-A / E-B / U-B / U-A 直接 connect。

### DrivingReportWidget

文件：

- `include/PhantomDrive/UI/DrivingReportWidget.h`
- `src/UI/DrivingReportWidget.cpp`

补充内容：

- 新增 `setReport(const ScoreReport& report)`，内部复用原 `setCurrentReport(report)`。

这样 U-A 可直接按文档连接：

```cpp
connect(scoreManager, &PhantomDrive::ScoreManager::reportReady,
        reportWindow, &PhantomDrive::DrivingReportWidget::setReport);
```

### ABDrivingReportWidget

文件：

- `include/PhantomDrive/UI/ABDrivingReportWidget.h`
- `src/UI/ABDrivingReportWidget.cpp`

补充内容：

- 新增 `setReport(const ScoreReport& report)`。
- 新增 `setCurrentReport(const ScoreReport& report)`。
- 新增 `setCoachReportMarkdown(const QString& markdown)`。
- 新增 `loadHistoryReports(const QList<ScoreReport>& reports)`。
- 新增内部转换函数 `summaryFromScoreReport(report)`，把 typed `ScoreReport` 转成旧控件使用的 `ReportSummary`。

这样旧 A-B 报告控件也能直接接新版 typed report。

### SaveLoadManager

文件：

- `include/PhantomDrive/core/saveloadmanager.h`
- `src/core/saveloadmanager.cpp`

补充内容：

- 新增 `saveReportJson(const QJsonObject& reportJson)` slot。
- 新增 `append(const QJsonObject& reportJson)` slot。

这样 U-B 历史保存可以直接接：

```cpp
connect(scoreManager, &PhantomDrive::ScoreManager::reportJsonReady,
        &SaveLoadManager::instance(), &SaveLoadManager::append);
```

`append()` 会复用现有 JSON 反序列化和 `saveReport()` 逻辑，不新建第二套历史格式。

## 未改变

- 未改变评分算法。
- 未改变反馈文案和扣分规则。
- 未改变 AI coach 异步生成逻辑。
- 未改动车辆物理、AI 运动或速度换算。
