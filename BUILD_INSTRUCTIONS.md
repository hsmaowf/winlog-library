# WinLog 构建说明文档

## 🏗️ 概述

WinLog 是一个 Windows 平台专用的日志库，使用 C++ 开发。本文档详细介绍了如何构建 WinLog 项目，包括环境准备、编译步骤和常见问题解决。

## 📋 系统要求

### 必需环境
- **操作系统**：Windows 7 或更高版本
- **编译器**：MinGW-w64 或 Visual Studio
- **C++标准**：C++11 或更高版本

### 推荐环境
- **操作系统**：Windows 10/11
- **编译器**：MinGW-w64 8.0+ 或 Visual Studio 2019+
- **内存**：至少 4GB RAM
- **磁盘空间**：至少 1GB 可用空间

## 🔧 环境准备

### 1. 安装 MinGW-w64

#### 方法一：使用 MSYS2（推荐）

1. 下载并安装 [MSYS2](https://www.msys2.org/)
2. 打开 MSYS2 Shell，执行：
```bash
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make
```

3. 将 `C:\msys64\mingw64\bin` 添加到系统 PATH

#### 方法二：直接安装 MinGW-w64

1. 下载 [MinGW-w64 安装器](https://sourceforge.net/projects/mingw-w64/)
2. 选择以下配置：
   - Version: 8.1.0 或更高
   - Architecture: x86_64
   - Threads: posix
   - Exception: seh
3. 安装完成后将 `bin` 目录添加到系统 PATH

### 2. 验证安装

打开命令提示符，执行：
```cmd
gcc --version
g++ --version
make --version
```

确保所有命令都能正常执行。

## 🚀 编译步骤

### 步骤 1：准备源代码

确保项目结构如下：
```
WinLog/
├── include/
│   └── winlog.h
├── src/
│   └── winlog.cpp
└── examples/
    └── example.cpp
```

### 步骤 2：编译 DLL（动态链接库）

#### 使用 MinGW-w64

```cmd
# 进入项目目录
cd WinLog

# 编译 DLL
g++ -shared -o bin/WinLog.dll src/winlog.cpp -I include -DWINDOWS -DWINGDIAPI= -DWINLOG_EXPORTS -static-libgcc -static-libstdc++ -Wl,--out-implib,lib/WinLog.lib

# 参数说明：
# -shared: 创建 DLL
# -o bin/WinLog.dll: 输出文件
# -I include: 包含目录
# -DWINDOWS: Windows 平台定义
# -DWINGDIAPI=: Windows GDI API 定义
# -DWINLOG_EXPORTS: 导出符号定义
# -static-libgcc -static-libstdc++: 静态链接运行时库
# -Wl,--out-implib,lib/WinLog.lib: 生成导入库
```

#### 使用 Visual Studio

```cmd
# 使用 Visual Studio 开发者命令提示符
cl /LD /Fe:bin\WinLog.dll src\winlog.cpp /I include /DWINLOG_EXPORTS /link /OUT:bin\WinLog.dll /IMPLIB:lib\WinLog.lib
```

### 步骤 3：编译示例程序

#### 编译示例程序

```cmd
# 编译示例程序
g++ -o examples/example.exe examples/example.cpp -I include -L lib -lWinLog -static-libgcc -static-libstdc++

# 参数说明：
# -o examples/example.exe: 输出可执行文件
# -I include: 包含头文件目录
# -L lib: 库文件目录
# -lWinLog: 链接 WinLog 库
# -static-libgcc -static-libstdc++: 静态链接运行时库
```

### 步骤 4：验证编译结果

```cmd
# 检查生成的文件
dir bin\
dir lib\
dir examples\

# 运行示例程序
examples\example.exe
```

## 📦 构建脚本

### 自动构建脚本（build.bat）

创建 `build.bat` 文件：

```batch
@echo off
setlocal enabledelayedexpansion

echo WinLog 构建脚本
echo ===================

REM 检查编译器
where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo 错误：未找到 g++ 编译器
    echo 请确保 MinGW-w64 已安装并添加到 PATH
    exit /b 1
)

echo 找到 g++ 编译器

REM 创建目录
if not exist "bin" mkdir bin
if not exist "lib" mkdir lib

echo 开始编译...

REM 编译 DLL
echo 1. 编译 WinLog.dll...
g++ -shared -o bin/WinLog.dll src/winlog.cpp -I include -DWINDOWS -DWINGDIAPI= -DWINLOG_EXPORTS -static-libgcc -static-libstdc++ -Wl,--out-implib,lib/WinLog.lib
if %errorlevel% neq 0 (
    echo 错误：DLL 编译失败
    exit /b 1
)
echo ✓ DLL 编译成功

REM 编译示例程序
echo 2. 编译示例程序...
g++ -o examples/example.exe examples/example.cpp -I include -L lib -lWinLog -static-libgcc -static-libstdc++
if %errorlevel% neq 0 (
    echo 错误：示例程序编译失败
    exit /b 1
)
echo ✓ 示例程序编译成功

echo.
echo 构建完成！
echo 生成文件：
dir /b bin\*.dll
lib\*.lib
examples\*.exe
echo.
echo 运行测试：
echo examples\example.exe

endlocal
```

### 清理脚本（clean.bat）

创建 `clean.bat` 文件：

```batch
@echo off
echo 清理构建文件...

if exist "bin\*.dll" del /q bin\*.dll
if exist "bin\*.exe" del /q bin\*.exe
if exist "lib\*.lib" del /q lib\*.lib
if exist "lib\*.a" del /q lib\*.a
if exist "examples\*.exe" del /q examples\*.exe

echo 清理完成！
```

## 🔧 CMake 构建

### CMakeLists.txt

创建 `CMakeLists.txt` 文件：

```cmake
cmake_minimum_required(VERSION 3.10)
project(WinLog VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译定义
add_definitions(-DWINDOWS -DWINGDIAPI=)

# 包含目录
include_directories(include)

# 创建 DLL
add_library(WinLog SHARED src/winlog.cpp)

# 设置 DLL 导出宏
target_compile_definitions(WinLog PRIVATE WINLOG_EXPORTS)

# 设置输出目录
set_target_properties(WinLog PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# 创建示例程序
add_executable(example examples/example.cpp)
target_link_libraries(example WinLog)
set_target_properties(example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)

# 安装规则
install(TARGETS WinLog
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES include/winlog.h DESTINATION include)
```

### 使用 CMake 构建

```cmd
# 创建构建目录
mkdir build
cd build

# 生成构建文件
cmake .. -G "MinGW Makefiles"

# 或者使用 Visual Studio
cmake .. -G "Visual Studio 16 2019" -A x64

# 构建
cmake --build . --config Release

# 安装（可选）
cmake --install . --prefix ../install
```

## ⚠️ 常见问题

### 1. 找不到 g++ 编译器

**错误信息：**
```
'g++' 不是内部或外部命令，也不是可运行的程序
```

**解决方案：**
- 确保 MinGW-w64 已正确安装
- 将 MinGW-w64 的 bin 目录添加到系统 PATH
- 重新打开命令提示符

### 2. 链接错误

**错误信息：**
```
undefined reference to `WinLog::getInstance()'
```

**解决方案：**
- 确保正确链接了 WinLog.lib
- 检查库文件路径是否正确
- 确保 DLL 和 LIB 文件存在

### 3. DLL 找不到

**错误信息：**
```
无法找到 WinLog.dll
```

**解决方案：**
- 将 WinLog.dll 复制到可执行文件同一目录
- 或将包含 DLL 的目录添加到 PATH
- 使用依赖查看工具检查 DLL 依赖

### 4. 运行时错误

**错误信息：**
```
应用程序无法正常启动 (0xc000007b)
```

**解决方案：**
- 确保所有依赖的 DLL 都存在
- 检查运行时库版本是否匹配
- 使用 Dependency Walker 检查依赖关系

### 5. 中文乱码

**解决方案：**
```cpp
// 在程序开始时设置控制台编码
#include <windows.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    // 现在可以正常使用中文日志
    logInfo("中文测试");
    
    return 0;
}
```

## 🧪 测试构建

### 简单测试

```cpp
#include "winlog.h"
#include <iostream>

int main() {
    std::cout << "测试 WinLog 库..." << std::endl;
    
    // 初始化
    if (!WinLog::getInstance().init("test.log", LogLevel::debug)) {
        std::cerr << "初始化失败！" << std::endl;
        return 1;
    }
    
    // 测试各种日志级别
    logTrace("跟踪信息");
    logDebug("调试信息");
    logInfo("信息消息");
    logWarn("警告消息");
    logError("错误消息");
    logCritical("严重错误消息");
    
    // 测试版本信息
    std::cout << "版本: " << getWinLogVersionString() << std::endl;
    
    // 关闭
    WinLog::getInstance().shutdown();
    
    std::cout << "测试完成！" << std::endl;
    return 0;
}
```

### 编译测试程序

```cmd
g++ -o test_build.exe test_build.cpp -I include -L lib -lWinLog -static-libgcc -static-libstdc++
```

## 📦 发布准备

### 发布文件清单

构建完成后，准备以下文件用于发布：

```
发布包/
├── bin/
│   ├── WinLog.dll          # 动态链接库
│   └── LogExample.exe      # 示例程序
├── lib/
│   └── WinLog.lib          # 导入库
├── include/
│   └── winlog.h            # 头文件
├── docs/
│   ├── README.md           # 项目说明
│   ├── API_REFERENCE.md    # API 参考
│   ├── USAGE_GUIDE.md      # 使用指南
│   └── BUILD_INSTRUCTIONS.md # 构建说明
└── examples/
    ├── example.cpp         # 示例源代码
    └── example.exe         # 示例可执行文件
```

### 版本管理

在发布前更新版本信息：

1. 修改 `include/winlog.h` 中的版本宏
2. 更新所有文档中的版本号
3. 创建版本标签（如果使用 Git）

```cmd
git tag -a v1.0.0 -m "版本 1.0.0 发布"
git push origin v1.0.0
```

## 🚀 自动化构建

### GitHub Actions 配置

创建 `.github/workflows/build.yml`：

```yaml
name: Build WinLog

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build-windows:
    runs-on: windows-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup MinGW
      uses: egor-tensin/setup-mingw@v2
      with:
        platform: x64
    
    - name: Build DLL
      run: |
        g++ -shared -o bin/WinLog.dll src/winlog.cpp -I include -DWINDOWS -DWINGDIAPI= -DWINLOG_EXPORTS -static-libgcc -static-libstdc++ -Wl,--out-implib,lib/WinLog.lib
    
    - name: Build Example
      run: |
        g++ -o examples/example.exe examples/example.cpp -I include -L lib -lWinLog -static-libgcc -static-libstdc++
    
    - name: Run Tests
      run: |
        examples\example.exe
    
    - name: Upload Artifacts
      uses: actions/upload-artifact@v3
      with:
        name: winlog-build
        path: |
          bin/
          lib/
          examples/
```

## 📚 相关资源

- [MinGW-w64 官方网站](https://www.mingw-w64.org/)
- [CMake 官方文档](https://cmake.org/documentation/)
- [Windows DLL 编程指南](https://docs.microsoft.com/en-us/windows/win32/dlls/dlls)
- [GCC 编译器文档](https://gcc.gnu.org/onlinedocs/)

---

**💡 提示**：构建过程中遇到问题时，首先检查编译器是否正确安装，然后查看错误信息，通常都能找到解决方案。如果问题持续存在，可以查看相关文档或在社区寻求帮助。