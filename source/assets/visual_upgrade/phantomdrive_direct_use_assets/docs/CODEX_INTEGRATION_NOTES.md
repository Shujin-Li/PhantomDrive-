# PhantomDrive Direct Use Assets

## 目录说明
- `menu/`：主菜单背景
- `powerups/`：9 个道具图标（透明底 PNG）
- `traffic/`：行人、斑马线、限速牌（透明底 PNG）
- `source_sheets/`：原始生成总图，留档

## 直接给 Codex 的接入要求
1. 用 `menu/pd_main_menu_bg_phantomdrive_1920x1080.png` 作为主菜单背景。
2. 删除当前界面里单独绘制/显示的 `PHANTOMDRIVE` 标题文字，因为背景图里已经包含标题。
3. 用 `powerups/` 下的图标替换当前游戏里的道具图标。
4. **保留当前道具名称文字**（Boost / Shield / Missile / Oil / EMP / Invisibility / Repair / Teleport / Magnet 等现有文字照旧显示），只替换图标，不删除文字标签。
5. 红绿灯不要替换图片，继续使用项目当前已有红绿灯素材；仅将当前红绿灯整体视觉尺寸放大约 `1.10x ~ 1.15x`。
6. 用 `traffic/pd_pedestrian_walk.png` 替换/美化行人图。
7. 用 `traffic/pd_crosswalk_stripes.png` 替换/美化斑马线。
8. 用 `traffic/pd_speed_limit_60.png` 替换/美化限速牌。
9. 所有新素材都已整理为透明底 PNG，适合直接放入资源目录并由 Qt 加载。

## 建议资源命名映射
- Boost -> `pd_powerup_boost.png`
- Shield -> `pd_powerup_shield.png`
- Missile -> `pd_powerup_missile.png`
- Oil -> `pd_powerup_oil.png`
- EMP -> `pd_powerup_emp.png`
- Invisibility -> `pd_powerup_invisibility.png`
- Repair -> `pd_powerup_repair.png`
- Teleport -> `pd_powerup_teleport.png`
- Magnet -> `pd_powerup_magnet.png`