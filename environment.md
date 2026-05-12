# PhantomDrive 跨平台代码适配指南

## 已发现的硬编码问题

### ❌ 需要修改的文件

#### 1. `packaging/package.sh` - Linux 打包脚本

**问题位置**：第 13 行

```bash
# ❌ 硬编码了 Linux 用户的 Qt 路径
QT_PATH=${QT6_DIR:-"/home/crocodile/Qt/6.8.3/gcc_64"}
```

**修改方式**：
```bash
# ✅ 方式 1：使用环境变量，不提供默认值
QT_PATH=${QT6_DIR}

# ✅ 方式 2：添加 Windows 支持
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    # Windows
    QT_PATH=${QT6_DIR:-"C:/Qt/6.8.3/msvc2019_64"}
else
    # Linux
    QT_PATH=${QT6_DIR:-"/home/crocodile/Qt/6.8.3/gcc_64"}
fi
```

**或者**：
```bash
# ✅ 方式 3：完全移除默认值，要求用户自己设置
if [ -z "$QT6_DIR" ]; then
    echo "错误：请设置 QT6_DIR 环境变量指向 Qt 安装目录"
    echo "Windows 示例：set QT6_DIR=C:\\Qt\\6.8.3\\msvc2019_64"
    echo "Linux 示例：export QT6_DIR=/path/to/Qt/6.8.3/gcc_64"
    exit 1
fi
QT_PATH=$QT6_DIR
```

***

#### 2. `packaging/InstallLinux.cmake` - Linux 安装脚本

**问题位置**：需要检查是否有硬编码路径

**检查内容**：
- 查找所有 `"/home/"` 路径
- 查找所有 `"/opt/"` 路径
- 查找所有 `"$ENV{HOME}"` 引用

***

#### 3. `packaging/InstallWindows.cmake` - 需要创建

**Windows 团队成员需要**：
1. 复制 `InstallLinux.cmake` 为模板
2. 修改为 Windows 安装逻辑
3. 使用 Windows 路径格式

***

## 需要检查的其他文件

### ⚠️ 可能有问题但不确定

以下文件在 build 目录中（自动生成，不需要修改）：
- `examples/*/build/CMakeFiles/Makefile.cmake`
- `examples/*/build/cmake_install.cmake`
- 其他 CMake 生成的文件

**这些文件是自动生成的，不需要修改！**

***

## 快速检查命令

### Windows 团队成员可以运行

```bash
# 在项目根目录运行
grep -r "/home/" --include="*.sh" --include="*.cmake" --include="*.cpp" --include="*.h" .
grep -r "system(" --include="*.cpp" --include="*.h" .
grep -r "#include <unistd.h>" --include="*.h" --include="*.cpp" .
```

**如果找到结果**：
- `.sh` 或 `.cmake` 文件中的路径 → 需要修改为跨平台或可配置
- `.cpp` 或 `.h` 文件中的 `system()` → 需要改为 Qt API
- Linux 头文件 → 需要改为 Qt 替代或 `#ifdef` 包裹

***

## 总结

### 已确认需要修改

- [x] `packaging/package.sh` - 硬编码 Qt 路径

### 需要检查

- [ ] `packaging/InstallLinux.cmake` - 检查是否有硬编码路径
- [ ] 其他 `.sh` 脚本文件
- [ ] 其他 `.cmake` 配置文件

### 不需要修改

- [x] build 目录中的文件（自动生成）
- [x] 源代码文件（已检查，没有硬编码）

***

**最后更新**：2026-05-07
