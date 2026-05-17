<br />

# 📦 PhantomDrive 打包文档（trae生成，仅供参考）

## 第一部分：DustRacing2D 打包思路详解

### 整体架构

DustRacing2D 使用 **CMake + CPack** 的打包方案，核心思想是：

```
编译 → 安装到临时目录 → 打包 → 分发给用户
```

***

### 🔑 核心组件

#### 1️⃣ **CMakeLists.txt（主配置）**

```cmake
# 版本配置
set(VERSION_MAJOR 2)
set(VERSION_MINOR 2)
set(VERSION_PATCH 0)

# CPack 变量
set(CPACK_PACKAGE_NAME dustracing2d)
set(CPACK_PACKAGE_VENDOR Juzzlin)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A traditional top-down racing game.")

# 打包选项
option(ReleaseBuild "创建发布版本" OFF)
option(DATA_PATH "数据文件路径" "")
option(BIN_PATH "二进制文件路径" "")

# 根据平台选择安装脚本
if(UNIX)
    include("InstallLinux.cmake")
elseif(WIN32)
    include("InstallWindows.cmake")
endif()

# CPack 配置
include(CPack)
```

**作用**：

- 定义版本号
- 设置 CPack 打包工具的参数
- 根据平台选择安装脚本

***

#### 2️⃣ **InstallLinux.cmake（Linux 安装脚本）**

这是打包的核心！分为两部分：

**A. 开发版本（默认）**

```cmake
# 复制到构建目录
add_custom_target(runtime ALL
    COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
    COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/AUTHORS ${CMAKE_BINARY_DIR}/AUTHORS
    COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/CHANGELOG ${CMAKE_BINARY_DIR}/CHANGELOG
    COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/COPYING ${CMAKE_BINARY_DIR}/COPYING
    COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/README.md ${CMAKE_BINARY_DIR}/README.md
)
```

**作用**：编译时自动复制资源文件到 `build/data` 目录

**B. 发布版本（-DReleaseBuild=ON）**

```cmake
# 设置安装路径
set(DATA_PATH ${CMAKE_INSTALL_PREFIX}/share/games/DustRacing2D/data)
set(BIN_PATH bin)

# 创建 DEB 包配置
set(CPACK_DEBIAN_PACKAGE_NAME "dustracing2d")
set(CPACK_DEBIAN_PACKAGE_VERSION ${VERSION})
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jussi Lind")

# 包含 CPack
include(CPack)
```

**作用**：

- 设置系统安装路径（`/usr/bin`, `/usr/share/games/...`）
- 配置 DEB 包的元数据
- 自动生成 `.deb` 安装包

***

#### 3️⃣ **InstallWindows.cmake（Windows 安装脚本）**

```cmake
# Windows 所有文件都在同一目录
set(BIN_PATH .)
set(DATA_PATH ./data)

# 复制资源文件
add_custom_target(runtimeData ALL
    COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
    COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/AUTHORS ${CMAKE_BINARY_DIR}/AUTHORS
    ...
)

# 安装配置
install(PROGRAMS ${CMAKE_BINARY_DIR}/dustrac-game.exe DESTINATION .)
install(PROGRAMS ${CMAKE_BINARY_DIR}/dustrac-editor.exe DESTINATION .)
install(DIRECTORY data/images DESTINATION ./data)
install(DIRECTORY data/levels DESTINATION ./data)
...
```

**作用**：

- Windows 使用**便携版**结构（所有文件在同一目录）
- 为 NSIS 安装程序做准备

***

#### 4️⃣ **packaging/windows/dustrac.nsi（NSIS 安装脚本）**

```nsis
; NSIS 安装器配置
!define PRODUCTNAME "Dust Racing 2D"
!define VERSIONMAJOR 2
!define VERSIONMINOR 2

; 安装包输出文件名
OutFile "dustracing2d-2.2.0-windows-x86-qt5_setup.exe"

; 安装目录
InstallDir "$PROGRAMFILES\${PRODUCTNAME}"

; 安装页面
Page license          ; 许可证
Page directory        ; 选择安装目录
Page instfiles        ; 安装进度

; 安装内容
Section "Install"
  SetOutPath "$INSTDIR"
  File /r "build\release\*.*"  ; 复制所有文件
SectionEnd

; 卸载程序
Section "Uninstall"
  DeleteRegKey HKLM "Software\${PRODUCTNAME}"
  RMDir /r "$INSTDIR"
SectionEnd
```

**作用**：

- 创建 Windows 安装程序（`.exe`）
- 提供图形化安装界面
- 自动创建卸载程序

***

### 📋 打包流程

#### **Linux 打包流程**

```bash
# 1. 配置（开发版本）
mkdir build && cd build
cmake ..

# 2. 编译
make -j$(nproc)

# 结果：build/dustrac-game 和 build/data/

# --- 或者发布版本 ---

# 1. 配置（发布版本）
cmake -DReleaseBuild=ON ..

# 2. 编译
make -j$(nproc)

# 3. 打包
cpack -G DEB  # 创建 .deb 包
cpack -G RPM  # 创建 .rpm 包
cpack -G TGZ  # 创建 .tar.gz

# 结果：dustracing2d-2.2.0_linux_amd64.deb
```

#### **Windows 打包流程**

```bash
# 1. 配置
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=C:\Qt\6.8.3\msvc2019_64\lib\cmake ..

# 2. 编译
cmake --build . --config Release

# 3. 部署 Qt 库
windeployqt release/dustrac-game.exe

# 4. 创建安装程序
cd ../packaging/windows
makensis dustrac.nsi

# 结果：dustracing2d-2.2.0-windows-x86-qt5_setup.exe
```

***

### 🎯 关键设计思想

#### 1. **跨平台一致性**

- Linux 和 Windows 使用相同的 CMake 配置
- 通过 `Install*.cmake` 文件处理平台差异

#### 2. **开发和发布分离**

- **开发版本**：直接运行在 `build/` 目录
- **发布版本**：安装到系统目录或创建安装包

#### 3. **资源文件管理**

- 所有资源文件（图片、声音、字体）都在 `data/` 目录
- 编译时自动复制到 `build/data/`
- 打包时一起打包

#### 4. **依赖处理**

- **Linux**：使用系统包管理器（apt/dnf）安装 Qt
- **Windows**：使用 `windeployqt` 工具自动复制 Qt 库

#### 5. **CPack 集成**

- CMake 内置的打包工具
- 支持多种格式：DEB, RPM, TGZ, NSIS, ZIP
- 自动生成包的元数据

***

### 📊 打包结构对比

| 平台          | 结构   | 安装位置                         | 打包格式        |
| :---------- | :--- | :--------------------------- | :---------- |
| **Linux**   | 系统安装 | `/usr/bin`, `/usr/share/...` | DEB/RPM/TGZ |
| **Windows** | 便携版  | `C:\Program Files\...`       | NSIS/ZIP    |

***

### 💡 总结

DustRacing2D 的打包思路可以概括为：

1. **CMake 统一配置** - 跨平台编译
2. *Install*.cmake 处理平台差异\* - Linux 系统安装 vs Windows 便携版
3. **CPack 自动打包** - 生成安装包
4. **NSIS 创建 Windows 安装程序** - 提供图形化安装界面
5. **windeployqt/linuxdeployqt 处理 Qt 依赖** - 自动复制运行时库

这种打包方式的优点：

- ✅ **自动化**：一条命令完成打包
- ✅ **可重复**：每次打包结果一致
- ✅ **跨平台**：Linux 和 Windows 使用相同的思路
- ✅ **专业**：生成标准的安装包（DEB/RPM/NSIS）

***

## 第二部分：PhantomDrive 的打包实现

### 🚀 直接照搬 DustRacing2D 的方案

PhantomDrive 已经完全复制了 DustRacing2D 的打包配置：

#### 已复制的文件

```
PhantomDrive--main/
├── CMakeLists.txt              # 已更新，包含 CPack 配置
├── InstallLinux.cmake          # ✓ 已复制（从 DustRacing2D）
├── InstallWindows.cmake        # ✓ 已复制（从 DustRacing2D）
├── packaging/
│   ├── windows/
│   │   └── phantomdrive.nsi    # ✓ 已复制并修改
│   └── linux/
│       └── package.sh          # ✓ 打包脚本
├── data/
│   ├── images/                 # 游戏资源
│   └── fonts/                  # 字体资源
└── src/
    ├── gamemode/               # 游戏模式
    ├── track/                  # 赛道系统
    └── physics/                # MiniCore 物理引擎
```

#### 已集成的组件

1. **MiniCore 物理引擎** - 从 DustRacing2D 完整复制
   - 物理碰撞检测
   - 力系统（重力、摩擦力等）
   - 形状系统（圆形、矩形等）
2. **DrivingDataCollector** - 数据采集系统
   - 车辆传感器（位置、速度、转向角）
   - 碰撞检测器
   - 数据存储和导出
3. **GameMode 框架** - 游戏模式管理
   - ArcadeMode（街机模式）
   - LearningMode（学习模式）

***

### 📦 PhantomDrive 打包步骤

#### **Linux 平台**

```bash
# 方法 1：使用打包脚本（推荐）
cd PhantomDrive--main
./packaging/package.sh Release

# 方法 2：手动打包
mkdir build && cd build

# 1. 配置
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.8.3/gcc_64/lib/cmake \
      -DReleaseBuild=ON \
      ..

# 2. 编译
make -j$(nproc)

# 3. 创建安装包
cpack -G DEB    # 创建 .deb 包
cpack -G TGZ    # 创建压缩包

# 输出文件：
# - phantomdrive-1.0.0_linux_amd64.deb
# - phantomdrive-1.0.0_linux_amd64.tar.gz
```

#### **Windows 平台**

```bash
# 1. 配置
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=C:\Qt\6.8.3\msvc2019_64\lib\cmake ..

# 2. 编译
cmake --build . --config Release

# 3. 部署 Qt 库
C:\Qt\6.8.3\msvc2019_64\bin\windeployqt.exe release\phantomdrive-game.exe

# 4. 创建安装程序
cd ../packaging/windows
makensis phantomdrive.nsi

# 输出文件：phantomdrive-1.0.0-windows-x86_64-setup.exe
```

***

### 🔧 跨平台编译配置

#### **问题：不同电脑 Qt 安装路径不同怎么办？**

**解决方案 1：使用 CMAKE\_PREFIX\_PATH**

```bash
# 电脑 A - Qt 在 /opt/Qt
cmake -DCMAKE_PREFIX_PATH=/opt/Qt/6.8.3/gcc_64/lib/cmake ..

# 电脑 B - Qt 在 /home/user/Qt
cmake -DCMAKE_PREFIX_PATH=/home/user/Qt/6.8.3/gcc_64/lib/cmake ..

# 电脑 C - 使用 conda 环境
conda activate dustracing_qt68
cmake -DCMAKE_PREFIX_PATH=$CONDA_PREFIX/lib/cmake ..
```

**解决方案 2：使用环境变量**

```bash
# 设置 Qt 路径
export Qt6_DIR=/opt/Qt/6.8.3/gcc_64/lib/cmake/Qt6
cmake ..
```

**解决方案 3：使用 pkg-config（Linux）**

```bash
# pkg-config 自动查找 Qt
export PKG_CONFIG_PATH=/opt/Qt/lib/pkgconfig:$PKG_CONFIG_PATH
cmake ..
```

***

### 📊 PhantomDrive 依赖项

#### **编译时依赖**

| 依赖库    | 版本要求  | 用途         | 查找方式                  |
| ------ | ----- | ---------- | --------------------- |
| Qt6    | 6.4+  | GUI、OpenGL | find\_package(Qt6)    |
| OpenGL | 2.1+  | 3D 渲染      | find\_package(OpenGL) |
| OpenAL | 任意    | 音频         | find\_package(OpenAL) |
| CMake  | 3.10+ | 构建系统       | 系统自带                  |

#### **运行时依赖**

**Linux：**

```bash
# Ubuntu/Debian
sudo apt install libqt6core6 libqt6gui6 libqt6widgets6 \
                 libqt6opengl6 libqt6xml6 libqt6sql6 \
                 libgl1-mesa-glx libopenal1

# Fedora
sudo dnf install qt6-qtbase qt6-qtbase-gui qt6-qtbase-widgets \
                 qt6-qtbase-opengl qt6-qtbase-xml \
                 mesa-libGL openal-soft
```

**Windows：**

- 包含在 windeployqt 自动部署的 Qt 库中
- OpenAL32.dll（需要单独安装包含在包中）

***

### ✅ 打包检查清单

在发布前，确保完成以下步骤：

- [ ] **编译成功**
  - [ ] Linux: `make -j$(nproc)` 无错误
  - [ ] Windows: `cmake --build . --config Release` 无错误
- [ ] **资源文件完整**
  - [ ] `data/` 目录已复制到 `build/data/`
  - [ ] 图片、字体、赛道文件齐全
- [ ] **依赖库部署**
  - [ ] Linux: 使用 `ldd phantomdrive-game` 检查缺失库
  - [ ] Windows: `windeployqt` 已复制所有 Qt 库
- [ ] **打包完成**
  - [ ] Linux: `.deb` 或 `.tar.gz` 文件已生成
  - [ ] Windows: `.exe` 安装程序已生成
- [ ] **测试运行**
  - [ ] 在干净的系统上测试安装
  - [ ] 验证游戏可以正常启动
  - [ ] 测试所有功能（赛道加载、物理引擎等）

***

### 🐛 常见问题解决

#### **问题 1：CMake 找不到 Qt**

```bash
# 错误信息：Could not find Qt6
# 解决方案：
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.8.3/gcc_64/lib/cmake ..
```

#### **问题 2：运行时找不到 Qt 库（Linux）**

```bash
# 错误：error while loading shared libraries: libQt6Core.so.6
# 解决方案 1：使用 linuxdeployqt
linuxdeployqt build/phantomdrive-game -appimage

# 解决方案 2：设置 LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/path/to/Qt/lib:$LD_LIBRARY_PATH
```

#### **问题 3：OpenGL 错误**

```bash
# 错误：QOpenGLContext::makeCurrent: Cannot make current
# 解决方案：确保显卡驱动支持 OpenGL 2.1+
# 检查：glxinfo | grep "OpenGL version"
```

#### **问题 4：OpenAL 音频库缺失**

```bash
# Linux:
sudo apt install libopenal1

# Windows:
# 复制 OpenAL32.dll 到可执行文件目录
```

***

### 📝 版本发布说明

#### **版本号规则**

```
主版本号。次版本号。修订号
例如：1.0.0
  - 主版本号：重大更新（架构变化）
  - 次版本号：新功能添加
  - 修订号：Bug 修复
```

#### **发布流程**

1. 更新 `CMakeLists.txt` 中的版本号
2. 更新 `CHANGELOG` 文件
3. 编译并测试
4. 打包
5. 创建 Git 标签：`git tag v1.0.0`
6. 推送标签：`git push origin v1.0.0`

***

### 🎓 学习资源

- **CMake 官方文档**：<https://cmake.org/documentation/>
- **CPack 使用指南**：<https://cmake.org/cmake/help/latest/module/CPack.html>
- **Qt 部署文档**：<https://doc.qt.io/qt-6/deployment.html>
- **linuxdeployqt**：<https://github.com/probonopd/linuxdeployqt>
- **NSIS 文档**：<https://nsis.sourceforge.io/>

***

### 👥 小组协作提示

1. **统一 Qt 版本**：所有成员使用相同的 Qt 版本（6.8.3）
2. **共享构建脚本**：使用 `packaging/package.sh` 确保一致性
3. **测试环境**：在虚拟机或 Docker 容器中测试打包结果
4. **文档更新**：每次修改打包配置后更新本文档

***

**最后更新**：2026-05-05\
**维护者**：PhantomDrive 开发团队
