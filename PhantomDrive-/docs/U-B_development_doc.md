# PhantomDrive U-B 模块开发文档

> 更新日期: 2026-05-18
> 模块: U-B (HUD、历史数据、图表、QSS 美化、存档)
> Qt 版本: 6.8.3

---

## 一、已完成功能概览

### 1.1 JSON 存档系统 ✅

**文件位置:**
- `include/PhantomDrive/core/saveloadmanager.h`
- `src/core/saveloadmanager.cpp`
- `include/PhantomDrive/core/datamodels.h`

**功能:**
- 完整 `ScoreReport` 序列化/反序列化
- 违规记录、教练建议、Q-Learning反馈 完整保存
- 版本兼容（支持旧版 PracticeReport 格式）
- 导入/导出功能

**API 接口:**
```cpp
// 单例访问
SaveLoadManager& mgr = SaveLoadManager::instance();

// 保存报告
bool saveReport(const ScoreReport& report);

// 加载历史
QList<ScoreReport> loadHistory();

// 删除/更新
bool deleteReport(int index);
bool updateReport(int index, const ScoreReport& newReport);

// 导入/导出到指定文件
bool exportReport(const ScoreReport& report, const QString& filePath);
bool importReport(const QString& filePath, ScoreReport& outReport);

// 信号
void historyChanged();
void reportSaved(const ScoreReport& report);
void reportDeleted(int index);
void reportUpdated(int index, const ScoreReport& newReport);
```

---

### 1.2 数据模型统一 ✅

**问题:** 存在 `PracticeReport` 和 `ScoreReport` 两个相似结构

**解决方案:** `datamodels.h` 重导出 `ScoreReport` 作为别名

```cpp
namespace PhantomDrive {
using PracticeReport = ScoreReport;
}
typedef PhantomDrive::ScoreReport PracticeReport;
```

---

### 1.3 图表可视化 ✅

**文件位置:**
- `include/PhantomDrive/UI/DrivingHistoryChartWidget.h`
- `src/UI/DrivingHistoryChartWidget.cpp`

**功能:**
- **历史趋势折线图**: 总分、安全分、规则分、平顺分随时间变化
- **分项得分柱状图**: 安全性、规则遵守、平顺性、效率
- **违规统计饼图**: 碰撞、超速、红灯、行人、逆行
- **报告列表**: 表格展示历史记录

**API 接口:**
```cpp
class DrivingHistoryChartWidget : public QWidget {
    void updateHistory(const QList<ScoreReport>& history);
    void addReport(const ScoreReport& report);
    void clear();
    QList<ScoreReport> getHistory() const;

signals:
    void reportSelected(int index, const ScoreReport& report);
};
```

---

### 1.4 LearningHUD 完善 ✅

**文件位置:**
- `include/PhantomDrive/UI/learninghud.h`
- `src/UI/learninghud.cpp`

**功能:**
- **速度显示**: 大字体实时速度，速度颜色随状态变化
- **限速显示**: 根据限速值显示不同颜色
- **交通信号**: 红/黄/绿灯状态和倒计时
- **违规警告**: 飘字动画、闪烁警告
- **道具状态**: 显示当前激活的道具
- **游戏模式**: Arcade/Learning 模式标签
- **圈数信息**: 当前圈数/总圈数

**API 接口:**
```cpp
class LearningHUD : public QWidget {
    // 速度
    void updateCurrentSpeed(qreal speed);
    void updateSpeedLimit(int limit);
    void updateSpeedStatus(bool isOverLimit);

    // 交通
    void updateTrafficLight(const QString& state, int remainingSeconds = 0);

    // 违规
    void showPenaltyMessage(const QString& message, int points);
    void showViolationWarning(const QString& violationType);

    // 道具
    void updatePowerupState(const QString& powerupId, const QString& name, bool active);

    // 模式
    void updateGameMode(const QString& mode);

    // 比赛
    void updateLapInfo(int currentLap, int totalLaps);

signals:
    void speedLimitChanged(int limit);
    void violationTriggered(const QString& type);
};
```

---

### 1.5 DrivingReportWidget 增强 ✅

**文件位置:**
- `include/PhantomDrive/UI/DrivingReportWidget.h`
- `src/UI/DrivingReportWidget.cpp`

**功能:**
- **实时速度图表**: 折线图显示实时车速
- **统计数据面板**: 当前/平均/最高速度、总分、等级、违规次数
- **分项得分柱状图**: 安全、规则、平顺、效率
- **违规事件表**: 详细违规记录列表
- **真实数据接口**: 从 E-B/E-B 接收真实数据

**API 接口:**
```cpp
class DrivingReportWidget : public QWidget {
    void addSpeedData(qreal speed, qint64 timestamp = -1);
    void setCurrentReport(const ScoreReport& report);
    void addViolationEvent(const ViolationEvent& violation);
    void clearData();
    void setMockDataEnabled(bool enabled);

signals:
    void reportUpdated(const ScoreReport& report);
};
```

---

### 1.6 QSS 主题系统 ✅

**文件位置:**
- `include/PhantomDrive/UI/ThemeManager.h`

**功能:**
- **三种主题**: 深色(默认)、浅色、赛车(霓虹)
- **完整样式覆盖**: 按钮、输入框、表格、Tab、进度条等
- **统一颜色系统**: 主色调、次色调、强调色
- **组件级定制**: 速度面板、警告面板等

**使用方式:**
```cpp
#include <PhantomDrive/UI/ThemeManager.h>

// 应用深色主题
qApp->setStyleSheet(ThemeManager::getStyleSheet("dark"));

// 应用赛车主题
qApp->setStyleSheet(ThemeManager::getStyleSheet("racing"));
```

---

## 二、文件变更清单

### 新增文件
| 文件 | 说明 |
|------|------|
| `include/PhantomDrive/UI/DrivingHistoryChartWidget.h` | 历史图表头文件 |
| `src/UI/DrivingHistoryChartWidget.cpp` | 历史图表实现 |
| `include/PhantomDrive/UI/ThemeManager.h` | 主题管理器 |
| `examples/core-test/CMakeLists.txt` | core-test 构建配置 |
| `examples/core-test/build.bat` | Windows 构建脚本 |
| `examples/core-test/test_ub_module.cpp` | U-B 模块测试 |
| `docs/U-B_development_doc.md` | 本文档 |

### 修改文件
| 文件 | 修改内容 |
|------|----------|
| `include/PhantomDrive/core/datamodels.h` | 重导出 ScoreReport |
| `include/PhantomDrive/core/saveloadmanager.h` | 完整 ScoreReport 支持 |
| `src/core/saveloadmanager.cpp` | 完整序列化实现 |
| `include/PhantomDrive/UI/DrivingReportWidget.h` | 添加真实数据接口 |
| `src/UI/DrivingReportWidget.cpp` | 完整重写，增强功能 |
| `include/PhantomDrive/UI/learninghud.h` | 扩展接口 |
| `src/UI/learninghud.cpp` | 完整重写，美化界面 |
| `CMakeLists.txt` | 添加新文件 |

---

## 三、对接接口

### A-B → U-B
```cpp
// A-B 生成报告后
ScoreReport report = calculator.generateReport();
SaveLoadManager::instance().saveReport(report);

// 添加到历史图表
historyWidget.addReport(report);
```

### E-B → U-B
```cpp
// 实时速度
hud.updateCurrentSpeed(vehicleData.speed);

// 限速
hud.updateSpeedLimit(vehicleData.currentSpeedLimit);

// 交通灯
hud.updateTrafficLight(lightState, remainingSeconds);

// 违规
hud.showPenaltyMessage(violation.description, violation.penaltyPoints);

// 道具
hud.updatePowerupState(powerup.id, powerup.name, true);
```

### U-B → U-A
```cpp
// 获取 HUD 指针用于添加到主窗口
LearningHUD* hud = new LearningHUD();
mainWindow->addOverlayWidget(hud);

// 获取图表
DrivingHistoryChartWidget* chart = new DrivingHistoryChartWidget();
mainWindow->setBottomWidget(chart);
```

---

## 四、编译测试

### 方式一: Qt Creator (推荐)

1. 打开 Qt Creator
2. 文件 → 打开文件或项目
3. 选择 `examples/core-test/CMakeLists.txt`
4. 配置 Kits (选择 Qt 6.8.3)
5. 点击运行

### 方式二: 命令行

```bash
cd examples/core-test
mkdir build && cd build
cmake .. -G "Qt Creator"
cmake --build . --config Release
.\bin\core-test.exe
```

### 方式三: Windows 批处理

```bash
cd examples/core-test
.\build.bat
```

---

## 五、DDL 进度

| 任务 | 截止日期 | 状态 |
|------|----------|------|
| JSON 存档最小版 | 5月16日 | ✅ 已完成 |
| 图表可视化 | 5月22日 | ✅ 已完成 |
| HUD 和 QSS 美化 | 5月27日 | ✅ 已完成 |

---

## 六、后续工作

### 6.1 集成测试
- 与 A-B 模块集成测试
- 与 E-B 模块集成测试
- 主游戏循环集成

### 6.2 功能增强
- [ ] 导出报告为 PDF/Markdown
- [ ] 历史数据统计（平均分、进步趋势）
- [ ] 多语言支持
- [ ] 声音提示（超速警告、违规提示）

### 6.3 性能优化
- [ ] 图表数据采样（减少点数）
- [ ] 延迟加载历史数据
- [ ] 内存缓存策略

---

## 七、代码示例

### 7.1 完整使用示例

```cpp
#include <QApplication>
#include <PhantomDrive/core/saveloadmanager.h>
#include <PhantomDrive/UI/learninghud.h>
#include <PhantomDrive/UI/DrivingHistoryChartWidget.h>
#include <PhantomDrive/UI/DrivingReportWidget.h>
#include <PhantomDrive/UI/ThemeManager.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 应用主题
    app.setStyleSheet(ThemeManager::getStyleSheet("dark"));

    // 创建 HUD
    LearningHUD* hud = new LearningHUD();
    hud->show();

    // 创建历史图表
    DrivingHistoryChartWidget* historyWidget = new DrivingHistoryChartWidget();

    // 加载历史
    QList<ScoreReport> history = SaveLoadManager::instance().loadHistory();
    historyWidget->updateHistory(history);

    // 模拟实时数据
    hud->updateCurrentSpeed(45.0);
    hud->updateSpeedLimit(60);
    hud->updateTrafficLight("green", 30);

    return app.exec();
}
```

### 7.2 报告保存示例

```cpp
// 驾驶结束后保存报告
void onDrivingFinished(const DrivingData& finalData)
{
    ScoreReport report;
    report.sessionId = QUuid::createUuid().toString();
    report.vehicleId = "player_1";
    report.generatedAt = QDateTime::currentDateTime();
    report.totalScore = calculateTotalScore(finalData);
    report.grade = ScoreReport::gradeFromScore(report.totalScore);

    // 保存
    SaveLoadManager::instance().saveReport(report);

    // 更新图表
    historyWidget->addReport(report);
}
```

---

## 八、联系与支持

如有疑问或问题，请联系 U-B 模块负责人。
