#include "async_log_queue.h"
#include <iostream>
#include <chrono>
#include <functional>
#include <cstring>
#include <algorithm>

// 注意：LogEntry的实现已经在winlog.cpp中，这里不需要重复实现

// 初始化线程本地存储静态成员
thread_local std::unique_ptr<AsyncLogQueue::ThreadLocalCache> AsyncLogQueue::threadLocalCache = nullptr;

// AsyncLogQueue 构造函数
AsyncLogQueue::AsyncLogQueue(size_t queueSize, size_t maxBatchSize, size_t memoryPoolSize, bool dropOnOverflow, int flushIntervalMs) :
    queueSize_(queueSize),
    maxBatchSize_(maxBatchSize),
    memoryPoolSize_(memoryPoolSize),
    stopRequested_(false),
    totalAllocations_(0),
    totalDeallocations_(0),
    peakPoolSize_(0),
    currentPoolSize_(0),
    tlsCacheHits_(0),
    stats_() {
    // 设置静态配置（仅当传入的参数与默认值不同时）
    if (dropOnOverflow != dropOnOverflow_) {
        dropOnOverflow_ = dropOnOverflow;
    }
    if (flushIntervalMs > 0 && flushIntervalMs != flushIntervalMs_) {
        flushIntervalMs_ = flushIntervalMs;
    }
    
    // 初始化全局内存池
    {  
        std::lock_guard<std::mutex> lock(poolMutex_);
        freeList_.reserve(memoryPoolSize); // 预分配空间，减少内存碎片
        for (size_t i = 0; i < memoryPoolSize; ++i) {
            freeList_.push_back(new LogEntry());
        }
        currentPoolSize_ = freeList_.size();
        size_t current = currentPoolSize_.load();
    size_t peak = peakPoolSize_.load();
    if (current > peak) {
        peakPoolSize_.store(current);
    }
    }
    
    // 启动工作线程
    workerThread_ = std::thread(&AsyncLogQueue::workerThread, this);
}

// AsyncLogQueue 析构函数
AsyncLogQueue::~AsyncLogQueue() {
    stop();
    
    // 释放全局内存池中的对象
    std::lock_guard<std::mutex> lock(poolMutex_);
    for (auto entry : freeList_) {
        delete entry;
    }
    freeList_.clear();
    currentPoolSize_ = 0;
    
    // 清理线程本地缓存
    std::lock_guard<std::mutex> threadLock(threadCacheMutex_);
    // 注意：线程本地缓存通常会在线程结束时自动销毁，这里只是为了确保所有资源都被释放
}

// 获取当前线程的本地缓存
AsyncLogQueue::ThreadLocalCache& AsyncLogQueue::getThreadLocalCache() {
    if (!threadLocalCache) {
        threadLocalCache = std::make_unique<ThreadLocalCache>();
        threadLocalCache->entries.reserve(ThreadLocalCache::CACHE_SIZE);
        
        // 注册到全局线程缓存映射中，以便后续管理
        std::lock_guard<std::mutex> lock(threadCacheMutex_);
        threadCaches_[std::this_thread::get_id()] = threadLocalCache.get();
    }
    return *threadLocalCache;
}

// 从本地缓存批量转移对象到全局池
void AsyncLogQueue::refillGlobalPool(ThreadLocalCache& cache) {
    if (cache.entries.empty()) return;
    
    std::lock_guard<std::mutex> lock(poolMutex_);
    
    // 批量转移，减少锁争用
    freeList_.insert(freeList_.end(), cache.entries.begin(), cache.entries.end());
    
    // 更新当前池大小
    currentPoolSize_ = freeList_.size();
    size_t current = currentPoolSize_.load();
    size_t peak = peakPoolSize_.load();
    if (current > peak) {
        peakPoolSize_.store(current);
    }
    
    // 清空本地缓存
    cache.entries.clear();
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
    
    // 创建一个临时统计对象，包含最新的内存池信息
    Stats result = stats_;
    
    // 更新内存池统计信息（使用原子操作获取最新值）
    result.totalAllocations = totalAllocations_.load();
    result.totalDeallocations = totalDeallocations_.load();
    result.peakPoolSize = peakPoolSize_.load();
    result.currentPoolSize = currentPoolSize_.load();
    result.tlsCacheHits = tlsCacheHits_.load();
    
    return result;
}

// 重置统计信息
void AsyncLogQueue::resetStats() {
    std::lock_guard<std::mutex> lock(statsMutex_);
    
    // 重置队列统计信息
    stats_.totalEnqueued = 0;
    stats_.totalDropped = 0;
    stats_.totalProcessed = 0;
    stats_.currentQueueSize = 0;
    
    // 重置内存池统计信息（原子操作）
    totalAllocations_ = 0;
    totalDeallocations_ = 0;
    tlsCacheHits_ = 0;
    // 注意：peakPoolSize_和currentPoolSize_不重置，因为它们反映当前状态
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

// 分配日志条目（优化版）
LogEntry* AsyncLogQueue::allocateEntry() {
    // 增加总分配计数
    totalAllocations_++;
    
    // 首先尝试从线程本地缓存获取，避免加锁
    ThreadLocalCache& cache = getThreadLocalCache();
    if (!cache.entries.empty()) {
        LogEntry* entry = cache.entries.back();
        cache.entries.pop_back();
        
        // 增加线程本地缓存命中计数
        tlsCacheHits_++;
        
        // 重置对象（如果需要，可以在从线程本地缓存获取时跳过重置，因为我们信任内部使用）
        entry->reset();
        return entry;
    }
    
    // 线程本地缓存为空，尝试从全局池批量获取
    std::vector<LogEntry*> batch;
    {  
        std::lock_guard<std::mutex> lock(poolMutex_);
        
        // 从全局池获取一批对象（最多CACHE_SIZE个）
        size_t takeCount = std::min(ThreadLocalCache::CACHE_SIZE, freeList_.size());
        if (takeCount > 0) {
            // 从尾部取对象，效率更高
            batch.reserve(takeCount);
            for (size_t i = 0; i < takeCount; ++i) {
                batch.push_back(freeList_.back());
                freeList_.pop_back();
            }
            
            // 更新当前池大小
            currentPoolSize_ = freeList_.size();
        }
    }
    
    // 如果获取到了对象
    if (!batch.empty()) {
        // 取出第一个对象作为返回值
        LogEntry* entry = batch.back();
        batch.pop_back();
        
        // 剩余的对象放入线程本地缓存
        if (!batch.empty()) {
            // 确保线程本地缓存有足够的空间
            if (cache.entries.capacity() < cache.entries.size() + batch.size()) {
                cache.entries.reserve(cache.entries.size() + batch.size());
            }
            cache.entries.insert(cache.entries.end(), batch.begin(), batch.end());
        }
        
        // 重置并返回对象
        entry->reset();
        return entry;
    }
    
    // 全局池也为空，创建新对象
    return new LogEntry();
}

// 释放日志条目（优化版）
void AsyncLogQueue::freeEntry(LogEntry* entry) {
    if (!entry) return;
    
    // 增加总释放计数
    totalDeallocations_++;
    
    // 首先尝试放入线程本地缓存
    ThreadLocalCache& cache = getThreadLocalCache();
    
    // 如果线程本地缓存未满，放入缓存
    if (cache.entries.size() < ThreadLocalCache::CACHE_SIZE) {
        entry->reset(); // 重置对象状态
        cache.entries.push_back(entry);
        return;
    }
    
    // 线程本地缓存已满，需要批量回收到全局池
    refillGlobalPool(cache);
    
    // 再次尝试放入缓存（现在应该有空间了）
    entry->reset();
    cache.entries.push_back(entry);
}

// 批量分配日志条目
std::vector<LogEntry*> AsyncLogQueue::allocateBatch(size_t count) {
    std::vector<LogEntry*> result;
    result.reserve(count);
    
    // 增加总分配计数
    totalAllocations_ += count;
    
    // 首先尝试从线程本地缓存获取
    ThreadLocalCache& cache = getThreadLocalCache();
    
    // 从线程本地缓存中取对象
    size_t takeFromCache = std::min(count, cache.entries.size());
    if (takeFromCache > 0) {
        // 从尾部取对象，效率更高
        for (size_t i = 0; i < takeFromCache; ++i) {
            result.push_back(cache.entries.back());
            cache.entries.pop_back();
        }
        
        // 增加线程本地缓存命中计数
        tlsCacheHits_ += takeFromCache;
        
        // 如果已经满足请求数量，直接返回
        if (result.size() >= count) {
            // 重置对象
            for (auto entry : result) {
                entry->reset();
            }
            return result;
        }
    }
    
    // 还需要从全局池获取更多对象
    size_t remaining = count - result.size();
    std::vector<LogEntry*> fromGlobal;
    
    {  
        std::lock_guard<std::mutex> lock(poolMutex_);
        
        // 从全局池获取剩余需要的对象
        size_t takeFromGlobal = std::min(remaining, freeList_.size());
        if (takeFromGlobal > 0) {
            fromGlobal.reserve(takeFromGlobal);
            for (size_t i = 0; i < takeFromGlobal; ++i) {
                fromGlobal.push_back(freeList_.back());
                freeList_.pop_back();
            }
            
            // 更新当前池大小
            currentPoolSize_ = freeList_.size();
        }
    }
    
    // 将从全局池获取的对象添加到结果中
    result.insert(result.end(), fromGlobal.begin(), fromGlobal.end());
    
    // 如果还需要更多对象，创建新的
    while (result.size() < count) {
        result.push_back(new LogEntry());
    }
    
    // 重置所有对象
    for (auto entry : result) {
        entry->reset();
    }
    
    return result;
}

// 批量释放日志条目
void AsyncLogQueue::freeBatch(const std::vector<LogEntry*>& entries) {
    if (entries.empty()) return;
    
    // 增加总释放计数
    totalDeallocations_ += entries.size();
    
    // 首先尝试放入线程本地缓存
    ThreadLocalCache& cache = getThreadLocalCache();
    
    // 计算可以放入线程本地缓存的数量
    size_t availableSpace = ThreadLocalCache::CACHE_SIZE - cache.entries.size();
    size_t putInCache = std::min(entries.size(), availableSpace);
    
    // 放入线程本地缓存
    if (putInCache > 0) {
        // 重置对象并放入缓存
        for (size_t i = 0; i < putInCache; ++i) {
            entries[i]->reset();
            cache.entries.push_back(entries[i]);
        }
    }
    
    // 如果有剩余对象需要释放，批量回收到全局池
    if (putInCache < entries.size()) {
        // 先尝试刷新线程本地缓存到全局池
        refillGlobalPool(cache);
        
        // 再次尝试放入缓存
        availableSpace = ThreadLocalCache::CACHE_SIZE - cache.entries.size();
        size_t remainingPutInCache = std::min(entries.size() - putInCache, availableSpace);
        
        for (size_t i = 0; i < remainingPutInCache; ++i) {
            entries[putInCache + i]->reset();
            cache.entries.push_back(entries[putInCache + i]);
        }
        
        // 如果还有剩余，直接放到全局池
        size_t directlyToGlobal = entries.size() - putInCache - remainingPutInCache;
        if (directlyToGlobal > 0) {
            std::lock_guard<std::mutex> lock(poolMutex_);
            
            // 重置并批量添加到全局池
            for (size_t i = 0; i < directlyToGlobal; ++i) {
                entries[putInCache + remainingPutInCache + i]->reset();
                freeList_.push_back(entries[putInCache + remainingPutInCache + i]);
            }
            
            // 更新当前池大小
            currentPoolSize_ = freeList_.size();
            size_t current = currentPoolSize_.load();
            size_t peak = peakPoolSize_.load();
            if (current > peak) {
                peakPoolSize_.store(current);
            }
        }
    }
}