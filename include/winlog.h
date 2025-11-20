#ifndef WINLOG_H
#define WINLOG_H

#include <chrono>
#include <string>
#include <cstdarg>

// Windows DLL导出宏定义
#ifdef WINLOG_EXPORTS
#define WINLOG_API __declspec(dllexport)
#else
#define WINLOG_API __declspec(dllimport)
#endif

// 版本号宏定义（语义化版本号：Major.Minor.Patch.Build）
#define WINLOG_VERSION_MAJOR    1
#define WINLOG_VERSION_MINOR    0
#define WINLOG_VERSION_PATCH    0
#define WINLOG_VERSION_BUILD    1

// 组合版本号宏
#define WINLOG_VERSION_STRING   "1.0.0.1"
#define WINLOG_VERSION_NUMBER   ((WINLOG_VERSION_MAJOR << 24) | (WINLOG_VERSION_MINOR << 16) | (WINLOG_VERSION_PATCH << 8) | WINLOG_VERSION_BUILD)

// 日志级别枚举
enum class LogLevel {
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    error = 4,
    critical = 5,
    off = 6
};

// 预定义的缓冲区大小
#define LOG_MESSAGE_BUFFER_SIZE 512
#define LOG_FILE_BUFFER_SIZE 256

// 日志条目结构 - 使用预分配缓冲区减少内存分配
struct WINLOG_API LogEntry {
    LogLevel level;                                  // 日志级别
    char time[32];                                   // 预分配的时间戳缓冲区
    char message[LOG_MESSAGE_BUFFER_SIZE];           // 预分配的消息缓冲区
    char file[LOG_FILE_BUFFER_SIZE];                 // 预分配的文件名缓冲区
    int line;                                        // 行号
    size_t messageLen;                               // 实际消息长度
    size_t fileLen;                                  // 实际文件名长度
    size_t timeLen;                                  // 实际时间戳长度
    
    LogEntry();
    LogEntry(LogLevel level, const std::string& message);
    LogEntry(LogEntry&& other) noexcept;
    LogEntry& operator=(LogEntry&& other) noexcept;
    
    // 禁用拷贝操作
    LogEntry(const LogEntry&) = delete;
    LogEntry& operator=(const LogEntry&) = delete;
    
    void reset();
    
    // 安全地设置消息，防止缓冲区溢出
    void setMessage(const char* msg, size_t len);
    
    // 安全地设置文件名
    void setFile(const char* filename, size_t len);
    
    // 安全地设置时间戳
    void setTime(const char* timestamp, size_t len);
    
    // 获取字符串形式的消息
    std::string getMessage() const {
        return std::string(message, messageLen);
    }
    
    // 获取字符串形式的文件名
    std::string getFile() const {
        return std::string(file, fileLen);
    }
    
    // 获取字符串形式的时间戳
    std::string getTime() const {
        return std::string(time, timeLen);
    }
};

// 异步配置结构体
struct WINLOG_API AsyncConfig {
    bool enabled;                 // 是否启用异步模式
    size_t queueSize;             // 异步队列大小
    int flushIntervalMs;          // 自动刷新间隔(毫秒)
    size_t maxBatchSize;          // 最大批量处理大小
    size_t memoryPoolSize;        // 内存池大小
    bool dropOnOverflow;          // 队列满时是否丢弃日志
    bool useMemoryPool;           // 是否使用内存池
    bool optimizeForThroughput;   // 是否优化吞吐量
    
    // 默认构造函数
    AsyncConfig() : 
        enabled(true),
        queueSize(10000),
        flushIntervalMs(1000),
        maxBatchSize(100),
        memoryPoolSize(1000),
        dropOnOverflow(false),
        useMemoryPool(true),
        optimizeForThroughput(false) {}
};

// 日志库的主要接口类
class WINLOG_API WinLog {
public:
    // 获取单例实例
    static WinLog& getInstance();
    
    // 初始化日志库（默认同步模式）
    bool init(const char* logFilePath = nullptr, LogLevel level = LogLevel::info);
    
    // 初始化日志库（支持异步配置）
    bool init(const char* logFilePath, LogLevel level, const AsyncConfig& asyncConfig);
    
    // 日志输出方法
    void trace(const char* format, ...);
    void debug(const char* format, ...);
    void info(const char* format, ...);
    void warn(const char* format, ...);
    void error(const char* format, ...);
    void critical(const char* format, ...);
    
    // 设置日志级别
    void setLevel(LogLevel level);
    
    // 关闭日志库
    void shutdown();
    
    // 刷新日志缓冲区（立即写入所有待处理日志）
    bool flush(int timeoutMs = -1);
    
    // 设置异步配置（必须在init之前调用）
    void setAsyncConfig(const AsyncConfig& config);
    
    // 获取当前异步配置
    AsyncConfig getAsyncConfig() const;
    
    // 判断是否使用异步模式
    bool isAsyncModeEnabled() const;
    
    // 版本管理接口
    static int getVersionMajor();
    static int getVersionMinor();
    static int getVersionPatch();
    static int getVersionBuild();
    static const char* getVersionString();
    static unsigned int getVersionNumber();
    
private:
    // 私有构造函数，实现单例模式
    WinLog();
    ~WinLog();
    
    // 禁止拷贝构造和赋值操作
    WinLog(const WinLog&) = delete;
    WinLog& operator=(const WinLog&) = delete;
    
    // 内部实现类
    class Impl;
    Impl* pImpl;
    
    // 异步配置
    AsyncConfig asyncConfig;
};

// 便捷的全局日志函数
extern "C" {
    WINLOG_API void logTrace(const char* format, ...);
    WINLOG_API void logDebug(const char* format, ...);
    WINLOG_API void logInfo(const char* format, ...);
    WINLOG_API void logWarn(const char* format, ...);
    WINLOG_API void logError(const char* format, ...);
    WINLOG_API void logCritical(const char* format, ...);

    // 版本管理全局接口
    WINLOG_API int getWinLogVersionMajor();
    WINLOG_API int getWinLogVersionMinor();
    WINLOG_API int getWinLogVersionPatch();
    WINLOG_API int getWinLogVersionBuild();
    WINLOG_API const char* getWinLogVersionString();
    WINLOG_API unsigned int getWinLogVersionNumber();
}

#endif // WINLOG_H