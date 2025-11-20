#include "winlog.h"
#include "async_log_queue.h"
#include <Windows.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <cstdarg>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <memory>
#include <cstring>

// 全局WinLog互斥锁定义
std::mutex globalWinLogMutex;





// 已在编译命令中定义WINLOG_EXPORTS，不需要在这里再次定义

// LogEntry 默认构造函数实现
LogEntry::LogEntry() : level(LogLevel::info), line(0), messageLen(0), fileLen(0), timeLen(0) {
    memset(this->message, 0, LOG_MESSAGE_BUFFER_SIZE);
    memset(this->file, 0, LOG_FILE_BUFFER_SIZE);
    memset(this->time, 0, 32);
}

// LogEntry 带参构造函数实现
LogEntry::LogEntry(LogLevel level, const std::string& message) :
    level(level), line(0), messageLen(0), fileLen(0), timeLen(0) {
    memset(this->message, 0, LOG_MESSAGE_BUFFER_SIZE);
    memset(this->file, 0, LOG_FILE_BUFFER_SIZE);
    memset(this->time, 0, 32);
    
    // 设置消息
    if (!message.empty()) {
        setMessage(message.c_str(), message.length());
    }
}

// LogEntry 移动构造函数实现
LogEntry::LogEntry(LogEntry&& other) noexcept :
    level(other.level),
    line(other.line),
    messageLen(other.messageLen),
    fileLen(other.fileLen),
    timeLen(other.timeLen) {
    // 复制消息内容
    if (messageLen > 0) {
        memcpy(this->message, other.message, messageLen + 1);
    }
    
    // 复制文件名
    if (fileLen > 0) {
        memcpy(this->file, other.file, fileLen + 1);
    }
    
    // 复制时间
    if (timeLen > 0) {
        memcpy(this->time, other.time, timeLen + 1);
    }
    
    // 重置源对象
    other.level = LogLevel::info;
    other.line = 0;
    other.messageLen = 0;
    other.fileLen = 0;
    other.timeLen = 0;
    other.message[0] = '\0';
    other.file[0] = '\0';
    other.time[0] = '\0';
}

// 重置对象状态
void LogEntry::reset() {
    level = LogLevel::info;
    line = 0;
    messageLen = 0;
    fileLen = 0;
    timeLen = 0;
    message[0] = '\0';
    file[0] = '\0';
    time[0] = '\0';
}

// 安全地设置消息，防止缓冲区溢出
void LogEntry::setMessage(const char* msg, size_t len) {
    if (msg && len > 0) {
        size_t copyLen = std::min(len, (size_t)LOG_MESSAGE_BUFFER_SIZE - 1);
        memcpy(message, msg, copyLen);
        message[copyLen] = '\0';
        messageLen = copyLen;
    }
}

// 安全地设置文件名
void LogEntry::setFile(const char* filename, size_t len) {
    if (filename && len > 0) {
        size_t copyLen = std::min(len, (size_t)LOG_FILE_BUFFER_SIZE - 1);
        memcpy(file, filename, copyLen);
        file[copyLen] = '\0';
        fileLen = copyLen;
    }
}

// 安全地设置时间戳
void LogEntry::setTime(const char* timestamp, size_t len) {
    if (timestamp && len > 0) {
        size_t copyLen = std::min(len, (size_t)31);
        memcpy(time, timestamp, copyLen);
        time[copyLen] = '\0';
        timeLen = copyLen;
    }
}

// 内部实现类
class WinLog::Impl {
public:
    Impl() : 
        logLevel(LogLevel::info), 
        fileStream(nullptr), 
        isInit(false),
        asyncMode(false),
        asyncQueue(nullptr) {}
    
    ~Impl() {
        shutdown();
    }
    
    bool init(const char* logFilePath, LogLevel level) {
        std::lock_guard<std::mutex> lock(logMutex);
        
        logLevel = level;
        asyncMode = false;
        
        // 初始化文件流
        if (logFilePath) {
            fileStream = new std::ofstream(logFilePath, std::ios::out | std::ios::app);
            if (!fileStream->is_open()) {
                delete fileStream;
                fileStream = nullptr;
                return false;
            }
        }
        
        isInit = true;
        return true;
    }
    
    bool init(const char* logFilePath, LogLevel level, const AsyncConfig& asyncConfig) {
        std::lock_guard<std::mutex> lock(logMutex);
        
        logLevel = level;
        asyncMode = asyncConfig.enabled;
        
        // 初始化文件流
        if (logFilePath) {
            fileStream = new std::ofstream(logFilePath, std::ios::out | std::ios::app);
            if (!fileStream->is_open()) {
                delete fileStream;
                fileStream = nullptr;
                return false;
            }
        }
        
        // 初始化异步队列
        if (asyncMode) {
            asyncQueue = new AsyncLogQueue(
                asyncConfig.queueSize,
                asyncConfig.maxBatchSize,
                asyncConfig.memoryPoolSize,
                asyncConfig.dropOnOverflow,
                asyncConfig.flushIntervalMs
            );
            
            // 设置日志处理回调
            auto self = this;
            asyncQueue->setLogHandler([self](const std::vector<LogEntry>& entries) {
                self->processLogEntries(entries);
            });
        }
        
        isInit = true;
        return true;
    }
    
    void log(LogLevel level, const char* format, va_list args) {
        if (!isInit || level < logLevel || level >= LogLevel::off) {
            return;
        }
        
        // 格式化日志消息
        char message[4096];
        vsnprintf(message, sizeof(message), format, args);
        
        LogEntry entry;
        entry.level = level;
        entry.setMessage(message, strlen(message));
        
        if (asyncMode && asyncQueue) {
            // 异步模式：放入队列
            asyncQueue->enqueue(std::move(entry));
        } else {
            // 同步模式：直接输出
            std::lock_guard<std::mutex> lock(logMutex);
            writeLogToOutputs(entry);
        }
    }
    
    void setLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(logMutex);
        logLevel = level;
    }
    
    void shutdown() {
        std::lock_guard<std::mutex> lock(logMutex);
        
        // 关闭异步队列
        if (asyncQueue) {
            asyncQueue->stop();
            delete asyncQueue;
            asyncQueue = nullptr;
        }
        
        // 关闭文件流
        if (fileStream) {
            fileStream->close();
            delete fileStream;
            fileStream = nullptr;
        }
        
        isInit = false;
        asyncMode = false;
    }
    
    bool flush(int timeoutMs = -1) {
        if (asyncMode && asyncQueue) {
            return asyncQueue->flush(timeoutMs);
        }
        
        // 同步模式下也尝试刷新文件流
        std::lock_guard<std::mutex> lock(logMutex);
        if (fileStream) {
            fileStream->flush();
        }
        return true;
    }
    
    bool isAsyncModeEnabled() const {
        return asyncMode;
    }
    
private:
    LogLevel logLevel;
    std::ofstream* fileStream;
    bool isInit;
    bool asyncMode;
    AsyncLogQueue* asyncQueue;
    std::mutex logMutex;
    
    // 格式化并写入日志到输出目标
    void writeLogToOutputs(const LogEntry& entry) {
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm;
        localtime_s(&local_tm, &now_time);
        
        // 格式化时间
        char timeStr[64];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &local_tm);
        
        // 获取毫秒
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        // 获取日志级别字符串
        const char* levelStr = getLevelString(entry.level);
        
        // 构建完整日志行
        std::stringstream logStream;
        logStream << "[" << timeStr << "." << std::setw(3) << std::setfill('0') << ms.count() << "] [" << levelStr << "] ";
        
        // 添加文件名和行号（如果有）
        if (entry.fileLen > 0 && entry.line > 0) {
            logStream << "(" << entry.file << ":" << entry.line << ") ";
        }
        
        // 添加日志消息
        logStream << entry.message << std::endl;
        
        std::string logLine = logStream.str();
        
        // 输出到文件
        if (fileStream) {
            *fileStream << logLine;
            fileStream->flush();
        }
        
        // 输出到控制台
        if (entry.level >= LogLevel::warn) {
            // 警告和错误输出到stderr
            std::cerr << logLine;
        } else {
            std::cout << logLine;
        }
    }
    
    // 兼容旧接口的重载版本
    void writeLogToOutputs(LogLevel level, const std::string& message) {
        LogEntry entry(level, message);
        writeLogToOutputs(entry);
    }
    
    // 处理日志条目批次（异步模式下使用）
    void processLogEntries(const std::vector<LogEntry>& entries) {
        std::lock_guard<std::mutex> lock(logMutex);
        
        for (const auto& entry : entries) {
            writeLogToOutputs(entry);
        }
    }
    
    const char* getLevelString(LogLevel level) {
        switch (level) {
            case LogLevel::trace:
                return "TRACE";
            case LogLevel::debug:
                return "DEBUG";
            case LogLevel::info:
                return "INFO";
            case LogLevel::warn:
                return "WARN";
            case LogLevel::error:
                return "ERROR";
            case LogLevel::critical:
                return "CRITICAL";
            default:
                return "UNKNOWN";
        }
    }
};

// WinLog类的实现
WinLog::WinLog() : pImpl(new Impl()) {}

WinLog::~WinLog() {
    delete pImpl;
}

// 使用Meyers单例模式，C++11后保证线程安全
WinLog& WinLog::getInstance() {
    static WinLog instance;
    return instance;
}

bool WinLog::init(const char* logFilePath, LogLevel level) {
    std::lock_guard<std::mutex> lock(globalWinLogMutex);
    return pImpl->init(logFilePath, level);
}

bool WinLog::init(const char* logFilePath, LogLevel level, const AsyncConfig& config) {
    std::lock_guard<std::mutex> lock(globalWinLogMutex);
    return pImpl->init(logFilePath, level, config);
}

bool WinLog::flush(int timeoutMs) {
    return pImpl->flush(timeoutMs);
}

void WinLog::setAsyncConfig(const AsyncConfig& config) {
    std::lock_guard<std::mutex> lock(globalWinLogMutex);
    asyncConfig = config;
    
    // 更新AsyncLogQueue的静态配置
    AsyncLogQueue::setDropOnOverflow(config.dropOnOverflow);
    AsyncLogQueue::setFlushIntervalMs(config.flushIntervalMs);
}

AsyncConfig WinLog::getAsyncConfig() const {
    std::lock_guard<std::mutex> lock(globalWinLogMutex);
    return asyncConfig;
}

bool WinLog::isAsyncModeEnabled() const {
    return pImpl->isAsyncModeEnabled();
}

void WinLog::trace(const char* format, ...) {
    va_list args;
    va_start(args, format);
    pImpl->log(LogLevel::trace, format, args);
    va_end(args);
}

void WinLog::debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    pImpl->log(LogLevel::debug, format, args);
    va_end(args);
}

void WinLog::info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    pImpl->log(LogLevel::info, format, args);
    va_end(args);
}

void WinLog::warn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    pImpl->log(LogLevel::warn, format, args);
    va_end(args);
}

void WinLog::error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    pImpl->log(LogLevel::error, format, args);
    va_end(args);
}

void WinLog::critical(const char* format, ...) {
    va_list args;
    va_start(args, format);
    pImpl->log(LogLevel::critical, format, args);
    va_end(args);
}

void WinLog::setLevel(LogLevel level) {
    pImpl->setLevel(level);
}

void WinLog::shutdown() {
    std::lock_guard<std::mutex> lock(globalWinLogMutex);
    pImpl->shutdown();
}

// 版本管理接口实现
int WinLog::getVersionMajor() {
    return WINLOG_VERSION_MAJOR;
}

int WinLog::getVersionMinor() {
    return WINLOG_VERSION_MINOR;
}

int WinLog::getVersionPatch() {
    return WINLOG_VERSION_PATCH;
}

int WinLog::getVersionBuild() {
    return WINLOG_VERSION_BUILD;
}

const char* WinLog::getVersionString() {
    return WINLOG_VERSION_STRING;
}

unsigned int WinLog::getVersionNumber() {
    return WINLOG_VERSION_NUMBER;
}

// 全局日志函数实现
void logTrace(const char* format, ...) {
    va_list args;
    va_start(args, format);
    WinLog::getInstance().trace(format, args);
    va_end(args);
}

void logDebug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    WinLog::getInstance().debug(format, args);
    va_end(args);
}

void logInfo(const char* format, ...) {
    va_list args;
    va_start(args, format);
    WinLog::getInstance().info(format, args);
    va_end(args);
}

void logWarn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    WinLog::getInstance().warn(format, args);
    va_end(args);
}

void logError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    WinLog::getInstance().error(format, args);
    va_end(args);
}

void logCritical(const char* format, ...) {
    va_list args;
    va_start(args, format);
    WinLog::getInstance().critical(format, args);
    va_end(args);
}

// 版本管理全局接口实现
int getWinLogVersionMajor() {
    return WinLog::getVersionMajor();
}

int getWinLogVersionMinor() {
    return WinLog::getVersionMinor();
}

int getWinLogVersionPatch() {
    return WinLog::getVersionPatch();
}

int getWinLogVersionBuild() {
    return WinLog::getVersionBuild();
}

const char* getWinLogVersionString() {
    return WinLog::getVersionString();
}

unsigned int getWinLogVersionNumber() {
    return WinLog::getVersionNumber();
}