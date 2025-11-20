# WinLog API 参考文档

## 📖 概述

WinLog 提供两套 API 接口：C++ 类接口和 C 风格全局函数接口。所有接口都支持线程安全操作。

## 🏗️ C++ 类接口

### WinLog 类

WinLog 类采用单例模式设计，确保全局只有一个日志实例。

#### 获取实例
```cpp
static WinLog& WinLog::getInstance();
```

获取 WinLog 单例实例，所有操作都通过此实例进行。

#### 初始化

**基本初始化（同步模式）**
```cpp
bool init(const char* logFilePath = nullptr, LogLevel level = LogLevel::info);
```

**异步模式初始化**
```cpp
bool init(const char* logFilePath, LogLevel level, const AsyncConfig& asyncConfig);
```

初始化日志库。

**参数：**
- `logFilePath`：日志文件路径，如果为 `nullptr` 则不写入文件
- `level`：日志级别，默认为 `LogLevel::info`
- `asyncConfig`：异步日志配置结构体

**返回值：**
- `true`：初始化成功
- `false`：初始化失败

**示例：**
```cpp
// 基本同步模式初始化
WinLog::getInstance().init("app.log", LogLevel::debug);

// 异步模式初始化
AsyncConfig config;
config.enabled = true;
config.queueSize = 5000;
config.flushIntervalMs = 500;
WinLog::getInstance().init("app.log", LogLevel::info, config);
```

#### 日志记录方法

```cpp
void trace(const char* format, ...);
void debug(const char* format, ...);
void info(const char* format, ...);
void warn(const char* format, ...);
void error(const char* format, ...);
void critical(const char* format, ...);
```

**参数：**
- `format`：格式化字符串（支持 printf 风格）
- `...`：可变参数

**示例：**
```cpp
WinLog::getInstance().info("程序启动成功");
WinLog::getInstance().debug("变量值: %d", value);
WinLog::getInstance().error("发生错误: %s", errorMsg);
```

#### 设置日志级别
```cpp
void setLevel(LogLevel level);
```

设置当前日志级别，低于此级别的日志将不会被记录。

**参数：**
- `level`：新的日志级别

**示例：**
```cpp
WinLog::getInstance().setLevel(LogLevel::warn);
```

#### 关闭日志库
```cpp
void shutdown();
```

关闭日志库，释放资源。

**示例：**
```cpp
WinLog::getInstance().shutdown();
```

#### 刷新日志缓冲区
```cpp
bool flush(int timeoutMs = -1);
```

立即写入所有待处理的日志到文件。在异步模式下特别有用。

**参数：**
- `timeoutMs`：超时时间（毫秒），-1 表示无限等待

**返回值：**
- `true`：刷新成功
- `false`：刷新失败或超时

**示例：**
```cpp
WinLog::getInstance().flush(2000); // 等待最多2000毫秒刷新完成
```

#### 性能统计接口

##### 重置统计数据
```cpp
void resetStats();
```

重置所有性能统计计数器。

**示例：**
```cpp
// 开始新的性能统计周期
WinLog::getInstance().resetStats();
```

##### 获取统计数据
```cpp
struct Stats {
    uint64_t allocationRequests;      // 内存分配请求次数
    uint64_t deallocationRequests;    // 内存释放请求次数
    uint64_t objectReuseCount;        // 对象复用次数
    uint64_t currentPoolSize;         // 当前内存池对象数量
    double memorySavingsPercent;      // 内存节省百分比
    uint64_t totalMemoryAllocated;    // 总共分配的内存（字节）
    uint64_t peakMemoryUsage;         // 峰值内存使用量（字节）
    uint64_t queueOverflowCount;      // 队列溢出次数
    uint64_t droppedMessages;         // 丢弃的消息数量
    uint64_t processedMessages;       // 已处理的消息数量
};

Stats getStats() const;
```

获取当前的性能统计数据。

**返回值：**
- 包含各种性能指标的 Stats 结构体

**示例：**
```cpp
WinLog::Stats stats = WinLog::getInstance().getStats();
printf("对象复用次数: %llu\n", stats.objectReuseCount);
printf("内存节省百分比: %.2f%%\n", stats.memorySavingsPercent);
```

#### 设置异步配置
```cpp
void setAsyncConfig(const AsyncConfig& config);
```

设置异步日志的配置参数。注意：必须在 `init()` 之前调用。

**参数：**
- `config`：异步日志配置结构体

**示例：**
```cpp
AsyncConfig config;
config.queueSize = 10000;
config.flushIntervalMs = 1000;
WinLog::getInstance().setAsyncConfig(config);
WinLog::getInstance().init("app.log", LogLevel::info);
```

#### 获取当前异步配置
```cpp
AsyncConfig getAsyncConfig() const;
```

获取当前的异步日志配置。

**返回值：**
- 当前的异步日志配置结构体

#### 判断是否使用异步模式
```cpp
bool isAsyncModeEnabled() const;
```

判断当前是否启用了异步日志模式。

**返回值：**
- `true`：已启用异步模式
- `false`：使用同步模式

#### 版本管理接口

```cpp
static int getVersionMajor();           // 获取主版本号
static int getVersionMinor();           // 获取次版本号
static int getVersionPatch();           // 获取修订号
static int getVersionBuild();           // 获取构建号
static const char* getVersionString();  // 获取版本字符串
static unsigned int getVersionNumber(); // 获取版本数字
```

**示例：**
```cpp
int major = WinLog::getVersionMajor();
const char* version = WinLog::getVersionString();
```

## 🌍 C 风格全局函数接口

### 日志记录函数

```cpp
extern "C" {
    void logTrace(const char* format, ...);
    void logDebug(const char* format, ...);
    void logInfo(const char* format, ...);
    void logWarn(const char* format, ...);
    void logError(const char* format, ...);
    void logCritical(const char* format, ...);
}
```

这些函数提供了与 C++ 类方法相同的功能，但使用更简洁的调用方式。

**示例：**
```cpp
logInfo("程序启动成功");
logDebug("变量值: %d", value);
logError("发生错误: %s", errorMsg);
```

### 版本管理函数

```cpp
extern "C" {
    int getWinLogVersionMajor();
    int getWinLogVersionMinor();
    int getWinLogVersionPatch();
    int getWinLogVersionBuild();
    const char* getWinLogVersionString();
    unsigned int getWinLogVersionNumber();
}
```

**示例：**
```cpp
const char* version = getWinLogVersionString();
int major = getWinLogVersionMajor();
```

## 📊 日志级别枚举

```cpp
enum class LogLevel {
    trace = 0,    // 最详细的跟踪信息
    debug = 1,    // 调试信息
    info = 2,     // 一般信息
    warn = 3,     // 警告信息
    error = 4,    // 错误信息
    critical = 5, // 严重错误信息
    off = 6       // 关闭所有日志
};
```

## ⚙️ 版本信息宏

```cpp
#define WINLOG_VERSION_MAJOR    1
#define WINLOG_VERSION_MINOR    0
#define WINLOG_VERSION_PATCH    0
#define WINLOG_VERSION_BUILD    1
#define WINLOG_VERSION_STRING   "1.0.0.1"
#define WINLOG_VERSION_NUMBER   0x01000001
```

## 🔄 线程安全

所有 WinLog 的 API 接口都是线程安全的，内部使用互斥锁保护共享资源。在多线程环境中，可以安全地从不同线程调用日志记录函数。

## 📁 输出格式

日志输出格式为：
```
[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] 消息内容
```

其中：
- `YYYY-MM-DD HH:MM:SS.mmm`：精确到毫秒的当前时间
- `LEVEL`：日志级别（TRACE、DEBUG、INFO、WARN、ERROR、CRITICAL）
- `消息内容`：用户提供的格式化消息

## 💡 使用建议

1. **初始化时机**：在程序开始时尽早初始化日志库
2. **日志级别**：开发环境使用 `DEBUG` 或 `TRACE`，生产环境使用 `INFO` 或 `WARN`
3. **文件路径**：使用绝对路径或相对于程序运行目录的路径
4. **资源释放**：程序结束前调用 `shutdown()` 释放资源
5. **格式化字符串**：使用 printf 风格的格式化字符串，确保参数类型匹配

## ⚠️ 注意事项

1. 日志库必须先初始化才能使用
2. 日志文件路径必须有效，否则初始化会失败
3. 格式化字符串中的格式说明符必须与参数类型匹配
4. 在高并发场景下，频繁的文件操作可能影响性能
5. 确保程序有足够的权限写入指定的日志文件

## 📋 错误处理

- 初始化失败时，所有日志函数都不会产生任何输出
- 文件写入失败时，日志仍会输出到控制台（如果级别允许）
- 格式化错误可能导致未定义行为，请确保格式字符串正确