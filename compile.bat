@echo off
echo 开始编译 WinLog 项目...

REM 编译 DLL
echo 编译 WinLog.dll...
g++ -shared -o bin/WinLog.dll src/winlog.cpp src/async_log_queue.cpp -I include -DWINDOWS -DWINGDIAPI= -DWINLOG_EXPORTS -static-libgcc -static-libstdc++ -Wl,--out-implib,lib/WinLog.lib
if %errorlevel% neq 0 (
    echo DLL 编译失败！
    exit /b 1
)
echo DLL 编译成功！

REM 编译示例程序
echo 编译示例程序...
g++ -o examples/example.exe examples/example.cpp -I include -L lib -lWinLog -static-libgcc -static-libstdc++
if %errorlevel% neq 0 (
    echo 示例程序编译失败！
    exit /b 1
)
echo 示例程序编译成功！

echo 编译异步日志示例程序...
g++ -o examples/async_log_example.exe examples/async_log_example.cpp -I include -L lib -lWinLog -static-libgcc -static-libstdc++
if %errorlevel% neq 0 (
    echo 异步日志示例程序编译失败！
    exit /b 1
)
echo 异步日志示例程序编译成功！

echo 编译测试程序...
g++ -o test/async_log_test.exe test/async_log_test.cpp -I include -L lib -lWinLog -static-libgcc -static-libstdc++
if %errorlevel% neq 0 (
    echo 测试程序编译失败！
    exit /b 1
)
echo 测试程序编译成功！

echo 所有编译完成！