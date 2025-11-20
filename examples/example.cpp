#include "winlog.h"
#include <iostream>
#include <windows.h>
#include <chrono>
#include <thread>
#include <cstring>

// 调试辅助函数：显示当前时间戳
void showTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    char buffer[64];
    ctime_s(buffer, sizeof(buffer), &now_c);
    buffer[strlen(buffer) - 1] = '\0'; // 移除换行符
    std::cout << "[DEBUG] [" << buffer << "]" << std::endl;
}

// 设置控制台输出为UTF-8编码
void setConsoleOutputUTF8() {
    std::cout << "[DEBUG] 设置控制台输出为UTF-8编码" << std::endl;
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::cout << "[DEBUG] 控制台编码设置完成" << std::endl;
}

int main() {
    std::cout << "[DEBUG] 程序开始执行" << std::endl;
    showTimestamp();
    
    // 设置控制台输出为UTF-8，确保中文正确显示
    std::cout << "[DEBUG] 准备设置控制台编码" << std::endl;
    setConsoleOutputUTF8();
    
    // 初始化日志库，设置日志文件路径和日志级别
    std::cout << "\n[DEBUG] 准备初始化WinLog库" << std::endl;
    std::cout << "[显示调用] 开始初始化WinLog库..." << std::endl;
    std::cout << "[DEBUG] 日志文件路径: application.log, 日志级别: LogLevel::trace" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().init(\"application.log\", LogLevel::trace)" << std::endl;
    
    bool initResult = WinLog::getInstance().init("application.log", LogLevel::trace);
    std::cout << "[DEBUG] 初始化结果: " << (initResult ? "成功" : "失败") << std::endl;
    
    if (!initResult) {
        std::cerr << "Failed to initialize WinLog!" << std::endl;
        return 1;
    }
    
    std::cout << "WinLog initialized successfully." << std::endl;
    std::cout << "\n[DEBUG] 初始化完成，等待100毫秒..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "\n[显示调用] 开始使用类方法记录不同级别的日志..." << std::endl;
    
    // 使用类方法记录日志
    std::cout << "[DEBUG] 记录TRACE级别日志" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().trace(\"This is a TRACE message with number: %d\", 42)" << std::endl;
    WinLog::getInstance().trace("This is a TRACE message with number: %d", 42);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::cout << "[DEBUG] 记录DEBUG级别日志" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().debug(\"This is a DEBUG message with string: %s\", \"Hello, World!\")" << std::endl;
    WinLog::getInstance().debug("This is a DEBUG message with string: %s", "Hello, World!");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::cout << "[DEBUG] 记录INFO级别日志" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().info(\"This is an INFO message\")" << std::endl;
    WinLog::getInstance().info("This is an INFO message");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::cout << "[DEBUG] 记录WARN级别日志" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().warn(\"This is a WARNING message\")" << std::endl;
    WinLog::getInstance().warn("This is a WARNING message");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::cout << "[DEBUG] 记录ERROR级别日志" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().error(\"This is an ERROR message\")" << std::endl;
    WinLog::getInstance().error("This is an ERROR message");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::cout << "[DEBUG] 记录CRITICAL级别日志" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().critical(\"This is a CRITICAL message\")" << std::endl;
    WinLog::getInstance().critical("This is a CRITICAL message");
    
    std::cout << "\n[DEBUG] 类方法日志记录完成，等待200毫秒..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "\n[显示调用] 开始使用全局函数记录日志..." << std::endl;
    
    // 使用全局函数记录日志
    std::cout << "[DEBUG] 使用全局函数记录TRACE日志" << std::endl;
    std::cout << "[显示调用] 调用: logTrace(\"Global TRACE message\")" << std::endl;
    logTrace("Global TRACE message");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::cout << "[DEBUG] 使用全局函数记录DEBUG日志" << std::endl;
    std::cout << "[显示调用] 调用: logDebug(\"Global DEBUG message\")" << std::endl;
    logDebug("Global DEBUG message");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::cout << "[DEBUG] 使用全局函数记录INFO日志" << std::endl;
    std::cout << "[显示调用] 调用: logInfo(\"Global INFO message\")" << std::endl;
    logInfo("Global INFO message");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::cout << "[DEBUG] 使用全局函数记录WARN日志" << std::endl;
    std::cout << "[显示调用] 调用: logWarn(\"Global WARNING message\")" << std::endl;
    logWarn("Global WARNING message");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::cout << "[DEBUG] 使用全局函数记录ERROR日志" << std::endl;
    std::cout << "[显示调用] 调用: logError(\"Global ERROR message\")" << std::endl;
    logError("Global ERROR message");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::cout << "[DEBUG] 使用全局函数记录CRITICAL日志" << std::endl;
    std::cout << "[显示调用] 调用: logCritical(\"Global CRITICAL message\")" << std::endl;
    logCritical("Global CRITICAL message");
    
    std::cout << "\n[DEBUG] 全局函数日志记录完成，等待200毫秒..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // 更改日志级别
    std::cout << "\n[DEBUG] 准备更改日志级别" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().setLevel(LogLevel::warn)" << std::endl;
    WinLog::getInstance().setLevel(LogLevel::warn);
    std::cout << "[DEBUG] 日志级别已更改为: LogLevel::warn" << std::endl;
    
    std::cout << "\n[DEBUG] 等待100毫秒..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "\n[显示调用] 开始测试不同日志级别..." << std::endl;
    std::cout << "[DEBUG] 当前日志级别为warn，以下低级别日志(trace, debug, info)将不会被记录" << std::endl;
    
    // 这些日志将不会被记录，因为它们的级别低于当前设置的级别
    std::cout << "[DEBUG] 尝试记录TRACE级别日志（不应显示）" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().trace(\"This TRACE message will not be shown\")" << std::endl;
    WinLog::getInstance().trace("This TRACE message will not be shown");
    
    std::cout << "[DEBUG] 尝试记录DEBUG级别日志（不应显示）" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().debug(\"This DEBUG message will not be shown\")" << std::endl;
    WinLog::getInstance().debug("This DEBUG message will not be shown");
    
    std::cout << "[DEBUG] 尝试记录INFO级别日志（不应显示）" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().info(\"This INFO message will not be shown\")" << std::endl;
    WinLog::getInstance().info("This INFO message will not be shown");
    
    std::cout << "[DEBUG] 以下日志级别(warn, error, critical)应正常显示" << std::endl;
    
    // 这些日志将会被记录
    std::cout << "[DEBUG] 记录WARN级别日志（应显示）" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().warn(\"This WARNING message will be shown\")" << std::endl;
    WinLog::getInstance().warn("This WARNING message will be shown");
    
    std::cout << "[DEBUG] 记录ERROR级别日志（应显示）" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().error(\"This ERROR message will be shown\")" << std::endl;
    WinLog::getInstance().error("This ERROR message will be shown");
    
    std::cout << "[DEBUG] 记录CRITICAL级别日志（应显示）" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().critical(\"This CRITICAL message will be shown\")" << std::endl;
    WinLog::getInstance().critical("This CRITICAL message will be shown");
    
    std::cout << "\n[DEBUG] 日志级别测试完成，等待200毫秒..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // 演示版本管理接口
    std::cout << "\n[显示调用] 开始演示版本管理接口..." << std::endl;
    
    // 使用类方法获取版本信息
    std::cout << "\n[DEBUG] 使用WinLog类的静态方法获取版本信息" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getVersionMajor()" << std::endl;
    int major = WinLog::getVersionMajor();
    std::cout << "[结果] 主版本号: " << major << std::endl;
    
    std::cout << "[显示调用] 调用: WinLog::getVersionMinor()" << std::endl;
    int minor = WinLog::getVersionMinor();
    std::cout << "[结果] 次版本号: " << minor << std::endl;
    
    std::cout << "[显示调用] 调用: WinLog::getVersionPatch()" << std::endl;
    int patch = WinLog::getVersionPatch();
    std::cout << "[结果] 修订号: " << patch << std::endl;
    
    std::cout << "[显示调用] 调用: WinLog::getVersionBuild()" << std::endl;
    int build = WinLog::getVersionBuild();
    std::cout << "[结果] 构建号: " << build << std::endl;
    
    std::cout << "[显示调用] 调用: WinLog::getVersionString()" << std::endl;
    const char* versionStr = WinLog::getVersionString();
    std::cout << "[结果] 版本字符串: " << versionStr << std::endl;
    
    std::cout << "[显示调用] 调用: WinLog::getVersionNumber()" << std::endl;
    unsigned int versionNum = WinLog::getVersionNumber();
    std::cout << "[结果] 版本数字: " << versionNum << std::endl;
    
    // 使用全局函数获取版本信息
    std::cout << "\n[DEBUG] 使用全局函数获取版本信息" << std::endl;
    std::cout << "[显示调用] 调用: getWinLogVersionString()" << std::endl;
    const char* globalVersionStr = getWinLogVersionString();
    std::cout << "[结果] 全局版本字符串: " << globalVersionStr << std::endl;
    
    // 记录版本信息到日志
    std::cout << "\n[DEBUG] 记录版本信息到日志" << std::endl;
    std::cout << "[显示调用] 调用: logInfo(\"WinLog Version: %s\", getWinLogVersionString())" << std::endl;
    logInfo("WinLog Version: %s", getWinLogVersionString());
    
    std::cout << "\n[DEBUG] 版本管理接口演示完成，等待200毫秒..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // 关闭日志库
    std::cout << "\n[DEBUG] 准备关闭日志库" << std::endl;
    std::cout << "[显示调用] 调用: WinLog::getInstance().shutdown()" << std::endl;
    WinLog::getInstance().shutdown();
    std::cout << "[DEBUG] 日志库已关闭" << std::endl;
    
    std::cout << "\nWinLog shutdown. Example completed." << std::endl;
    showTimestamp();
    
    return 0;
}