@echo off
REM 键盘控制修复测试脚本

echo ========================================
echo PhantomDrive528 键盘控制修复测试
echo ========================================
echo.

echo 步骤 1: 打开 Visual Studio Developer Command Prompt
echo 或者确保 cmake 在 PATH 环境变量中
echo.

echo 步骤 2: 编译项目
echo cd /d "d:\c++ 大作业\PhantomDrive528\build"
echo cmake ..
echo cmake --build . --config Release
echo.

echo 步骤 3: 运行测试程序
echo cd /d "d:\c++ 大作业\PhantomDrive528\build\bin"
echo test_main_window.exe
echo.

echo ========================================
echo 键盘控制测试清单:
echo ========================================
echo 1. W/上箭头 - 加速 (应该平滑加速)
echo 2. S/下箭头 - 刹车/倒车 (应该先减速再倒车)
echo 3. A/左箭头 - 左转 (应该平滑转向)
echo 4. D/右箭头 - 右转 (应该平滑转向)
echo 5. 空格键 - 手刹 (应该快速减速)
echo 6. 同时按多个键 (应该正确响应组合键)
echo.

echo ========================================
echo 修复内容:
echo ========================================
echo 1. GameViewWidget 自动获取焦点
echo 2. 使用 QSet 跟踪所有按下的键
echo 3. updateControlStates() 统一更新控制状态
echo 4. 添加调试输出显示按键状态
echo 5. 改进 keyPressEvent/keyReleaseEvent 处理
echo.

echo 按任意键关闭...
pause > nul
