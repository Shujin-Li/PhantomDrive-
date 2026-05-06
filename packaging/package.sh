#!/bin/bash
# PhantomDrive 打包脚本（Linux）
# 完全照搬 DustRacing2D 的打包方式

set -e

echo "=== PhantomDrive 打包脚本 ==="
echo ""

# 配置变量
BUILD_TYPE=${1:-Release}
BUILD_DIR="build_package"
QT_PATH=${QT6_DIR:-"/home/crocodile/Qt/6.8.3/gcc_64"}

echo "构建类型：$BUILD_TYPE"
echo "Qt 路径：$QT_PATH"
echo ""

# 清理旧的构建目录
if [ -d "$BUILD_DIR" ]; then
    echo "清理旧的构建目录..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 配置 CMake
echo "配置 CMake..."
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DCMAKE_PREFIX_PATH=$QT_PATH/lib/cmake \
      -DReleaseBuild=ON \
      ..

# 编译
echo ""
echo "编译项目..."
make -j$(nproc)

# 安装到临时目录
echo ""
echo "安装到临时目录..."
make install DESTDIR=appdir

# 使用 linuxdeployqt 部署 Qt 依赖（如果可用）
if command -v linuxdeployqt &> /dev/null; then
    echo ""
    echo "部署 Qt 依赖..."
    linuxdeployqt appdir/usr/bin/phantomdrive-game \
                  -appimage \
                  -verbose=1
else
    echo ""
    echo "警告：linuxdeployqt 未安装，跳过 Qt 依赖部署"
    echo "请手动复制 Qt 库或使用其他部署工具"
fi

# 创建压缩包
echo ""
echo "创建压缩包..."
cd appdir
tar -czvf ../PhantomDrive-${BUILD_TYPE}-linux-x86_64.tar.gz .
cd ..

echo ""
echo "=== 打包完成 ==="
echo "输出文件：PhantomDrive-${BUILD_TYPE}-linux-x86_64.tar.gz"
echo ""

# 清理
read -p "是否清理临时文件？(y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf appdir
    echo "已清理临时文件"
fi

echo ""
echo "打包成功！"
