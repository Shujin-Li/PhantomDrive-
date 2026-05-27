#!/bin/bash
# PhantomDrive 打包脚本
# 使用方法：./package.sh

set -e

echo "=== PhantomDrive 打包脚本 ==="

# 设置变量
PROJECT_NAME="PhantomDrive"
VERSION="1.0.0"
BUILD_DIR="build"
PACKAGE_DIR="${PROJECT_NAME}-${VERSION}"
OUTPUT_FILE="${PROJECT_NAME}-${VERSION}-linux-x86_64.tar.gz"

# 清理旧的打包文件
rm -rf ${PACKAGE_DIR} ${OUTPUT_FILE}

echo "1. 编译项目..."
cd ${BUILD_DIR}
make clean
make -j$(nproc)
cd ..

echo "2. 创建打包目录..."
mkdir -p ${PACKAGE_DIR}
mkdir -p ${PACKAGE_DIR}/bin
mkdir -p ${PACKAGE_DIR}/lib
mkdir -p ${PACKAGE_DIR}/include
mkdir -p ${PACKAGE_DIR}/data

echo "3. 复制库文件..."
cp ${BUILD_DIR}/libPhantomDrive.so ${PACKAGE_DIR}/lib/

echo "4. 复制头文件..."
cp -r include/PhantomDrive ${PACKAGE_DIR}/include/

echo "5. 复制数据文件..."
if [ -d "data" ]; then
    cp -r data/* ${PACKAGE_DIR}/data/
fi

echo "6. 复制文档..."
cp README.md ${PACKAGE_DIR}/ 2>/dev/null || echo "无 README.md"
cp LICENSE ${PACKAGE_DIR}/ 2>/dev/null || echo "无 LICENSE"
cp DRIVING_DATA_COLLECTOR_README.md ${PACKAGE_DIR}/ 2>/dev/null || echo "无额外文档"

echo "7. 创建使用说明文件..."
cat > ${PACKAGE_DIR}/README.txt << EOF
PhantomDrive ${VERSION}
=====================

这是一个基于 Qt6 的游戏框架，包含：
- GameMode 抽象基类和模式框架
- 自定义赛道加载系统
- DrivingDataCollector 数据采集系统
- MiniCore 物理引擎（来自 DustRacing2D）

目录结构：
- bin/          可执行文件
- lib/          库文件
- include/      头文件
- data/         数据文件

依赖项：
- Qt 6.8+
- OpenGL

使用方法：
1. 确保已安装 Qt 6.8+
2. 设置环境变量：
   export CMAKE_PREFIX_PATH=/path/to/Qt/6.8/gcc_64/lib/cmake
3. 编译：
   mkdir build && cd build
   cmake ..
   make
4. 运行测试程序（如果有）

作者：Your Name
日期：$(date +%Y-%m-%d)
EOF

echo "8. 创建依赖检查脚本..."
cat > ${PACKAGE_DIR}/check_dependencies.sh << 'EOF'
#!/bin/bash
echo "检查 PhantomDrive 依赖..."

# 检查 Qt6
if command -v qmake6 &> /dev/null; then
    echo "✓ Qt6 已安装：$(qmake6 --version | head -1)"
else
    echo "✗ Qt6 未安装"
    echo "  请安装 Qt 6.8+ 或设置 CMAKE_PREFIX_PATH"
fi

# 检查 OpenGL
if ldconfig -p | grep -q libGL; then
    echo "✓ OpenGL 已安装"
else
    echo "✗ OpenGL 未安装"
    echo "  请安装：sudo apt-get install libgl1-mesa-dev"
fi

# 检查 GLEW
if ldconfig -p | grep -q libGLEW; then
    echo "✓ GLEW 已安装"
else
    echo "ℹ GLEW 未使用系统库（项目自带 GLEW）"
fi

echo "依赖检查完成！"
EOF
chmod +x ${PACKAGE_DIR}/check_dependencies.sh

echo "9. 创建 CMake 配置示例..."
cat > ${PACKAGE_DIR}/example_cmake.sh << 'EOF'
#!/bin/bash
# CMake 配置示例

# 设置 Qt6 路径（根据你的安装位置修改）
export CMAKE_PREFIX_PATH=/home/crocodile/Qt/6.8.3/gcc_64/lib/cmake

# 创建 build 目录
mkdir -p build
cd build

# 运行 CMake
cmake ..

# 编译
make -j$(nproc)

echo "编译完成！"
EOF
chmod +x ${PACKAGE_DIR}/example_cmake.sh

echo "10. 打包..."
tar -czvf ${OUTPUT_FILE} ${PACKAGE_DIR}/

echo "11. 清理临时文件..."
rm -rf ${PACKAGE_DIR}

echo ""
echo "=== 打包完成！ ==="
echo "输出文件：${OUTPUT_FILE}"
echo ""
echo "文件大小：$(du -h ${OUTPUT_FILE} | cut -f1)"
echo ""
echo "分发说明："
echo "1. 将 ${OUTPUT_FILE} 分发给用户"
echo "2. 用户解压后运行 ./check_dependencies.sh 检查依赖"
echo "3. 按照 README.txt 中的说明进行编译和使用"
