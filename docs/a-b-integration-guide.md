# A-B Integration Guide (v1.0)

## 0. v1.0 根工程构建状态说明

当前 v1.0 中，A-B scoring 已加入根 `CMakeLists.txt`（头文件、源文件、include path、`Qt6::Network` 依赖均已接入），可作为 scoring 接入主工程的代码层证明。

但完整根工程 `build` 目前被 physics/MiniCore 的缺失依赖或缺失头文件阻塞（例如 `mcobjectfactory.hh` 相关依赖），该问题不属于 A-B 范围。

因此当前 A-B 验收采用独立 `examples` 测试方案，重点验证：
- scoring 算法计算链路可用；
- `ScoreReport` 的 JSON/Markdown 输出可用；
- `ScoreManager::scoreReady` 信号链路可用；
- `AIAPIClient` 的 DeepSeek -> Zhipu -> Mock fallback 可用；
- `QLearningFeedback` 与 `DrivingData`/`ViolationEvent` 结构联动可用。

待引擎/物理模块修复根工程构建问题后，A-B 可直接随主库参与完整构建与联调。

## 1. A-B 模块职责

A-B（驾驶评分算法与 AI 教练报告）当前负责：
- 对 `DrivingData + ViolationEvent` 计算 `ScoreReport`。
- 输出分数维度、违规统计、教练建议、`QLearningFeedback`。
- 通过 `ScoreManager` 提供统一调用入口与 Qt 信号：
  - `scoreReady(const ScoreReport&)`
  - `coachReportReady(const QString&)`
  - `scoringFailed(const QString&)`
- 通过 `AIAPIClient` 生成中文教练报告，并在 API 不可用时自动 fallback 到本地 Mock。

不负责：完整 UI、完整引擎回路、完整 AI 对手、完整游戏渲染。

## 2. 当前文件结构

- 公共头文件：
  - `include/PhantomDrive/scoring/ScoreReport.h`
  - `include/PhantomDrive/scoring/DrivingScoreCalculator.h`
  - `include/PhantomDrive/scoring/TrafficRuleEnforcer.h`
  - `include/PhantomDrive/scoring/ScoreManager.h`
  - `include/PhantomDrive/scoring/AIAPIClient.h`
- 实现文件：
  - `src/scoring/ScoreReport.cpp`
  - `src/scoring/DrivingScoreCalculator.cpp`
  - `src/scoring/TrafficRuleEnforcer.cpp`
  - `src/scoring/ScoreManager.cpp`
  - `src/scoring/AIAPIClient.cpp`
- 主库导出：
  - `include/PhantomDrive/PhantomDrive.h` 已导出必要 scoring 头。

## 3. 主调用链

1. E-B 或测试代码提供 `QList<DrivingData>` 与 `QList<ViolationEvent>`。
2. 调用 `ScoreManager::evaluate(data, violations)`。
3. `ScoreManager`：
   - 可选调用 `TrafficRuleEnforcer` 基于 `TrafficObjectManager` 补充规则违规。
   - 调用 `DrivingScoreCalculator::evaluate(...)` 产出 `ScoreReport`。
   - 发出 `scoreReady(report)`。
4. UI 或上层可调用 `ScoreManager::generateCoachReport(report)`：
   - 内部调用 `AIAPIClient::generateCoachReport(report)`。
   - 发出 `coachReportReady(markdown)`。

采集器链路：
- `ScoreManager::evaluateFromCollector(const IDrivingDataCollector*)`
- 内部直接消费 `collector->getCollectedData()` + `collector->getViolations()`。

## 4. ScoreReport 字段说明

- 基础字段：`sessionId`, `vehicleId`, `generatedAt`, `totalScore`, `grade`, `summary`
- `metrics`：时长、平均速度、最大速度、违规计数、平顺性计数等
- `breakdown`：安全/规则/平顺/效率维度分与罚分拆解
- `violations`：违规事件列表
- `coachAdvices`：规则化建议列表（分类、严重级别、证据）
- `qLearningFeedback`：episode 级强化学习反馈

## 5. UI 显示字段（建议）

当前 UI 文件（`include/src/UI`）可直接展示下列字段：
- 总分与等级：`totalScore`, `grade`
- 维度分：`breakdown.safetyScore`, `breakdown.ruleComplianceScore`, `breakdown.smoothnessScore`, `breakdown.efficiencyScore`
- 违规摘要：`metrics.collisionCount`, `metrics.speedViolationCount`, `metrics.redLightViolationCount`, `metrics.pedestrianViolationCount`
- 教练建议：`coachAdvices[*].message`
- 强化学习反馈：`qLearningFeedback.reward`, `qLearningFeedback.safetyRisk`, `qLearningFeedback.ruleCompliance`, `qLearningFeedback.recommendedActionHint`

## 6. JSON / 历史记录字段

`ScoreReport::toJson()` 已完整输出：
- 基础字段
- `metrics`
- `breakdown`
- `violations`
- `coachAdvices`
- `qLearningFeedback`

兼容性说明：
- `metrics.durationMs` 当前为 JSON 数值类型（便于消费端直接计算）。

## 7. QLearningFeedback 字段与 JSON 示例

字段：
- `reward`
- `normalizedScore`
- `safetyRisk`
- `ruleCompliance`
- `collisionPenalty`
- `speedPenalty`
- `smoothnessPenalty`
- `terminalPenalty`
- `recommendedActionHint`

示例：

```json
{
  "sessionId": "session_1715600000000",
  "vehicleId": "AB_MAIN_INTEGRATION",
  "totalScore": 82.6,
  "grade": "B",
  "qLearningFeedback": {
    "reward": 0.4123,
    "normalizedScore": 0.8260,
    "safetyRisk": 0.2400,
    "ruleCompliance": 0.7800,
    "collisionPenalty": 0.1500,
    "speedPenalty": 0.1200,
    "smoothnessPenalty": 0.0900,
    "terminalPenalty": 0.2000,
    "recommendedActionHint": "observe_speed_limit_and_signals"
  }
}
```

说明：当前反馈是 **episode-level feedback**（单次回合汇总），不是逐帧 Q-learning 训练闭环。

如果 A-A 后续要接入训练回路，请补齐并确认：
- `state`
- `action`
- `reward`
- `next_state`
- `done`

## 8. E-B 需要提供的数据

`DrivingData` 建议至少稳定提供：
- `timestamp`
- `position`
- `speed`
- `acceleration`
- `steeringAngle`
- `currentSpeedLimit`

`ViolationEvent` 建议提供：
- `timestamp`
- `type`
- `description`
- `position`
- `speedAtViolation`
- `speedLimit`
- `penaltyPoints`

可选：
- 由 `TrafficObjectManager` + `TrafficRuleEnforcer` 自动补充红灯/行人/限速区规则违规。

## 9. U-A / U-B 对接信号方式

推荐在 UI 层连接 `ScoreManager`：
- `scoreReady(const ScoreReport&)`：刷新分数卡片、维度条、违规列表。
- `coachReportReady(const QString&)`：显示中文教练报告（Markdown）。
- `scoringFailed(const QString&)`：显示错误提示并允许重试。

基本流程：
1. 一次驾驶结束后调用 `evaluate(...)` 或 `evaluateFromCollector(...)`。
2. 在 `scoreReady` 收到结果后渲染评分界面。
3. 调用 `generateCoachReport(report)`，在 `coachReportReady` 收到文本后渲染报告区域。

## 10. AIAPIClient fallback 配置

环境变量：
- `PHANTOMDRIVE_AI_MODE=auto/mock/deepseek/zhipu`
- `PHANTOMDRIVE_AI_TIMEOUT_MS=5000`
- `DEEPSEEK_API_KEY`
- `DEEPSEEK_BASE_URL=https://api.deepseek.com`
- `DEEPSEEK_MODEL=deepseek-v4-flash`
- `ZHIPU_API_KEY`
- `ZHIPU_BASE_URL=https://api.z.ai/api/paas/v4`
- `ZHIPU_MODEL=glm-4.7-flash`

策略：
- `mode=mock`：直接本地 Mock。
- `mode=auto`：DeepSeek -> Zhipu -> Mock。
- `mode=deepseek`：DeepSeek 失败后 fallback Mock。
- `mode=zhipu`：Zhipu 失败后 fallback Mock。

保证：
- 没有 key、网络失败、API 报错都不会让程序崩溃。
- fallback 原因会写入 Mock 报告文本，便于调试。

## 11. 当前限制与后续清单

当前限制：
- AI 报告请求采用最小阻塞实现（短超时），用于演示稳定优先。
- 训练反馈为 episode 级汇总，不是逐帧 RL 环。
- UI 仍需 U-A/U-B 按产品需求完成布局与交互细化。

后续建议：
- A-A 明确 RL 状态空间与动作空间定义。
- E-B 提供更稳定的事件语义（如碰撞分类、红灯判定上下文）。
- U-A/U-B 增加报告折叠、违规回放定位、历史对比视图。

## 12. 当前验收结果

### 12.1 a-b-scoring-test 已通过

已验证：
- `DrivingScoreCalculator` 评分算法可用；
- `ScoreReport` 生成可用；
- `scoreReady` 信号可触发；
- JSON / Markdown 序列化可用；
- Mock AI 教练报告可生成；
- `QLearningFeedback` 输出可用。

### 12.2 a-b-main-integration-test 已通过

已验证：
- `ScoreManager::evaluate` 可用；
- `scoreReady` 信号可触发；
- `coachReportReady` 信号可触发；
- `ScoreReport::toJson` / `toMarkdown` 可用；
- `metrics.durationMs` 为数值 JSON；
- `QLearningFeedback` 字段完整；
- `auto` 无 key 时可 fallback 到 Mock；
- `mock` 模式可强制 Mock；
- `deepseek` invalid key 时可 fallback 到 Mock；
- 配置真实 DeepSeek API 后，可返回非 Mock 的中文 AI 教练报告。

### 12.3 a-b-collector-integration-test 已通过

已验证：
- `ScoreManager::evaluateFromCollector` 可用；
- A-B 可消费 `DrivingDataCollector` / `DrivingData` / `ViolationEvent`；
- `report.metrics` 与 collector 数据数量一致；
- collector violations 可进入 `ScoreReport`；
- `QLearningFeedback` 可正常生成。

### 12.4 真实 API 验收说明

- DeepSeek 真实 API 已成功生成非 Mock 中文教练报告；
- Zhipu / GLM 请求已到达服务器，但曾因 HTTP 429（模型访问量过大）而 fallback 到 Mock；
- 该现象证明 API 失败时 fallback 机制有效；
- 答辩演示建议默认使用 `mock` 或 `auto`，降低网络波动对演示稳定性的影响。

### 12.5 根工程状态说明

- A-B scoring 已保留在根 `CMakeLists.txt` 的接入项中；
- 当前 v1.0 根工程完整 build 仍被 physics/MiniCore 缺失依赖阻塞；
- 该问题不属于 A-B 范围；
- A-B 当前通过独立 examples 完成验收，待 physics/engine 修复后可继续主库级联调。
