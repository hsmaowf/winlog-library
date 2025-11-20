#include "async_log_queue.h"
#include <iostream>
#include <chrono>
#include <functional>
#include <cstring>

// 注意：LogEntry的实现已经在winlog.cpp中，这里不需要重复实现

// AsyncLogQueue 构造函数
AsyncLogQueue::AsyncLogQueue(size_t queueSize, size_t maxBatchSize, size_t memoryPoolSize, bool dropOnOverflow, int flushIntervalMs) :
    queueSize_(queueSize),
    maxBatchSize_(maxBatchSize),
    stopRequested_(false),
    stats_() {
    // 设置静态配置（仅当传入的参数与默认值不同时）
    if (dropOnOverflow != dropOnOverflow_) {
        dropOnOverflow_ = dropOnOverflow;
    }
    if (flushIntervalMs > 0 && flushIntervalMs != flushIntervalMs_) {
        flushIntervalMs_ = flushIntervalMs;
    }
    // 初始化内存池
    for (size_t i = 0; i < memoryPoolSize; ++i) {
        freeList_.push_back(new LogEntry());
    }
    
    // 启动工作线程
    workerThread_ = std::thread(&AsyncLogQueue::workerThread, this);
}

// AsyncLogQueue 析构函数
AsyncLogQueue::~AsyncLogQueue() {
    stop();
    
    // 释放内存池中的对象
    std::lock_guard<std::mutex> lock(poolMutex_);
    for (auto entry : freeList_) {
        delete entry;
    }
    freeList_.clear();
}

// AsyncLogQueue 静态成员变量定义
bool AsyncLogQueue::dropOnOverflow_ = false;
int AsyncLogQueue::flushIntervalMs_ = 1000;

// 设置日志处理回调函数
void AsyncLogQueue::setLogHandler(AsyncLogQueue::LogHandler handler) {
    logHandler_ = std::move(handler);
}

// 设置队列溢出时是否丢弃日志
void AsyncLogQueue::setDropOnOverflow(bool drop) {
    dropOnOverflow_ = drop;
}

// 设置自动刷新间隔
void AsyncLogQueue::setFlushIntervalMs(int ms) {
    if (ms > 0) {
        flushIntervalMs_ = ms;
    }
}

// 添加日志到队列（拷贝版本）
bool AsyncLogQueue::enqueue(const LogEntry& entry) {
    // 由于LogEntry的拷贝构造函数被删除，我们需要创建一个新的LogEntry对象并手动复制必要的字段
    LogEntry tempEntry;
    tempEntry.level = entry.level;
    // 复制message字段
    memcpy(tempEntry.message, entry.message, sizeof(tempEntry.message));
    tempEntry.message[sizeof(tempEntry.message) - 1] = '\0'; // 确保字符串以null结尾
    // 复制其他字段
    tempEntry.line = entry.line;
    tempEntry.file[0] = '\0'; // 清空file字段
    // 调用移动版本的enqueue方法
    return enqueue(std::move(tempEntry));
}

// 添加日志到队列（移动版本）
bool AsyncLogQueue::enqueue(LogEntry&& entry) {
    std::unique_lock<std::mutex> lock(queueMutex_);
    
    // 检查队列是否已满且已停止
    if (isStopped()) {
        return false;
    }
    
    // 检查队列是否已满
    if (queue_.size() >= queueSize_) {
        // 根据配置决定是丢弃还是等待
        if (dropOnOverflow_) {
            // 更新统计信息
            { 
                std::lock_guard<std::mutex> statsLock(statsMutex_);
                stats_.totalDropped++;
            }
            return false;
        }
        
        // 不丢弃时等待队列不满
        bool success = notFull_.wait_for(lock, std::chrono::milliseconds(100), 
            [this] { return queue_.size() < queueSize_ || isStopped(); });
        
        if (!success || isStopped()) {
            // 更新统计信息
            { 
                std::lock_guard<std::mutex> statsLock(statsMutex_);
                stats_.totalDropped++;
            }
            return false;
        }
    }
    
    // 添加到队列（使用移动语义）
    queue_.push(std::move(entry));
    
    // 更新统计信息
    { 
        std::lock_guard<std::mutex> statsLock(statsMutex_);
        stats_.totalEnqueued++;
        stats_.currentQueueSize = queue_.size();
    }
    
    // 通知消费者
    notEmpty_.notify_one();
    return true;
}

// 刷新队列中的所有日志
bool AsyncLogQueue::flush(int timeoutMs) {
    std::unique_lock<std::mutex> lock(queueMutex_);
    
    if (isStopped()) {
        return false;
    }
    
    // 通知工作线程有数据需要处理
    notEmpty_.notify_one();
    
    // 等待队列清空
    bool flushed = notFull_.wait_for(lock, std::chrono::milliseconds(timeoutMs == -1 ? 5000 : timeoutMs), 
        [this] { return queue_.empty() || isStopped(); });
    
    return flushed;
}

// 停止日志处理线程
void AsyncLogQueue::stop() {
    if (stopRequested_) {
        return;
    }
    
    // 设置停止标志
    stopRequested_ = true;
    
    // 通知所有等待的线程
    notEmpty_.notify_all();
    notFull_.notify_all();
    
    // 等待工作线程结束
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}

// 获取队列当前大小
size_t AsyncLogQueue::size() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return queue_.size();
}

// 判断队列是否已满
bool AsyncLogQueue::isFull() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return queue_.size() >= queueSize_;
}

// 判断队列是否已停止
bool AsyncLogQueue::isStopped() const {
    return stopRequested_;
}

// 获取当前统计信息
AsyncLogQueue::Stats AsyncLogQueue::getStats() const {
    std::lock_guard<std::mutex> lock(statsMutex_);
    return stats_;
}

// 重置统计信息
void AsyncLogQueue::resetStats() {
    std::lock_guard<std::mutex> lock(statsMutex_);
    stats_.totalEnqueued = 0;
    stats_.totalDropped = 0;
    stats_.totalProcessed = 0;
    stats_.currentQueueSize = 0;
}

// 日志处理工作线程函数
void AsyncLogQueue::workerThread() {
    auto lastFlushTime = std::chrono::steady_clock::now();
    
    while (!stopRequested_) {
        // 检查是否需要自动刷新
        auto now = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFlushTime).count();
        
        // 从队列中批量获取日志
        std::vector<LogEntry> batch = dequeueBatch();
        
        if (!batch.empty() && logHandler_) {
            try {
                // 处理日志批次
                logHandler_(batch);
                
                // 更新统计信息
                { 
                    std::lock_guard<std::mutex> statsLock(statsMutex_);
                    stats_.totalProcessed += batch.size();
                    stats_.currentQueueSize = queue_.size();
                }
                
                // 更新最后刷新时间
                lastFlushTime = now;
            } catch (const std::exception& e) {
                std::cerr << "Error in log handler: " << e.what() << std::endl;
            }
        } else if (elapsedMs >= flushIntervalMs_ && !queue_.empty()) {
            // 自动刷新间隔到达且队列非空，强制处理剩余日志
            batch = dequeueBatch();
            if (!batch.empty() && logHandler_) {
                try {
                    logHandler_(batch);
                    { 
                        std::lock_guard<std::mutex> statsLock(statsMutex_);
                        stats_.totalProcessed += batch.size();
                        stats_.currentQueueSize = queue_.size();
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error in log handler: " << e.what() << std::endl;
                }
            }
            lastFlushTime = now;
        }
        
        // 如果队列为空，等待新的日志或超时
        if (queue_.empty() && !stopRequested_) {
            std::unique_lock<std::mutex> lock(queueMutex_);
            if (queue_.empty() && !stopRequested_) {
                // 等待直到有新日志或超时（自动刷新间隔）
                notEmpty_.wait_for(lock, std::chrono::milliseconds(flushIntervalMs_));
            }
        }
    }
    
    // 处理剩余的日志
    std::vector<LogEntry> remaining = dequeueBatch();
    while (!remaining.empty() && logHandler_) {
        try {
            logHandler_(remaining);
            remaining = dequeueBatch();
        } catch (const std::exception& e) {
            std::cerr << "Error in log handler: " << e.what() << std::endl;
            break;
        }
    }
}

// 从队列中批量获取日志
std::vector<LogEntry> AsyncLogQueue::dequeueBatch() {
    std::vector<LogEntry> batch;
    size_t count = 0;
    
    std::lock_guard<std::mutex> lock(queueMutex_);
    
    // 限制最大批量大小
    while (!queue_.empty() && count < maxBatchSize_) {
        batch.push_back(std::move(queue_.front()));
        queue_.pop();
        count++;
    }
    
    // 通知生产者队列不满
    if (!queue_.empty() && queue_.size() < queueSize_) {
        notFull_.notify_one();
    }
    
    return batch;
}

// 分配日志条目
LogEntry* AsyncLogQueue::allocateEntry() {
    std::lock_guard<std::mutex> lock(poolMutex_);
    
    if (!freeList_.empty()) {
        LogEntry* entry = freeList_.back();
        freeList_.pop_back();
        entry->reset();
        return entry;
    }
    
    // 如果没有空闲对象，创建新的
    return new LogEntry();
}

// 释放日志条目
void AsyncLogQueue::freeEntry(LogEntry* entry) {
    if (!entry) return;
    
    std::lock_guard<std::mutex> lock(poolMutex_);
    entry->reset();
    freeList_.push_back(entry);
}