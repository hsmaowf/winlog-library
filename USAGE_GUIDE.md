# WinLog 使用指南

## 📚 目录

1. [快速开始](#快速开始)
2. [基本使用](#基本使用)
3. [高级功能](#高级功能)
4. [最佳实践](#最佳实践)
5. [常见问题](#常见问题)
6. [性能优化](#性能优化)

## 🚀 快速开始

### 步骤 1：包含头文件

```cpp
#include "winlog.h"
```

### 步骤 2：初始化日志库

```cpp
// 初始化，日志写入文件和控制台
WinLog::getInstance().init("myapp.log", LogLevel::info);

// 或者只输出到控制台（不写入文件）
WinLog::getInstance().init(nullptr, LogLevel::debug);
```

### 步骤 3：记录日志

```cpp
// 使用类方法
WinLog::getInstance().info("程序启动成功");

// 使用全局函数
logInfo("程序启动成功");
```

### 步骤 4：关闭日志库

```cpp
WinLog::getInstance().shutdown();
```

## 📝 基本使用

### 日志级别选择

根据应用场景选择合适的日志级别：

```cpp
// 开发环境 - 显示所有日志
WinLog::getInstance().init("dev.log", LogLevel::trace);

// 测试环境 - 显示调试及以上级别
WinLog::getInstance().init("test.log", LogLevel::debug);

// 生产环境 - 只显示警告及以上级别
WinLog::getInstance().init("prod.log", LogLevel::warn);
```

### 格式化日志消息

```cpp
// 字符串参数
const char* user = "张三";
WinLog::getInstance().info("用户 %s 登录成功", user);

// 数值参数
int count = 42;
WinLog::getInstance().debug("处理完成，共处理 %d 个项目", count);

// 多个参数
float temperature = 23.5f;
WinLog::getInstance().info("当前温度: %.1f°C，湿度: %d%%", temperature, 65);

// 错误码
int errorCode = -1;
WinLog::getInstance().error("操作失败，错误码: %d", errorCode);
```

### 中文日志记录

```cpp
// WinLog 完全支持中文
WinLog::getInstance().info("系统初始化完成");
WinLog::getInstance().warn("磁盘空间不足，剩余 %d GB", freeSpace);
WinLog::getInstance().error("数据库连接失败：%s", errorMsg);
```

## 🔧 高级功能

### 动态调整日志级别

```cpp
// 初始设置为 INFO 级别
WinLog::getInstance().init("app.log", LogLevel::info);

// 调试时临时切换到 DEBUG 级别
WinLog::getInstance().setLevel(LogLevel::debug);
logDebug("调试信息：变量 x = %d", x);

// 调试完成后恢复 INFO 级别
WinLog::getInstance().setLevel(LogLevel::info);
```

### 条件日志记录

```cpp
// 只在调试模式下记录详细日志
#ifdef DEBUG_MODE
    logTrace("进入函数：%s", __FUNCTION__);
    logDebug("参数值：%d", param);
#endif

// 根据日志级别记录不同详细程度的信息
if (logLevel <= LogLevel::debug) {
    logDebug("详细调试信息：%s", detailedInfo);
}
```

### 错误处理日志

```cpp
try {
    // 可能抛出异常的代码
    riskyOperation();
} catch (const std::exception& e) {
    logError("捕获异常：%s", e.what());
} catch (...) {
    logCritical("捕获未知异常");
}
```

### 性能监控日志

```cpp
auto start = std::chrono::high_resolution_clock::now();

// 执行操作
performOperation();

auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

logInfo("操作完成，耗时：%lld 毫秒", duration.count());
```

## 🎯 最佳实践

### 1. 日志命名规范

```cpp
// ✅ 好的做法：清晰描述事件
logInfo("用户登录成功 - 用户ID: %d", userId);
logWarn("API调用频率过高 - 接口: %s, 次数: %d", apiName, callCount);

// ❌ 避免：过于简单或模糊
logInfo("OK");
logWarn("Something happened");
```

### 2. 错误日志详细程度

```cpp
// ✅ 好的做法：包含足够的上下文信息
logError("数据库查询失败 - 表: %s, 错误: %s, SQL: %s", 
         tableName, mysql_error(conn), sqlQuery);

// ❌ 避免：信息不足
logError("Database error");
```

### 3. 日志级别选择

```cpp
// TRACE: 详细的执行路径
logTrace("进入函数: processData()");
logTrace("循环开始，迭代次数: %d", iterations);

// DEBUG: 调试信息
logDebug("变量状态 - x: %d, y: %d", x, y);
logDebug("缓存命中率: %.2f%%", hitRate);

// INFO: 重要事件
logInfo("服务启动完成 - 端口: %d", port);
logInfo("用户注册成功 - 邮箱: %s", email);

// WARN: 警告但不影响继续执行
logWarn("配置文件缺失，使用默认值");
logWarn("API响应时间过长: %d ms", responseTime);

// ERROR: 错误但程序可继续
logError("文件读取失败: %s", filename);
logError("网络连接超时 - 主机: %s", hostname);

// CRITICAL: 严重错误，可能导致程序终止
logCritical("数据库连接池耗尽");
logCritical("系统内存不足");
```

### 4. 资源管理

```cpp
// ✅ 好的做法：确保正确关闭日志
class Application {
public:
    bool initialize() {
        if (!WinLog::getInstance().init("app.log", LogLevel::info)) {
            return false;
        }
        logInfo("应用程序初始化成功");
        return true;
    }
    
    void cleanup() {
        logInfo("应用程序关闭中...");
        WinLog::getInstance().shutdown();
    }
    
    ~Application() {
        cleanup();
    }
};
```

## ❓ 常见问题

### Q1: 日志文件无法创建？

**可能原因：**
- 文件路径不存在
- 没有写入权限
- 文件被其他程序占用

**解决方案：**
```cpp
// 检查路径是否存在
std::string logPath = "logs/app.log";
std::filesystem::create_directories(std::filesystem::path(logPath).parent_path());

// 初始化日志
if (!WinLog::getInstance().init(logPath.c_str(), LogLevel::info)) {
    std::cerr << "日志初始化失败" << std::endl;
    return -1;
}
```

### Q2: 中文显示乱码？

**解决方案：**
```cpp
// 设置控制台为 UTF-8 编码
SetConsoleOutputCP(CP_UTF8);
SetConsoleCP(CP_UTF8);

// 现在可以正常显示中文
logInfo("中文日志消息测试");
```

### Q3: 日志文件过大？

**解决方案：**
```cpp
// 实现日志轮转
void rotateLog(const std::string& baseName) {
    static int fileIndex = 0;
    std::string logFile = baseName + "_" + std::to_string(fileIndex) + ".log";
    
    // 关闭当前日志
    WinLog::getInstance().shutdown();
    
    // 切换到新文件
    WinLog::getInstance().init(logFile.c_str(), LogLevel::info);
    
    fileIndex++;
}

// 定期调用日志轮转
if (logFileSize > MAX_LOG_SIZE) {
    rotateLog("application");
}
```

### Q4: 多线程环境下日志顺序混乱？

**解决方案：**
WinLog 内部已经实现了线程安全机制，无需额外处理。每个日志消息都会完整输出，不会出现交叉混乱。

### Q5: 如何过滤敏感信息？

**解决方案：**
```cpp
// 创建日志包装函数
void logUserAction(const std::string& userId, const std::string& action) {
    // 记录用户ID的哈希值而不是明文
    std::size_t hash = std::hash<std::string>{}(userId);
    logInfo("用户操作 - 用户哈希: %zu, 操作: %s", hash, action.c_str());
}

// 使用包装函数
logUserAction("user123", "登录");
```

## ⚡ 性能优化

### 1. 减少不必要的日志调用

```cpp
// ✅ 好的做法：先检查日志级别
if (logLevel <= LogLevel::debug) {
    logDebug("详细调试信息：%s", expensiveOperation());
}

// ❌ 避免：总是调用，即使不会输出
logDebug("详细调试信息：%s", expensiveOperation());
```

### 2. 批量日志记录

```cpp
// 对于大量日志，考虑批量处理
std::vector<std::string> logMessages;

// 收集日志消息
for (const auto& item : items) {
    logMessages.push_back(formatLogMessage(item));
}

// 一次性输出
for (const auto& msg : logMessages) {
    logInfo("%s", msg.c_str());
}
```

### 3. 异步日志（高级）

```cpp
// 使用队列实现异步日志
#include <queue>
#include <thread>
#include <condition_variable>

class AsyncLogger {
private:
    std::queue<std::string> logQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::thread workerThread;
    bool running;
    
public:
    AsyncLogger() : running(true) {
        workerThread = std::thread([this]() {
            while (running) {
                std::unique_lock<std::mutex> lock(queueMutex);
                cv.wait(lock, [this] { return !logQueue.empty() || !running; });
                
                while (!logQueue.empty()) {
                    std::string msg = logQueue.front();
                    logQueue.pop();
                    lock.unlock();
                    
                    logInfo("%s", msg.c_str());
                    
                    lock.lock();
                }
            }
        });
    }
    
    ~AsyncLogger() {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            running = false;
        }
        cv.notify_all();
        workerThread.join();
    }
    
    void logAsync(const std::string& message) {
        std::lock_guard<std::mutex> lock(queueMutex);
        logQueue.push(message);
        cv.notify_one();
    }
};
```

## 📋 日志格式建议

### 标准格式

```
[2024-01-15 14:30:45.123] [INFO] 用户登录成功 - 用户ID: 12345
[2024-01-15 14:30:46.456] [WARN] API调用频率过高 - 接口: /api/users, IP: 192.168.1.100
[2024-01-15 14:30:47.789] [ERROR] 数据库连接失败 - 主机: localhost, 端口: 3306, 错误: Connection refused
```

### 结构化日志（JSON格式）

```cpp
void logStructured(LogLevel level, const std::string& event, 
                  const std::map<std::string, std::string>& data) {
    std::stringstream json;
    json << "{\"event\":\"" << event << "\",";
    json << "\"timestamp\":\"" << getCurrentTimestamp() << "\",";
    json << "\"data\":{";
    
    bool first = true;
    for (const auto& [key, value] : data) {
        if (!first) json << ",";
        json << "\"" << key << "\":\"" << value << "\"";
        first = false;
    }
    
    json << "}}";
    
    switch (level) {
        case LogLevel::info: logInfo("%s", json.str().c_str()); break;
        case LogLevel::warn: logWarn("%s", json.str().c_str()); break;
        case LogLevel::error: logError("%s", json.str().c_str()); break;
        default: break;
    }
}

// 使用示例
logStructured(LogLevel::info, "user_login", {
    {"user_id", "12345"},
    {"ip_address", "192.168.1.100"},
    {"user_agent", "Mozilla/5.0"}
});
```

---

**💡 提示**：良好的日志记录是程序调试和维护的关键。合理使用不同级别的日志，既能帮助开发调试，又能在生产环境中快速定位问题。