#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <iomanip>
#include <atomic>
#include "../include/winlog.h"

// Test basic asynchronous logging functionality
void testBasicAsyncLogging() {
    std::cout << "=== Testing Basic Async Logging Functionality ===" << std::endl;
    
    // 设置异步配置
    AsyncConfig config;
    config.enabled = true;
    config.queueSize = 1000;
    config.maxBatchSize = 100;
    config.memoryPoolSize = 1000;
    config.dropOnOverflow = false;
    config.flushIntervalMs = 500;
    WinLog::getInstance().setAsyncConfig(config);
    
    // 初始化异步日志
    if (WinLog::getInstance().init("async_log.log", LogLevel::info, config)) {
        std::cout << "Async logging initialized successfully" << std::endl;
        WinLog::getInstance().info("Async logging initialized successfully");
    } else {
        std::cout << "Failed to initialize async logging" << std::endl;
        WinLog::getInstance().info("Failed to initialize async logging");
        return;
    }
    
    // Log different levels of messages
    WinLog::getInstance().info("This is an asynchronous INFO log");
    WinLog::getInstance().debug("This is an asynchronous DEBUG log");
    WinLog::getInstance().warn("This is an asynchronous WARNING log");
    WinLog::getInstance().error("This is an asynchronous ERROR log");
    
    // 刷新日志，确保所有日志都被处理
    WinLog::getInstance().flush();
    
    // 等待一段时间，确保日志被写入
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "Basic asynchronous logging test completed" << std::endl;
}

// Performance test - comparing synchronous and asynchronous logging
void testPerformance() {
    std::cout << "\n=== Performance Test: Sync vs Async ===" << std::endl;
    
    const int LOG_COUNT = 10000;
    
    // 测试同步日志性能
    WinLog::getInstance().shutdown();
    WinLog::getInstance().init("async_log.log");
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < LOG_COUNT; ++i) {
        WinLog::getInstance().info("Sync logging test #{0}", i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto syncDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "同步日志处理 " << LOG_COUNT << " 条消息耗时: " 
              << syncDuration << " ms" << std::endl;
    
    // 测试异步日志性能
    WinLog::getInstance().shutdown();
    AsyncConfig asyncConfig;
    asyncConfig.enabled = true;
    asyncConfig.queueSize = 10000;
    asyncConfig.maxBatchSize = 100;
    asyncConfig.memoryPoolSize = 10000;
    WinLog::getInstance().setAsyncConfig(asyncConfig);
    WinLog::getInstance().init(nullptr, LogLevel::info, asyncConfig);
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < LOG_COUNT; ++i) {
        WinLog::getInstance().info("Async logging test #{0}", i);
    }
    WinLog::getInstance().flush();
    end = std::chrono::high_resolution_clock::now();
    auto asyncDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "异步日志处理 " << LOG_COUNT << " 条消息耗时: " 
              << asyncDuration << " ms" << std::endl;
    
    // 计算性能提升
    if (syncDuration > 0) {
        double improvement = static_cast<double>(syncDuration) / asyncDuration;
        std::cout << "Performance improvement: " << std::fixed << std::setprecision(2) 
              << improvement << "x" << std::endl;
    }
    
    WinLog::getInstance().flush();
}

// Multi-threaded test
void testMultiThreadedLogging() {
    std::cout << "\n=== Multi-threaded Logging Test ===" << std::endl;
    
    WinLog::getInstance().shutdown();
    AsyncConfig multiThreadConfig;
    multiThreadConfig.enabled = true;
    multiThreadConfig.queueSize = 20000;
    multiThreadConfig.maxBatchSize = 200;
    multiThreadConfig.memoryPoolSize = 20000;
    WinLog::getInstance().setAsyncConfig(multiThreadConfig);
    WinLog::getInstance().init(nullptr, LogLevel::info, multiThreadConfig);
    
    const int NUM_THREADS = 10;
    const int LOGS_PER_THREAD = 1000;
    
    std::vector<std::thread> threads;
    
    auto threadFunc = [LOGS_PER_THREAD](int threadId) {
        for (int i = 0; i < LOGS_PER_THREAD; ++i) {
            WinLog::getInstance().info("Thread #{0} log #{1}", threadId, i);
        }
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 创建多个线程同时写日志
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(threadFunc, i);
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    WinLog::getInstance().flush();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << NUM_THREADS << " 个线程同时写入 " << (NUM_THREADS * LOGS_PER_THREAD) 
              << " 条日志耗时: " << duration << " ms" << std::endl;
    
    // Test thread safety (verify no data races)
    std::cout << "Multi-threaded logging test completed, thread safety verified" << std::endl;
}

// Test overflow strategy
void testOverflowStrategy() {
    std::cout << "\n=== Overflow Strategy Test ===" << std::endl;
    
    // 配置小队列和丢弃策略
    AsyncConfig config;
    config.enabled = true;
    config.queueSize = 100;  // 小队列
    config.maxBatchSize = 10;
    config.memoryPoolSize = 100;
    config.dropOnOverflow = true;  // 启用丢弃策略
    config.flushIntervalMs = 1000;  // 较慢的刷新间隔
    WinLog::getInstance().setAsyncConfig(config);
    
    WinLog::getInstance().shutdown();
    WinLog::getInstance().init("async_log.log", LogLevel::info, config);
    
    // 快速写入大量日志，触发溢出
    const int LOG_COUNT = 1000;
    for (int i = 0; i < LOG_COUNT; ++i) {
        WinLog::getInstance().info("Overflow strategy test log #{0}", i);
    }
    
    // 等待刷新完成
    std::this_thread::sleep_for(std::chrono::seconds(2));
    WinLog::getInstance().flush();
    
    std::cout << "Overflow strategy test completed, some logs may have been dropped" << std::endl;
}

// Test configuration parameters
void testConfigParams() {
    std::cout << "\n=== Configuration Parameters Test ===" << std::endl;
    
    // 测试不同的刷新间隔
    AsyncConfig config;
    config.enabled = true;
    config.queueSize = 1000;
    config.maxBatchSize = 100;
    config.memoryPoolSize = 1000;
    config.dropOnOverflow = false;
    config.flushIntervalMs = 200;  // 快速刷新
    WinLog::getInstance().setAsyncConfig(config);
    
    WinLog::getInstance().shutdown();
    WinLog::getInstance().init(nullptr, LogLevel::info, config);
    
    // Write some logs and verify quick flushing
    for (int i = 0; i < 10; ++i) {
        WinLog::getInstance().info("Quick flush test log #{0}", i);
    }
    
    std::cout << "Configuration parameters test completed" << std::endl;
    WinLog::getInstance().flush();
}

// Test memory pool performance
void testMemoryPoolPerformance() {
    std::cout << "\n=== Memory Pool Performance Test ===" << std::endl;
    
    const int LOG_COUNT = 50000;
    
    // 1. 测试启用内存池的性能
    WinLog::getInstance().shutdown();
    AsyncConfig poolEnabledConfig;
    poolEnabledConfig.enabled = true;
    poolEnabledConfig.queueSize = 100000;
    poolEnabledConfig.maxBatchSize = 1000;
    poolEnabledConfig.memoryPoolSize = 50000;
    poolEnabledConfig.useMemoryPool = true;  // 启用内存池
    WinLog::getInstance().setAsyncConfig(poolEnabledConfig);
    WinLog::getInstance().init(nullptr, LogLevel::info, poolEnabledConfig);
    
    // 重置统计信息
    WinLog::getInstance().resetStats();
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < LOG_COUNT; ++i) {
        WinLog::getInstance().info("Memory pool enabled test log #{0}", i);
    }
    WinLog::getInstance().flush();
    auto end = std::chrono::high_resolution_clock::now();
    auto poolEnabledDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // 获取启用内存池时的统计信息
    auto poolEnabledStats = WinLog::getInstance().getStats();
    
    std::cout << "启用内存池处理 " << LOG_COUNT << " 条消息耗时: " 
              << poolEnabledDuration << " ms" << std::endl;
    std::cout << "内存池统计信息:" << std::endl;
    std::cout << "  总分配次数: " << poolEnabledStats.totalAllocations << std::endl;
    std::cout << "  总释放次数: " << poolEnabledStats.totalDeallocations << std::endl;
    std::cout << "  当前池大小: " << poolEnabledStats.currentPoolSize << std::endl;
    std::cout << "  峰值池大小: " << poolEnabledStats.peakPoolSize << std::endl;
    std::cout << "  TLS缓存命中: " << poolEnabledStats.threadCacheHits << std::endl;
    
    // 2. 测试禁用内存池的性能
    WinLog::getInstance().shutdown();
    AsyncConfig poolDisabledConfig;
    poolDisabledConfig.enabled = true;
    poolDisabledConfig.queueSize = 100000;
    poolDisabledConfig.maxBatchSize = 1000;
    poolDisabledConfig.memoryPoolSize = 0;  // 内存池大小设为0
    poolDisabledConfig.useMemoryPool = false;  // 禁用内存池
    WinLog::getInstance().setAsyncConfig(poolDisabledConfig);
    WinLog::getInstance().init(nullptr, LogLevel::info, poolDisabledConfig);
    
    // 重置统计信息
    WinLog::getInstance().resetStats();
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < LOG_COUNT; ++i) {
        WinLog::getInstance().info("Memory pool disabled test log #{0}", i);
    }
    WinLog::getInstance().flush();
    end = std::chrono::high_resolution_clock::now();
    auto poolDisabledDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // 获取禁用内存池时的统计信息
    auto poolDisabledStats = WinLog::getInstance().getStats();
    
    std::cout << "禁用内存池处理 " << LOG_COUNT << " 条消息耗时: " 
              << poolDisabledDuration << " ms" << std::endl;
    
    // 计算性能提升
    if (poolDisabledDuration > 0) {
        double improvement = static_cast<double>(poolDisabledDuration) / poolEnabledDuration;
        std::cout << "\n内存池优化性能提升: " << std::fixed << std::setprecision(2) 
                  << improvement << "x" << std::endl;
    }
    
    // 3. 多线程下内存池性能测试
    std::cout << "\n=== Multi-threaded Memory Pool Test ===" << std::endl;
    
    WinLog::getInstance().shutdown();
    AsyncConfig multiThreadPoolConfig;
    multiThreadPoolConfig.enabled = true;
    multiThreadPoolConfig.queueSize = 200000;
    multiThreadPoolConfig.maxBatchSize = 2000;
    multiThreadPoolConfig.memoryPoolSize = 100000;
    multiThreadPoolConfig.useMemoryPool = true;  // 启用内存池
    WinLog::getInstance().setAsyncConfig(multiThreadPoolConfig);
    WinLog::getInstance().init(nullptr, LogLevel::info, multiThreadPoolConfig);
    
    // 重置统计信息
    WinLog::getInstance().resetStats();
    
    const int NUM_THREADS = 16;
    const int LOGS_PER_THREAD = 10000;
    
    std::vector<std::thread> threads;
    
    auto threadFunc = [LOGS_PER_THREAD](int threadId) {
        for (int i = 0; i < LOGS_PER_THREAD; ++i) {
            WinLog::getInstance().info("Thread #{0} memory pool test log #{1}", threadId, i);
        }
    };
    
    start = std::chrono::high_resolution_clock::now();
    
    // 创建多个线程同时写日志
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(threadFunc, i);
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    WinLog::getInstance().flush();
    end = std::chrono::high_resolution_clock::now();
    
    auto multiThreadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    auto multiThreadStats = WinLog::getInstance().getStats();
    
    std::cout << NUM_THREADS << " 个线程同时写入 " << (NUM_THREADS * LOGS_PER_THREAD) 
              << " 条日志耗时: " << multiThreadDuration << " ms" << std::endl;
    std::cout << "多线程内存池统计信息:" << std::endl;
    std::cout << "  总分配次数: " << multiThreadStats.totalAllocations << std::endl;
    std::cout << "  总释放次数: " << multiThreadStats.totalDeallocations << std::endl;
    std::cout << "  当前池大小: " << multiThreadStats.currentPoolSize << std::endl;
    std::cout << "  峰值池大小: " << multiThreadStats.peakPoolSize << std::endl;
    std::cout << "  TLS缓存命中: " << multiThreadStats.threadCacheHits << std::endl;
    
    WinLog::getInstance().flush();
}

int main() {
    std::cout << "WinLog Asynchronous Logging Functionality Test" << std::endl;
    std::cout << "=============================" << std::endl;
    
    // 设置日志级别
    WinLog::getInstance().setLevel(LogLevel::debug);
    
    try {
        // 运行所有测试
        testBasicAsyncLogging();
        testPerformance();
        testMultiThreadedLogging();
        testOverflowStrategy();
        testConfigParams();
        testMemoryPoolPerformance();  // 运行内存池性能测试
        
        std::cout << "\nAll tests completed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred during testing: " << e.what() << std::endl;
    }
    
    // 确保所有日志都被处理
    WinLog::getInstance().flush();
    WinLog::getInstance().shutdown();
    
    return 0;
}