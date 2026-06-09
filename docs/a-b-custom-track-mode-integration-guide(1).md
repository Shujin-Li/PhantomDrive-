# A-B Custom Track Mode 对接说明

> 分支：`feature/a-b-custom-track-mode`  
> 模块负责人：A-B  
> 文档用途：给 E-B、A-A、U-A/U-B 和其他组员说明当前自定义赛道模式已经完成的内容、可对接接口、边界和后续优先级。

---

## 1. 当前模块定位

本分支将项目从原来的 Arcade Mode / Learning Mode 双模式扩展为三模式：

1. **Arcade Mode**：固定竞速赛道、AI 对手、道具、圈速与排名。
2. **Learning Mode**：交通规则学习、限速/红绿灯/行人、违规检测、评分与 AI 教练。
3. **Custom Track Mode**：游戏内嵌 tile-based 自定义赛道编辑器，支持编辑、保存、加载、导出，并直接在自定义赛道上完成一次路线挑战。

Custom Track Mode 是第三个独立模式，不是 Arcade 或 Learning 的附属按钮。

---

## 2. 本轮 A-B 已完成内容

### 2.1 模式与入口

已新增 Custom Track Mode 相关代码，并接入主程序：

- `include/PhantomDrive/gamemode/CustomTrackMode.h`
- `src/gamemode/CustomTrackMode.cpp`
- `include/PhantomDrive/UI/CustomTrackEditorWidget.h`
- `src/UI/CustomTrackEditorWidget.cpp`
- `include/PhantomDrive/track/TrackValidator.h`
- `src/track/TrackValidator.cpp`

主菜单中已加入 **Custom Track Mode** 入口，可从主程序进入游戏内嵌编辑器。

`CMakeLists.txt` 已加入上述新增 `.h/.cpp`，这些文件会编入主 `PhantomDrive` shared library，并由 `PhantomDriveApp` 使用。

### 2.2 游戏内嵌赛道编辑器

当前编辑器为游戏内嵌 QWidget，不是外部独立编辑器窗口。第一版固定地图尺寸为：

```text
24 × 18 tiles
```

支持 8 个画笔：

| 工具 | 当前用途 |
|---|---|
| Road | 绘制可驾驶道路 |
| Grass | 绘制草地/背景 |
| Wall | 绘制墙体或不可通行区域 |
| Start | 设置唯一玩家出生点 |
| Finish | 设置唯一终点/完成点 |
| CP | 按点击顺序设置 CP1、CP2、CP3... |
| Item | 放置 Item Box 占位 |
| Erase | 擦除 Road/Wall/Start/Finish/CP/Item 等元素 |

编辑器已实现两行按钮布局、当前画笔高亮、Start/Finish/CP/Item 标记显示和基础 UI 美化。

### 2.3 保存、加载与导出

当前复用项目已有 `TrackIO`，没有重新设计文件格式。

支持：

- `Save Track`：默认保存 `.pdtrack`
- `Load Track`：加载 `.pdtrack` 或 `.json`
- `Export JSON`：导出 `.json` 方便调试和人工检查

`TrackIO` 已支持 `pdtrack` 和 `json` 两种扩展名。自定义赛道保存内容包括：

- `tiles`
- `checkpoints`
- `startPosition`
- `startPositions`
- `itemBoxes`
- `maxLaps`
- 赛道基础元信息，例如 name、id、author、description、difficulty 等

### 2.4 TrackData 扩展

本轮对 `TrackData` 做了轻量扩展，主要是为了支持自定义赛道编辑和保存：

- 保留原有 `tiles`、`checkpoints`、`startPosition`、`startPositions` 等结构。
- 新增 `itemBoxPositions` 相关能力，用于保存自定义赛道中 Item Box 的位置。
- 新增/使用统一的 tile/world 坐标转换函数：
  - `TrackData::tileToWorldCenter(row, col, tileSize)`
  - `TrackData::worldToTile(worldPos, tileSize)`

坐标约定：

```text
row = y 方向，第几行
col = x 方向，第几列
QPoint(col, row)：x 表示列，y 表示行
worldX = col * tileSize + tileSize / 2
worldY = row * tileSize + tileSize / 2
```

### 2.5 合法性检查

新增 `TrackValidator`，用于 Custom Track Mode 点击 `Play This Track` 前的合法性检查。

当前最低合法性规则：

- 必须有 Start。
- 必须有 Finish。
- 至少需要 2 个 checkpoint。
- 必须有足够可驾驶道路。
- Start / Finish / CP 应放在可驾驶区域上或由编辑器自动调整为可驾驶道路。
- 非法赛道不能进入比赛，只提示错误。

第一版没有实现复杂道路连通性算法，后续可作为 P1 优化。

### 2.6 自定义赛道比赛逻辑

Custom Track 当前不是固定 3 圈逻辑，而是一次路线挑战：

```text
Start → CP1 → CP2 → ... → Finish
```

规则：

- 玩家从编辑器设置的 Start tile 中心出生。
- CP 按用户点击顺序编号。
- 玩家必须按 `CP1 → CP2 → ...` 顺序通过。
- 未经过当前 required CP 时，直接到 Finish 不会完成。
- 通过所有 CP 后到达 Finish，判定 Custom Track Finished，并进入现有结算/报告流程。

Custom Track 的 HUD 使用路线进度文案，例如：

```text
Route: x/y CPs
Next: CPn / FINISH
```

Arcade Mode 仍保留原来的 lap/ranking/AI 竞速逻辑。

### 2.7 模式隔离

已处理 Custom Track 对其他模式的污染问题：

- Custom Track 使用独立赛道状态/运行态 snapshot。
- 进入 Arcade / Learning 时会恢复默认赛道和对应模式对象。
- Custom Track 的 Start/Finish/CP/HUD 改动只服务于 Custom Track 分支。

验收目标：在 Custom Track 中编辑/运行赛道后，再进入 Arcade 或 Learning，二者不应显示自定义赛道。

---

## 3. 对 E-B 的对接说明

E-B 主要负责赛道美化、道具、地图对象和运行时环境对象。当前 A-B 侧边界如下：

### 3.1 当前 A-B 已提供

- Custom Track Editor 中可以放置 Item Box。
- `TrackData` 中已有 `itemBoxPositions`。
- `.pdtrack/.json` 中会保存 `itemBoxes`。
- GameViewWidget 中可以显示 Item Box 占位 marker。

### 3.2 当前 A-B 未实现

A-B 没有实现真实道具效果，也没有修改：

- `PowerupManager`
- `PowerupBox`
- 道具拾取、生效、冷却、碰撞触发等逻辑

### 3.3 E-B 后续可接入方向

E-B 可以基于 `TrackData::getItemBoxPositions()`：

1. 在 Custom Track Play 阶段生成真实 `PowerupBox`。
2. 复用现有 Powerup/PowerupManager 逻辑实现拾取与效果。
3. 保证 Item Box 在 Arcade 固定赛道和 Custom Track Mode 中表现一致。
4. 美化 Road / Grass / Wall / Start / Finish / CP / Item 的 tile 视觉，但不要改变其数据语义。

建议 E-B 接入时只读取 Custom Track 的 itemBoxPositions，不要重新定义一套 item 坐标格式。

---

## 4. 对 A-A 的对接说明

当前 Custom Track 第一版没有接入 AI 对手。A-A 后续接入时应复用同一套自定义赛道数据，而不是写死默认赛道坐标。

### 4.1 AI 应读取的数据

建议 A-A 从当前 custom track 中读取：

- Start：AI 初始位置可基于玩家 Start 或后续扩展 startPositions。
- Checkpoints：AI 路线顺序应使用 `getCheckpointsInOrder()`。
- Finish：AI 完成路线也应以 Finish tile 为终点。
- Road/Wall：AI 避障与路径选择应基于当前 TrackData tiles。

### 4.2 AI 完成条件

AI 在 Custom Track 中应与玩家一致：

```text
Start → CP1 → CP2 → ... → Finish
```

不建议 A-A 在 Custom Track 中沿用 Arcade 的固定 3 圈 lap 逻辑，也不建议写死默认赛道的 waypoint。

### 4.3 当前保留边界

A-B 没有修改：

- `AIOpponentManager` 核心算法
- AI 排名算法
- AI 避障算法
- AI 难度逻辑

A-A 后续可在 Custom Track Mode 的 Play 阶段接入 AI，但需要保持 Arcade / Learning 原有逻辑不受影响。

---

## 5. 对 U-A / U-B 的对接说明

当前 Custom Track UI 已完成基础可用与一轮美化，后续 UI 组可以继续做整体统一。

### 5.1 已有 UI

- 主菜单 `Custom Track Mode` 入口。
- 游戏内嵌编辑器。
- 工具栏：Road / Grass / Wall / Start / Finish / CP / Item / Erase。
- 操作栏：Play This Track / Save Track / Load Track / Export JSON / Back。
- 底部状态提示。
- 自定义赛道游戏界面的路线进度 HUD。
- Start / Finish / CP / Item overlay。

### 5.2 UI 组后续优化建议

- 统一三模式的整体视觉风格。
- 保持 Custom Track Editor 的功能布局不变，主要优化样式。
- 不要破坏按钮信号、Play 流程、TrackValidator 拦截逻辑。
- 若继续美化 GameViewWidget，只改显示层，不改坐标、判定、物理、AI、评分逻辑。

---

## 6. 对评分/AI 教练模块的影响

当前 Custom Track 完成后复用现有驾驶结束/报告流程。A-B 本轮没有修改：

- `ScoreManager` 评分算法
- `DrivingScoreCalculator`
- `TrafficRuleEnforcer`
- `AIAPIClient`

也就是说，Custom Track Mode 当前主要提供“可完成一次自定义路线驾驶”的入口；评分和 AI 教练报告仍走既有模块能力。

---

## 7. 测试与验收建议

### 7.1 最小合法赛道验收

进入 `Custom Track Mode` 后，画一个最小路线：

```text
Start → CP1 → CP2 → Finish
```

建议：

1. 用 Road 画一条连续道路。
2. 放置一个 Start。
3. 放置一个 Finish。
4. 按行驶顺序放置 CP1、CP2。
5. 点击 `Play This Track`。

预期：

- 玩家出生在 Start。
- 游戏界面中 Start / CP / Finish 与编辑器位置一致。
- 直接冲 Finish 不完成。
- 按 CP1 → CP2 → Finish 完成。
- 完成后进入结算/报告流程。

### 7.2 保存/加载验收

1. Save Track 保存 `.pdtrack`。
2. Load Track 加载刚才的 `.pdtrack`。
3. Export JSON 导出 `.json`。
4. 加载 `.json`，确认 tiles / Start / Finish / CP / Item 位置一致。

### 7.3 模式隔离验收

1. 进入 Custom Track Mode，编辑并 Play 一条自定义赛道。
2. 返回主菜单，进入 Learning Mode。
3. 确认 Learning Mode 仍是原学习模式地图和交通对象。
4. 返回主菜单，进入 Arcade Mode。
5. 确认 Arcade Mode 仍是原默认竞速赛道、AI、HUD 和道具逻辑。

---

## 8. 当前限制

当前第一版 Custom Track Mode 仍有以下限制：

1. 地图尺寸固定为 `24 × 18`。
2. 不包含复杂道路连通性检查。
3. Custom Track 第一版未接入 AI 对手。
4. Item Box 当前主要是编辑器放置、保存、加载和显示占位，真实道具效果待 E-B 对接。
5. 自定义赛道暂不支持多关卡管理、解锁系统或完整关卡选择页。
6. UI 已有基础美化，但整体风格仍可由 UI 组继续统一。

---

## 9. 后续对接优先级

| 优先级 | 任务 | 负责/对接 |
|---|---|---|
| P0 | E-B 基于 `itemBoxPositions` 接真实道具生成与拾取 | E-B + A-B |
| P0 | A-A 让 AI 读取 Custom Track 的 CP 顺序和 Finish 完成条件 | A-A + A-B |
| P0 | U 组统一 Custom / Arcade / Learning 的 UI 风格 | U-A/U-B |
| P1 | 增加道路连通性检查 | A-B 或 E-B 协助 |
| P1 | 增加更多自定义赛道示例文件 | A-B / E-B |
| P2 | 加关卡管理、难度、解锁或赛道库 | 全组后续扩展 |

---

## 10. 自检结论

本说明已按当前分支代码和本轮对话中的实际实现状态编写，重点核对如下：

- `CMakeLists.txt` 中已包含 `CustomTrackMode`、`CustomTrackEditorWidget`、`TrackValidator`。
- `README.md` 已将项目描述为 tri-mode，并列出 Custom Track Mode。
- `GameMode.h` 中已有 `ModeType::CustomTrack`。
- `TrackData` 已包含 `itemBoxPositions`、tile/world 坐标转换函数和 JSON 序列化。
- `TrackIO` 已支持 `.pdtrack` / `.json`，并保存/加载 `itemBoxes`。
- `ArcadeHUD` 已包含 `updateRouteProgress` 和 `setCustomTrackVisualMode`，用于 Custom Track 的路线进度显示。
- A-B 本轮没有声明已完成真实道具效果、AI 对手或复杂道路连通性，文档中也未夸大这些部分。

