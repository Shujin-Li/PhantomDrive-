# A-B Demo Script (答辩/组会演示脚本)

## 1. 演示目标

本次演示目标是证明 A-B 已完成以下能力闭环：
- 评分算法计算；
- 报告结构生成（`ScoreReport` + JSON + Markdown）；
- Qt 信号对接（`scoreReady` / `coachReportReady`）；
- collector 对接（`evaluateFromCollector`）；
- API fallback（DeepSeek -> Zhipu/GLM -> Mock）；
- `QLearningFeedback` 输出。

## 2. 推荐演示顺序

1. 运行 `a-b-scoring-test`
2. 运行 `a-b-main-integration-test`
3. 运行 `a-b-collector-integration-test`
4. 演示 API 模式，说明 DeepSeek / Zhipu / Mock fallback

## 3. 每个 example 的演示要点

### 3.1 `a-b-scoring-test`

它验证什么：
- A-B 核心评分与报告链路是否可独立运行。

看到什么算成功：
- good/bad scenario 输出；
- `scoreReady` 触发；
- JSON 与 Markdown 非空；
- Mock 教练报告输出；
- `QLearningFeedback` 字段有值。

与 A-B 分工关系：
- 证明 A-B 核心算法和报告生成能力可用。

### 3.2 `a-b-main-integration-test`

它验证什么：
- `ScoreManager::evaluate`、信号链路、序列化、fallback 策略、可选真实 API 分支。

看到什么算成功：
- `[PASS]` 覆盖 `scoreReady`、`toJson`、`toMarkdown`、`durationMs` 数值化、`QLearningFeedback`；
- fallback 场景通过（`auto` 无 key、`mock` 强制、`deepseek` invalid key）；
- 若开启真实 API 分支并配置有效 key，可出现非 Mock 报告成功提示。

与 A-B 分工关系：
- 证明 A-B 对外主接口与 AI 报告策略在工程侧可联调。

### 3.3 `a-b-collector-integration-test`

它验证什么：
- `ScoreManager::evaluateFromCollector` 能消费 collector 数据结构。

看到什么算成功：
- `scoreReady` 触发；
- `report.metrics` 与 collector 数据数量一致；
- collector violations 进入 `ScoreReport`；
- `QLearningFeedback` 正常生成。

与 A-B 分工关系：
- 证明 A-B 与 E-B 数据采集链路可对接。

## 4. API 演示说明

- 答辩稳定演示建议优先 `mock` 或 `auto`。
- 若有 DeepSeek key，可演示真实中文教练报告（非 Mock）。
- 若 Zhipu 返回 429（访问量过大），不视为 A-B 失败；这说明 fallback 机制生效。
- 不在文档、代码、日志中暴露 API key。

## 5. 1 分钟口头讲解稿（示例）

“我们 A-B 组当前已经完成驾驶评分与 AI 教练报告的核心闭环。输入层面支持 `DrivingData` 和 `ViolationEvent`，经过 `ScoreManager` 与 `DrivingScoreCalculator` 产出 `ScoreReport`，并可输出 JSON、Markdown，同时通过 `scoreReady` 和 `coachReportReady` 提供 UI 对接。强化学习方面我们提供了 episode-level 的 `QLearningFeedback`，用于训练策略评估。AI 报告部分采用 DeepSeek 主路、Zhipu 兜底、Mock 保底，确保无 key、网络失败或限流时系统仍可稳定演示。当前根工程完整 build 的阻塞在 physics/MiniCore 缺失依赖，不属于 A-B 责任范围。A-B 已通过三个独立 examples 验收，后续等待引擎组、UI 组和 A-A 组按接口清单继续联调。”
