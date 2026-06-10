# Speed Display Unit Fix - 2026-05-30

## 问题

玩家车和 AI 车底层都使用同一套内部物理速度，但 UI 显示时只有玩家速度经过了 km/h 映射，AI 车标签直接把内部速度当成 km/h 显示，导致看起来 AI 速度比玩家大很多。

## 修复

- 新增 `MainWindow::speedToDisplayKmh(qreal physicsSpeed)`，统一内部物理速度到 km/h 的显示换算。
- `MainWindow::displaySpeedKmh()` 改为调用统一换算函数。
- AI 车初始化显示和运行中刷新都改为 `speedToDisplayKmh(ai->getSpeed())`。
- 玩家传感器写入的速度改为 km/h 显示单位，使 `DrivingData`、限速比较、报告速度曲线保持同一单位。
- 碰撞事件、Safe Driving tick、DrivingReportWidget 实时速度曲线改为使用 km/h 显示速度。
- 保留内部 `m_playerSpeed` / `ai->getSpeed()` 给物理移动和碰撞计算使用，不影响车辆实际移动。

## 验证

- VS2022 + Qt 6.8.3 + CMake/Ninja Release 构建通过。
- `a_b_ddl_event_integration_demo.exe` 运行通过。
- `a_a_race_integration_test.exe` 运行通过。

