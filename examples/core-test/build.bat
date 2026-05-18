@echo off
chcp 65001 >nul
echo ========================================
echo   PhantomDrive Core Test Build Script
echo ========================================
echo.

set ROOT_DIR=%~dp0..
set BUILD_DIR=%ROOT_DIR%\build-core-test

echo [1] 清理旧的构建目录...
if exist "%BUILD_DIR%" rd /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"

echo [2] 配置 CMake 项目...
cd /d "%BUILD_DIR%"
cmake "%ROOT_DIR%\examples\core-test" -G "Qt Creator" 2>&1

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] CMake 配置失败！
    echo 请确保已安装 Qt 6.8.3 和 CMake 3.20+
    echo.
    echo 尝试使用 Ninja 构建器...
    cmake "%ROOT_DIR%\examples\core-test" -G "Ninja" -DCMAKE_BUILD_TYPE=Release
)

echo.
echo [3] 编译项目...
cmake --build . --config Release

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo   编译成功！
    echo ========================================
    echo.
    echo 运行测试: .\bin\core-test.exe
    echo.
) else (
    echo.
    echo [ERROR] 编译失败！
    echo.
)

pause
