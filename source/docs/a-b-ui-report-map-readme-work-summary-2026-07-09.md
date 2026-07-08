# A-B 窗口工作总结

日期：2026-07-09

## 1. Arcade 赛前设置界面

- 将原先横向铺满的设置控件重构为居中的赛前控制台。
- 使用赛道、玩家阵容和 AI 难度卡片建立清晰的信息层级。
- 优化背景、边框、字号、间距、按钮状态及桌面端显示比例。
- 修复玩家选项和开始按钮文字被裁切的问题。
- 新增 SVG 下拉箭头，修复 Qt 默认箭头显示异常。
- 将最终效果截图加入 README 的 Race Setup 章节。

相关文件：

- `src/UI/mainwindow.cpp`
- `assets/ui/chevron_down.svg`

## 2. AI 教练报告

- 检查 AI 报告生成、异步回传、缓存和本地兜底流程。
- 修正 AI 提示词中损坏的中文章节要求。
- 修正本地兜底报告乱码，并保留可执行的训练建议。
- 恢复 `QTextBrowser::setMarkdown()` 原生 Markdown 渲染。
- 移除会破坏标题与正文顺序的自定义 HTML 转换。
- 删除冗余的“总览”章节。
- 新生成的报告不再要求“总览”；已有缓存报告在显示时也会过滤该章节。
- 当前报告重点保留：
  - 驾驶亮点
  - 主要问题
  - 改进建议
  - 规则遵守分析
  - 速度控制分析
  - 下次训练目标

相关文件：

- `src/scoring/AIAPIClient.cpp`
- `src/UI/DrivingReportWidget.cpp`

## 3. 模式地图状态隔离

问题原因是 Arcade、Learning 和 Two-Player 共用了 `m_selectedTrackId` 与
`m_defaultRaceTrack`。在 Arcade 中切换赛道后，其他模式会恢复同一个赛道对象。

完成的修复：

- 新增 Arcade 独立赛道状态 `m_arcadeTrackId`。
- 新增 Two-Player 独立赛道状态 `m_twoPlayerTrackId`。
- Learning 每次启动时强制重新创建默认 `neon_loop` 地图。
- Arcade 和 Two-Player 的选图记录互不覆盖。
- Coin Challenge 的选图不会改变 Learning 默认地图。

相关文件：

- `include/PhantomDrive/UI/mainwindow.h`
- `src/UI/mainwindow.cpp`

## 4. 双人模式赛前界面

- Two-Player 入口不再直接开始比赛。
- 新增与 Arcade 风格一致的 `TWO-PLAYER SETUP` 界面。
- 双人界面支持独立赛道和 AI 难度选择。
- 玩家阵容固定为 `2 Players + AI`，避免误切回单人。
- 双人选择会被独立记忆，不影响 Arcade 的赛道设置。
- 主菜单中的 `TWO-PLAYER / AI DEMO` 入口已接入该界面。

## 5. README 与截图

更新了根目录、源码目录和 Windows Release 中的 README：

- `README.md`
- `source/README.md`
- `PhantomDrive_Windows_Release/README.md`

加入的最新截图包括：

- 主菜单
- 游戏模式选择
- Race Setup
- Coin Challenge HUD
- Balloon Rush 过场
- Balloon Rush 玩法
- Coin Challenge 结算

图片存放于：

- `source/docs/images/readme/`
- `PhantomDrive_Windows_Release/docs/images/readme/`

同时修正了 README 中过期的 Windows Release 目录与程序名称：

- Release 目录：`PhantomDrive_Windows_Release`
- 启动程序：`PhantomDrive.exe`

## 6. Release 同步与验证

每轮功能修改后均执行 Release 构建，并同步：

- `PhantomDrive.exe`
- `libPhantomDrive.dll`
- 新增界面资源
- README
- README 截图

最终验证结果：

- Release 构建成功。
- 程序能够正常启动并保持响应。
- 构建目录与 Release 的核心二进制文件哈希一致。
- Release README 中引用的截图均存在。

## 7. 当前交付位置

- 源码：`source/`
- 文档：`source/docs/`
- Windows Release：`PhantomDrive_Windows_Release/`
- 启动程序：`PhantomDrive_Windows_Release/PhantomDrive.exe`
