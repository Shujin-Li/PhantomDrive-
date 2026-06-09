# PhantomDrive 6.2 UI Main Menu & Feedback Fix

日期：2026-06-04

## 本轮目标

1. 主界面改为与游戏界面一致的深色霓虹风格。
2. 修正 U-B 反馈的问题：浮动提示不能只显示 GO，碰撞、超速、检查点、圈数、道具等提示都要能显示。
3. 保持现有 A-B / A-A / U-B 集成接口不被破坏。

## 已完成修改

### 1. 主界面霓虹风格

修改文件：

- `include/PhantomDrive/UI/ThemeManager.h`
- `include/PhantomDrive/UI/mainwindow.h`
- `src/UI/mainwindow.cpp`

完成内容：

- 新增 `ThemeManager::mainMenuNeonQss()`，为主菜单单独提供霓虹 QSS。
- 主界面背景改为深色蓝黑渐变，按钮改为蓝色/红色霓虹描边和 hover 高亮。
- 主标题改为 `PHANTOMDRIVE` 大号霓虹 Logo 风格。
- AI 难度下拉框、Arcade / Learning / Custom Track / History / Exit 等按钮统一为游戏 HUD 风格。
- 调整主菜单布局间距和按钮尺寸，使菜单更接近游戏内界面视觉。

### 2. 浮动提示显示修正

修改文件：

- `include/PhantomDrive/UI/InteractiveFeedback.h`
- `src/UI/InteractiveFeedback.cpp`
- `src/UI/mainwindow.cpp`

完成内容：

- 新增 `InteractiveFeedback::ensureOverlayVisible()`，每次加入提示前都会确认浮层已显示、已定位、已置顶。
- 浮层现在会绑定 `GameViewWidget` 区域，避免提示跑到不可见区域。
- 修复提示容器尺寸更新问题，切换窗口大小后提示仍会在画面中央上方显示。
- 修复透明动画：从直接修改控件 `windowOpacity` 改为 `QGraphicsOpacityEffect`，让普通子控件标签的淡入淡出可靠生效。
- 支持多条提示排队和堆叠显示，不再只出现第一条 GO。
- 将 MainWindow 内的碰撞、道具、HUD 中央通知、学习模式扣分提示统一走 `showInteractiveFeedback()`，保证调用路径一致。

现在应能显示的提示包括：

- `GO!`
- `Lap X Complete!`
- `Checkpoint`
- `Wall Hit!`
- `Speeding!`
- 道具收集提示
- 学习模式违规扣分提示

## 验证结果

已完成本地编译：

- CMake + MSVC + Ninja 编译通过
- 构建目录：`build-codex-ui-feedback`

已运行集成 demo：

- `a_b_ddl_event_integration_demo.exe`：退出码 `0`
- `a_a_race_integration_test.exe`：退出码 `0`

## 备注

本轮没有改动车辆物理、速度换算、AI 运动逻辑和评分算法，只处理主界面视觉和浮动提示显示可靠性。
