#ifndef ASYNC_LOG_QUEUE_H
#define ASYNC_LOG_QUEUE_H

#include "winlog.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <functional>
#include <thread>
#include <vector>

// 异步日志队列类
class WINLOG_API AsyncLogQueue {
public:
    // 日志处理回调函数类型
    using LogHandler = std::function<void(const std::vector<LogEntry>&)>;
    
    // 构造函数
    AsyncLogQueue(size_t queueSize, size_t maxBatchSize, size_t memoryPoolSize, bool dropOnOverflow = false, int flushIntervalMs = 1000);
    
    // 析构函数
    ~AsyncLogQueue();
    
    // 禁止拷贝构造和赋值操作
    AsyncLogQueue(const AsyncLogQueue&) = delete;
    AsyncLogQueue& operator=(const AsyncLogQueue&) = delete;
    
    // 设置日志处理回调函数
    void setLogHandler(LogHandler handler);
    
    // 添加日志到队列
    bool enqueue(const LogEntry& entry);
    
    // 添加日志到队列（移动语义）
    bool enqueue(LogEntry&& entry);
    
    // 刷新队列中的所有日志
    bool flush(int timeoutMs = -1);
    
    // 停止日志处理线程
    void stop();
    
    // 获取队列当前大小
    size_t size() const;
    
    // 判断队列是否已满
    bool isFull() const;
    
    // 判断队列是否已停止
    bool isStopped() const;
    
    // 获取队列统计信息
    struct Stats {
        size_t totalEnqueued;        // 总入队数
        size_t totalDropped;         // 总丢弃数
        size_t totalProcessed;       // 总处理数
        size_t currentQueueSize;     // 当前队列大小
    };
    
    // 获取当前统计信息
    Stats getStats() const;
    
    // 重置统计信息
    void resetStats();
    
    // 静态配置方法
    static void setDropOnOverflow(bool drop);
    static void setFlushIntervalMs(int ms);

private:
    // 日志处理工作线程函数
    void workerThread();
    
    // 从队列中批量获取日志
    std::vector<LogEntry> dequeueBatch();
    
    // 分配日志条目
    LogEntry* allocateEntry();
    
    // 释放日志条目
    void freeEntry(LogEntry* entry);
    
    // 内部成员变量
    size_t queueSize_;           // 队列最大大小
    size_t maxBatchSize_;        // 最大批量处理大小
    static bool dropOnOverflow_; // 队列溢出时是否丢弃
    static int flushIntervalMs_; // 自动刷新间隔（毫秒）
    
    // 线程安全队列
    std::queue<LogEntry> queue_;
    mutable std::mutex queueMutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    
    // 日志处理相关
    LogHandler logHandler_;
    std::thread workerThread_;
    std::atomic<bool> stopRequested_;     // 停止请求标志
    
    // 内存池相关
    std::vector<LogEntry*> freeList_;      // 空闲对象列表
    std::mutex poolMutex_;
    
    // 统计信息
    mutable std::mutex statsMutex_;
    Stats stats_;
};

#endif // ASYNC_LOG_QUEUE_H