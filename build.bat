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
g++ -shared -o bin/WinLog.dll src/winlog.cpp src/async_log_queue.cpp -I include -DWINDOWS -DWINGDIAPI= -DWINLOG_EXPORTS -static-libgcc -static-libstdc++ -Wl,--out-implib,lib/WinLog.lib
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

echo 3. 编译异步日志示例程序...
g++ -o examples/async_log_example.exe examples/async_log_example.cpp -I include -L lib -lWinLog -static-libgcc -static-libstdc++
if %errorlevel% neq 0 (
    echo 错误：异步日志示例程序编译失败
    exit /b 1
)
echo ✓ 异步日志示例程序编译成功

echo 4. 编译测试程序...
g++ -o test/async_log_test.exe test/async_log_test.cpp -I include -L lib -lWinLog -static-libgcc -static-libstdc++
if %errorlevel% neq 0 (
    echo 错误：测试程序编译失败
    exit /b 1
)
echo ✓ 测试程序编译成功

echo.
echo 构建完成！
echo 生成文件：
dir /b bin\*.dll
lib\*.lib
examples\*.exe
test\*.exe
echo.
echo 运行测试：
echo examples\example.exe
echo examples\async_log_example.exe
echo test\async_log_test.exe

endlocal