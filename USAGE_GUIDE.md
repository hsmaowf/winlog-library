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
// 基本初始化，只输出到控制台
WinLog::getInstance().init();

// 输出到文件和控制台
WinLog::getInstance().init("application.log");

// 设置日志级别为DEBUG
WinLog::getInstance().init("application.log", LogLevel::debug);

// 异步日志初始化
AsyncConfig config;
config.enabled = true;
config.queueSize = 5000;
config.flushIntervalMs = 500;
WinLog::getInstance().init("application.log", LogLevel::info, config);

// 或者先设置配置，再初始化
AsyncConfig config2;
config2.enabled = true;
config2.queueSize = 10000;
config2.flushIntervalMs = 1000;
WinLog::getInstance().setAsyncConfig(config2);
WinLog::getInstance().init("application.log", LogLevel::info);
```

### 步骤 3：记录日志

```cpp
// 使用类方法
WinLog::getInstance().info("程序启动成功");
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
WinLog::getInstance().setLogLevel(LogLevel::debug);
WinLog::getInstance().debug("调试信息：变量 x = %d", x);

// 获取当前日志级别
LogLevel currentLevel = WinLog::getInstance().getLogLevel();

// 调试完成后恢复 INFO 级别
WinLog::getInstance().setLogLevel(LogLevel::info);
```

### 条件日志记录

```cpp
// 只在调试模式下记录详细日志
#ifdef DEBUG_MODE
    WinLog::getInstance().trace("进入函数：%s", __FUNCTION__);
    WinLog::getInstance().debug("参数值：%d", param);
#endif

// 根据日志级别记录不同详细程度的信息
if (logLevel <= LogLevel::debug) {
    WinLog::getInstance().debug("详细调试信息：%s", detailedInfo);
}
```

### 错误处理日志

```cpp
try {
    // 可能抛出异常的代码
    riskyOperation();
} catch (const std::exception& e) {
    WinLog::getInstance().error("捕获异常：%s", e.what());
} catch (...) {
    WinLog::getInstance().critical("捕获未知异常");
}
```

### 性能监控日志

```cpp
auto start = std::chrono::high_resolution_clock::now();

// 执行操作
performOperation();

auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

WinLog::getInstance().info("操作完成，耗时：%lld 毫秒", duration.count());
```

## 🎯 最佳实践

### 1. 日志命名规范

```cpp
// ✅ 好的做法：清晰描述事件
WinLog::getInstance().info("用户登录成功 - 用户ID: %d", userId);
WinLog::getInstance().warn("API调用频率过高 - 接口: %s, 次数: %d", apiName, callCount);

// ❌ 避免：过于简单或模糊
WinLog::getInstance().info("OK");
WinLog::getInstance().warn("Something happened");
```

### 2. 错误日志详细程度

```cpp
// ✅ 好的做法：包含足够的上下文信息
WinLog::getInstance().error("数据库查询失败 - 表: %s, 错误: %s, SQL: %s", 
         tableName, mysql_error(conn), sqlQuery);

// ❌ 避免：信息不足
WinLog::getInstance().error("Database error");
```

### 3. 日志级别选择

```cpp
// TRACE: 详细的执行路径
WinLog::getInstance().trace("进入函数: processData()");
WinLog::getInstance().trace("循环开始，迭代次数: %d", iterations);

// DEBUG: 调试信息
WinLog::getInstance().debug("变量状态 - x: %d, y: %d", x, y);
WinLog::getInstance().debug("缓存命中率: %.2f%%", hitRate);

// INFO: 重要事件
WinLog::getInstance().info("服务启动完成 - 端口: %d", port);
WinLog::getInstance().info("用户注册成功 - 邮箱: %s", email);

// WARN: 警告但不影响继续执行
WinLog::getInstance().warn("配置文件缺失，使用默认值");
WinLog::getInstance().warn("API响应时间过长: %d ms", responseTime);

// ERROR: 错误但程序可继续
WinLog::getInstance().error("文件读取失败: %s", filename);
WinLog::getInstance().error("网络连接超时 - 主机: %s", hostname);

// CRITICAL: 严重错误，可能导致程序终止
WinLog::getInstance().critical("数据库连接池耗尽");
WinLog::getInstance().critical("系统内存不足");
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
        WinLog::getInstance().info("应用程序初始化成功");
        return true;
    }
    
    void cleanup() {
        WinLog::getInstance().info("应用程序关闭中...");
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
WinLog::getInstance().info("中文日志消息测试");
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
    WinLog::getInstance().info("用户操作 - 用户哈希: %zu, 操作: %s", hash, action.c_str());
}

// 使用包装函数
logUserAction("user123", "登录");
```

## ⚡ 性能优化

### 1. 减少不必要的日志调用

```cpp
// ✅ 好的做法：先检查日志级别
if (WinLog::getInstance().getLogLevel() <= LogLevel::debug) {
    // 只有在调试级别时才构建复杂的日志消息
    std::string complexMessage = "复杂的调试信息：" + someVariable;
    WinLog::getInstance().debug(complexMessage);
}

// ❌ 避免：总是调用，即使不会输出
WinLog::getInstance().debug("详细调试信息：%s", expensiveOperation());
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
        WinLog::getInstance().info("%s", msg.c_str());
    }
```

### 3. 异步日志（高级）

```cpp
// 使用WinLog内置的异步日志功能
AsyncConfig config;
config.enabled = true;
config.queueSize = 10000;  // 较大的队列大小
config.flushIntervalMs = 1000;  // 适当的刷新间隔
WinLog::getInstance().init("high_freq.log", LogLevel::info, config);

// 检查是否使用异步模式
if (WinLog::getInstance().isAsyncModeEnabled()) {
    WinLog::getInstance().info("当前使用异步日志模式");
}

// 获取异步配置
AsyncConfig currentConfig = WinLog::getInstance().getAsyncConfig();
WinLog::getInstance().info("异步队列大小: %d", currentConfig.queueSize);

// 立即刷新日志缓冲区（在程序退出前很有用）
WinLog::getInstance().flush(2000); // 等待最多2000毫秒

// 注意：使用异步日志时，在程序退出前最好调用flush确保所有日志都写入
// 在程序退出前调用
WinLog::getInstance().flush(); // 无限等待直到所有日志写入完成
WinLog::getInstance().shutdown(); // 关闭日志库
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
        case LogLevel::info: WinLog::getInstance().info("%s", json.str().c_str()); break;
        case LogLevel::warn: WinLog::getInstance().warn("%s", json.str().c_str()); break;
        case LogLevel::error: WinLog::getInstance().error("%s", json.str().c_str()); break;
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