# PhantomDrive Speed Limit Sign Variants

这些图片基于现有 `pd_speed_limit_60.png` 统一生成，保持同一风格、透明背景、512x512 PNG。

## 文件
- pd_speed_limit_20.png
- pd_speed_limit_30.png
- pd_speed_limit_40.png
- pd_speed_limit_50.png
- pd_speed_limit_60.png
- pd_speed_limit_70.png
- pd_speed_limit_80.png
- pd_speed_limit_90.png
- pd_speed_limit_100.png
- pd_speed_limit_120.png

## Codex 接入建议
放入：
`source/assets/visual_upgrade/phantomdrive_direct_use_assets/traffic/`

如果项目有不同限速值，可按 speedLimitKmh 动态选择：
- 20 -> pd_speed_limit_20.png
- 30 -> pd_speed_limit_30.png
- 40 -> pd_speed_limit_40.png
- 50 -> pd_speed_limit_50.png
- 60 -> pd_speed_limit_60.png
- 70 -> pd_speed_limit_70.png
- 80 -> pd_speed_limit_80.png
- 90 -> pd_speed_limit_90.png
- 100 -> pd_speed_limit_100.png
- 120 -> pd_speed_limit_120.png

如果找不到对应值，fallback 到 `pd_speed_limit_60.png` 或旧 QPainter 绘制。
