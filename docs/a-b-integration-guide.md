# A-B Integration Guide (v1.0-a-b)

## 1. A-B 当前完成状态摘要

A-B（驾驶评分算法与 AI 教练报告）在 `v1.0-a-b` 分支已完成从 `DrivingData` / `ViolationEvent` 输入到 `ScoreReport` 输出的闭环，包含：
- 本地确定性评分计算（`DrivingScoreCalculator`）。
- `ScoreReport` 结构化输出（对象、JSON、Markdown）。
- `ScoreManager` 调用与 Qt 信号（`scoreReady` / `coachReportReady` / `scoringFailed`）。
- `AIAPIClient` 中文教练报告生成与多级 fallback。
- `QLearningFeedback` episode-level 数值反馈输出。

当前根工程完整 build 仍被 physics/MiniCore 缺失依赖或缺失头文件阻塞；该阻塞不属于 A-B 负责范围。

A-B 当前验收依据为三个独立 examples：
- `examples/a-b-scoring-test`
- `examples/a-b-main-integration-test`
- `examples/a-b-collector-integration-test`

## 2. A-B 模块职责边界

### 2.1 A-B 负责

- 评分算法与规则聚合：
  - `DrivingScoreCalculator`
  - `TrafficRuleEnforcer`
- 评分报告与序列化：
  - `ScoreReport`
  - `ScoreReport::toJson()`
  - `ScoreReport::toMarkdown()`
- 评分协调与对外信号：
  - `ScoreManager`
  - `scoreReady(const ScoreReport&)`
  - `coachReportReady(const QString&)`
  - `scoringFailed(const QString&)`
- AI 教练文本报告：
  - `AIAPIClient`
  - DeepSeek -> Zhipu/GLM -> Mock fallback
- 强化学习数值反馈：
  - `QLearningFeedback`（episode-level）
- 对接说明：
  - E-B 数据输入字段
  - U-A/U-B UI 展示字段
  - JSON / 图表 / 历史记录消费字段

### 2.2 A-B 不负责

- 完整 UI 页面实现与交互动效。
- 完整引擎主回路与完整游戏渲染。
- 完整 AI 对手系统。
- physics/MiniCore 缺失依赖修复。

## 3. 对 E-B / 引擎组的接口需求

### 3.1 DrivingData 建议稳定字段

- `timestamp`
- `position`
- `speed`
- `acceleration`
- `steeringAngle`
- `currentSpeedLimit`

### 3.2 ViolationEvent 建议稳定字段

- `timestamp`
- `type`
- `description`
- `position`
- `speedAtViolation`
- `speedLimit`
- `penaltyPoints`

### 3.3 交通对象补充规则

若 E-B 提供 `TrafficObjectManager` 与交通对象状态，A-B 可通过 `TrafficRuleEnforcer` 追加规则违规（如限速区、红灯、行人区）。

### 3.4 当前可消费入口

A-B 当前已支持两条输入链路：
- `ScoreManager::evaluate(data, violations)`
- `ScoreManager::evaluateFromCollector(collector)`

## 4. 对 U-A / U-B / UI 组的接口需求

UI 层建议只连接 `ScoreManager` 的三个信号：
- `scoreReady(const ScoreReport&)`
- `coachReportReady(const QString&)`
- `scoringFailed(const QString&)`

建议优先展示字段：
- `totalScore`
- `grade`
- `breakdown` 四个维度分（安全/规则/平顺/效率）
- 违规数量与关键违规项
- `coachAdvices`
- 教练报告 markdown 文本

`ScoreReport::toJson()` 可直接用于：
- 历史记录存档
- JSON 落盘/回放
- 图表与统计面板数据源

## 5. 对 A-A / Q-Learning 组的接口需求

`qLearningFeedback` 当前为 episode-level feedback，已提供：
- `reward`
- `normalizedScore`
- `safetyRisk`
- `ruleCompliance`
- `collisionPenalty`
- `speedPenalty`
- `smoothnessPenalty`
- `terminalPenalty`
- `recommendedActionHint`

若 A-A 要做逐帧/逐 transition 的 Q-Learning 训练回路，需要另行确认并冻结：
- `state`
- `action`
- `reward`
- `next_state`
- `done`

## 6. API 与 fallback 策略

统一策略：
- 主 API：DeepSeek V4 Flash
- 免费兜底：Zhipu / GLM-4.7-Flash
- 最后保底：本地 Mock

执行优先级：
- `DeepSeek -> Zhipu/GLM -> Mock`

关键约束：
- API 仅用于中文教练报告、赛后分析、策略建议、调试解释等文本生成。
- 评分、碰撞/违规统计、Q-Learning 数值反馈保持本地确定性运行，不依赖外网。
- 不硬编码 API key。
- 无 key、网络失败、HTTP 错误、限流（如 429）等场景必须自动 fallback 到 Mock，确保演示稳定。

## 7. 当前验收结果

### 7.1 `a-b-scoring-test` 已通过

验证内容：
- `DrivingScoreCalculator` 评分算法。
- `ScoreReport` 生成。
- `scoreReady` 信号触发。
- JSON / Markdown 序列化。
- Mock AI 教练报告。
- `QLearningFeedback` 输出。

### 7.2 `a-b-main-integration-test` 已通过

验证内容：
- `ScoreManager::evaluate`。
- `scoreReady` 信号。
- `coachReportReady` 信号。
- `ScoreReport::toJson()` / `toMarkdown()`。
- `metrics.durationMs` 为数值 JSON。
- `QLearningFeedback` 字段完整。
- `auto` 无 key fallback 到 Mock。
- `mock` 模式强制 Mock。
- `deepseek` invalid key fallback 到 Mock。
- 配置真实 DeepSeek API 后成功返回非 Mock 中文 AI 教练报告。

### 7.3 `a-b-collector-integration-test` 已通过

验证内容：
- `ScoreManager::evaluateFromCollector`。
- A-B 可消费 `DrivingDataCollector` / `DrivingData` / `ViolationEvent`。
- `report.metrics` 与 collector 数据数量一致。
- collector violations 可进入 `ScoreReport`。
- `QLearningFeedback` 正常生成。

结论：
- 三个 examples 全部通过后，可证明 A-B 当前独立功能闭环成立。

## 8. 真实 API 验收说明

- DeepSeek 真实 API 已成功生成非 Mock 中文教练报告。
- Zhipu / GLM 请求已到达服务器，但曾出现 HTTP 429（访问量过大）并 fallback 到 Mock。
- 该结果证明 fallback 机制在真实故障场景下有效。
- 答辩演示建议默认使用 `mock` 或 `auto`，降低网络波动风险。

## 9. 等待其他组的事项

- E-B：提供稳定真实驾驶数据流、违规事件流、交通对象状态。
- U-A/U-B：完成报告展示入口、分数卡片、维度条、违规列表、教练报告文本区、历史记录/JSON 消费界面。
- A-A：确认 `qLearningFeedback` 是否足够；若需训练回路，补充 transition 定义。
- physics/MiniCore：修复根工程完整 build 阻塞。

## 10. 根工程状态说明

- A-B scoring 接入项已保留在根 `CMakeLists.txt`（头文件、源文件、include path、`Qt6::Network`）。
- 当前 v1.0 根工程完整 build 被 physics/MiniCore 缺失依赖阻塞。
- 该阻塞不属于 A-B 范围。
- A-B 当前通过独立 examples 完成验收；待 physics/engine 修复后可继续主库级联调。
