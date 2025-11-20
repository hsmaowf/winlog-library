# WinLog 异步日志队列架构设计

## 📊 架构概述

### 核心组件

1. **LogEntry** - 日志条目结构
   - 存储单条日志的所有信息
   - 包含时间戳、日志级别、消息内容等
   - 优化内存使用的设计

2. **AsyncLogQueue** - 异步日志队列
   - 线程安全的队列实现
   - 生产-消费者模式
   - 支持高并发写入

3. **LogWorker** - 日志工作线程
   - 后台处理日志队列
   - 批量写入文件
   - 错误处理和恢复机制

4. **MemoryPool** - 内存池管理
   - 预分配和重用LogEntry对象
   - 减少内存碎片
   - 提高性能

5. **AsyncConfig** - 异步配置管理
   - 队列大小控制
   - 刷新间隔配置
   - 线程优先级设置

### 数据流图

```
┌─────────────────┐      ┌───────────────────┐      ┌─────────────────┐      ┌─────────────────┐
│                 │      │                   │      │                 │      │                 │
│  应用线程       │──────▶  AsyncLogQueue   │──────▶  LogWorker     │──────▶  文件系统/控制台 │
│  (生产者)       │      │  (缓冲区)         │      │  (消费者)       │      │                 │
│                 │      │                   │      │                 │      │                 │
└─────────────────┘      └───────────────────┘      └─────────────────┘      └─────────────────┘
        ▲                         │                         │
        │                         │                         │
        │                         │                         │
        │                         ▼                         ▼
┌─────────────────┐      ┌───────────────────┐      ┌─────────────────┐
│                 │      │                   │      │                 │
│  MemoryPool     │◀─────▶  内存分配/释放    │      │  错误处理和监控 │
│                 │      │                   │      │                 │
└─────────────────┘      └───────────────────┘      └─────────────────┘
```

## 📝 详细设计

### 1. LogEntry 结构

```cpp
struct LogEntry {
    std::chrono::system_clock::time_point timestamp;  // 日志时间戳
    LogLevel level;                                  // 日志级别
    std::string message;                             // 日志消息内容
    
    // 构造函数和析构函数
    LogEntry();
    LogEntry(LogLevel level, const std::string& message);
    LogEntry(LogEntry&& other) noexcept;            // 移动构造，提高性能
    LogEntry& operator=(LogEntry&& other) noexcept; // 移动赋值
    
    // 禁用拷贝操作
    LogEntry(const LogEntry&) = delete;
    LogEntry& operator=(const LogEntry&) = delete;
    
    // 重置对象状态
    void reset();
};
```

### 2. AsyncLogQueue 类

```cpp
class AsyncLogQueue {
public:
    AsyncLogQueue(size_t maxQueueSize = 10000);
    ~AsyncLogQueue();
    
    // 推送日志到队列
    bool push(LogEntry&& entry);
    
    // 批量推送多个日志
    bool pushBatch(const std::vector<LogEntry>& entries);
    
    // 从队列获取日志（阻塞调用）
    bool pop(LogEntry& entry, int timeoutMs = -1);
    
    // 批量获取日志
    size_t popBatch(std::vector<LogEntry>& entries, size_t maxEntries, int timeoutMs = -1);
    
    // 获取队列当前大小
    size_t size() const;
    
    // 获取队列最大容量
    size_t capacity() const;
    
    // 判断队列是否为空
    bool empty() const;
    
    // 判断队列是否已满
    bool full() const;
    
    // 清空队列
    void clear();
    
    // 获取统计信息
    struct Statistics {
        size_t totalPushed;     // 总推送数量
        size_t totalPopped;     // 总弹出数量
        size_t droppedLogs;     // 丢弃的日志数量
        size_t peakSize;        // 峰值队列大小
    };
    
    Statistics getStatistics() const;
    
private:
    std::queue<LogEntry> logQueue;              // 日志队列
    mutable std::mutex queueMutex;              // 队列互斥锁
    std::condition_variable notEmptyCondition;  // 非空条件变量
    std::condition_variable notFullCondition;   // 非满条件变量
    size_t maxQueueSize;                        // 最大队列大小
    bool shutdownRequested;                     // 是否请求关闭
    
    // 统计信息
    std::atomic<size_t> totalPushed{0};
    std::atomic<size_t> totalPopped{0};
    std::atomic<size_t> droppedLogs{0};
    std::atomic<size_t> peakSize{0};
};
```

### 3. LogWorker 类

```cpp
class LogWorker {
public:
    LogWorker(AsyncLogQueue& queue, const char* logFilePath = nullptr);
    ~LogWorker();
    
    // 启动工作线程
    bool start();
    
    // 停止工作线程
    void stop();
    
    // 等待刷新完成
    bool flush(int timeoutMs = -1);
    
    // 设置日志级别
    void setLogLevel(LogLevel level);
    
    // 设置文件路径
    bool setLogFilePath(const char* logFilePath);
    
    // 配置批量处理参数
    void setBatchParameters(size_t maxBatchSize, int flushIntervalMs);
    
    // 获取工作状态
    bool isRunning() const;
    
    // 获取统计信息
    struct Statistics {
        size_t processedLogs;   // 处理的日志数量
        size_t writeErrors;     // 写入错误数量
        double avgProcessTime;  // 平均处理时间(ms)
    };
    
    Statistics getStatistics() const;
    
private:
    // 工作线程主函数
    void workerThreadFunc();
    
    // 批量处理日志
    void processBatch(const std::vector<LogEntry>& entries);
    
    // 写入单条日志到文件
    bool writeLogToFile(const LogEntry& entry);
    
    // 写入日志到控制台
    void writeLogToConsole(const LogEntry& entry);
    
    // 格式化日志条目
    std::string formatLogEntry(const LogEntry& entry);
    
private:
    AsyncLogQueue& logQueue;                // 引用的日志队列
    std::ofstream* fileStream;              // 文件输出流
    std::thread workerThread;               // 工作线程
    std::atomic<bool> running;              // 运行状态
    std::mutex workerMutex;                 // 工作线程互斥锁
    std::condition_variable flushCondition; // 刷新条件变量
    LogLevel currentLevel;                  // 当前日志级别
    
    // 批量处理配置
    size_t maxBatchSize;                    // 最大批量大小
    int flushIntervalMs;                    // 刷新间隔(毫秒)
    
    // 统计信息
    std::atomic<size_t> processedLogs{0};
    std::atomic<size_t> writeErrors{0};
    std::chrono::steady_clock::time_point startTime;
};
```

### 4. MemoryPool 类

```cpp
class MemoryPool {
public:
    MemoryPool(size_t initialSize = 1000, size_t maxSize = 10000);
    ~MemoryPool();
    
    // 从池中获取LogEntry对象
    LogEntry* acquire();
    
    // 释放LogEntry对象回池
    void release(LogEntry* entry);
    
    // 预分配内存
    bool preallocate(size_t count);
    
    // 收缩内存池
    void shrinkTo(size_t targetSize);
    
    // 清空内存池
    void clear();
    
    // 获取统计信息
    struct Statistics {
        size_t currentSize;     // 当前池大小
        size_t peakSize;        // 峰值大小
        size_t allocCount;      // 分配次数
        size_t releaseCount;    // 释放次数
        size_t misses;          // 池未命中次数
    };
    
    Statistics getStatistics() const;
    
private:
    std::vector<std::unique_ptr<LogEntry>> pool;
    std::stack<LogEntry*> available;
    std::mutex poolMutex;
    size_t initialSize;
    size_t maxSize;
    
    // 统计信息
    std::atomic<size_t> peakSize{0};
    std::atomic<size_t> allocCount{0};
    std::atomic<size_t> releaseCount{0};
    std::atomic<size_t> misses{0};
};
```

## 🚀 性能优化策略

### 1. 内存管理优化
- **内存池**：预分配和重用LogEntry对象
- **移动语义**：使用C++11移动语义避免不必要的拷贝
- **缓冲区设计**：减少动态内存分配

### 2. 队列性能优化
- **无锁设计考虑**：高并发场景下的无锁队列选项
- **批量操作**：支持批量推送和批量消费
- **阻塞/非阻塞模式**：灵活的API支持不同场景

### 3. I/O优化
- **批量写入**：收集多条日志一次性写入
- **缓冲刷新策略**：基于时间和数量的双触发机制
- **异步I/O**：考虑使用Windows的异步I/O API

## ⚙️ 配置参数

| 参数 | 类型 | 默认值 | 描述 |
|------|------|--------|------|
| queueSize | size_t | 10000 | 异步队列最大容量 |
| asyncEnabled | bool | true | 是否启用异步模式 |
| flushIntervalMs | int | 1000 | 自动刷新间隔(毫秒) |
| maxBatchSize | size_t | 100 | 最大批量处理大小 |
| memoryPoolSize | size_t | 1000 | 内存池初始大小 |
| maxMemoryPoolSize | size_t | 10000 | 内存池最大大小 |
| dropOnOverflow | bool | false | 队列满时是否丢弃日志 |
| workerPriority | int | 0 | 工作线程优先级 |

## 📊 预期性能提升

| 场景 | 当前实现 | 异步实现 | 预期提升 |
|------|----------|----------|----------|
| 单线程写入 | ~1,000/秒 | ~100,000/秒 | 100倍 |
| 多线程写入 | ~500/秒 | ~500,000/秒 | 1000倍 |
| 峰值处理能力 | ~2,000/秒 | ~1,000,000/秒 | 500倍 |
| 内存使用 | 动态 | 可控 | 减少50% |
| CPU占用 | 高 | 低 | 降低80% |

## 🔧 实现步骤

1. 实现基础的LogEntry结构
2. 实现AsyncLogQueue核心功能
3. 实现MemoryPool内存管理
4. 实现LogWorker工作线程
5. 集成到WinLog类
6. 添加配置接口
7. 编写测试和性能基准

## 📝 注意事项

1. **线程安全**：所有共享数据结构必须保证线程安全
2. **异常处理**：避免异常逃逸导致程序崩溃
3. **资源管理**：确保资源正确释放，避免泄漏
4. **优雅关闭**：确保程序退出时日志能被正确处理
5. **错误恢复**：日志写入失败时的降级处理
6. **性能监控**：添加关键性能指标的统计

---

## 💡 设计原则

1. **高性能优先**：最小化锁竞争，优化内存使用
2. **可靠性保障**：确保日志不丢失，错误可恢复
3. **可配置性**：提供灵活的配置选项适应不同场景
4. **可扩展性**：易于扩展支持新的输出目标
5. **易用性**：保持简单的API接口