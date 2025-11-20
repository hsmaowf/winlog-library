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
#include <unordered_map>

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
        // 内存池统计信息
        size_t totalAllocations;     // 总分配次数
        size_t totalDeallocations;   // 总释放次数
        size_t peakPoolSize;         // 峰值池大小
        size_t currentPoolSize;      // 当前池大小
        size_t tlsCacheHits;         // 线程本地缓存命中次数
    };
    
    // 获取当前统计信息
    Stats getStats() const;
    
    // 重置统计信息
    void resetStats();
    
    // 静态配置方法
    static void setDropOnOverflow(bool drop);
    static void setFlushIntervalMs(int ms);

private:
    // 线程本地缓存结构
    struct ThreadLocalCache {
        std::vector<LogEntry*> entries; // 本地缓存的对象
        static constexpr size_t CACHE_SIZE = 32; // 每个线程的缓存大小
        static constexpr size_t BATCH_THRESHOLD = 8; // 批量回收阈值
    };
    
    // 获取当前线程的本地缓存
    ThreadLocalCache& getThreadLocalCache();
    
    // 从本地缓存批量转移对象到全局池
    void refillGlobalPool(ThreadLocalCache& cache);
    
    // 日志处理工作线程函数
    void workerThread();
    
    // 从队列中批量获取日志
    std::vector<LogEntry> dequeueBatch();
    
    // 分配日志条目（优化版）
    LogEntry* allocateEntry();
    
    // 释放日志条目（优化版）
    void freeEntry(LogEntry* entry);
    
    // 批量分配日志条目
    std::vector<LogEntry*> allocateBatch(size_t count);
    
    // 批量释放日志条目
    void freeBatch(const std::vector<LogEntry*>& entries);
    
    // 内部成员变量
    size_t queueSize_;           // 队列最大大小
    size_t maxBatchSize_;        // 最大批量处理大小
    size_t memoryPoolSize_;      // 内存池初始大小
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
    
    // 优化的内存池相关 - 使用无锁实现
    std::vector<LogEntry*> freeList_;      // 全局空闲对象列表
    std::mutex poolMutex_;                 // 全局池锁
    std::atomic<size_t> totalAllocations_; // 总分配计数（无锁）
    std::atomic<size_t> totalDeallocations_; // 总释放计数（无锁）
    std::atomic<size_t> peakPoolSize_;     // 峰值池大小（无锁）
    std::atomic<size_t> currentPoolSize_;  // 当前池大小（无锁）
    std::atomic<size_t> tlsCacheHits_;     // 线程本地缓存命中计数（无锁）
    
    // 线程本地存储 - 每个线程有自己的缓存
    static thread_local std::unique_ptr<ThreadLocalCache> threadLocalCache;
    
    // 线程缓存清理机制
    std::mutex threadCacheMutex_;
    std::unordered_map<std::thread::id, ThreadLocalCache*> threadCaches_;
    
    // 统计信息
    mutable std::mutex statsMutex_;
    Stats stats_;
};

#endif // ASYNC_LOG_QUEUE_H