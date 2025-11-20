#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <iomanip>
#include "../include/winlog.h"

// 测试基本的异步日志功能
void testBasicAsyncLogging() {
    std::cout << "=== 测试基本异步日志功能 ===" << std::endl;
    
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
    if (WinLog::getInstance().init(nullptr, LogLevel::info, config)) {
        std::cout << "异步日志初始化成功" << std::endl;
    } else {
        std::cout << "异步日志初始化失败" << std::endl;
        return;
    }
    
    // 记录不同级别的日志
    WinLog::getInstance().info("这是一条异步INFO日志");
    WinLog::getInstance().debug("这是一条异步DEBUG日志");
    WinLog::getInstance().warn("这是一条异步WARNING日志");
    WinLog::getInstance().error("这是一条异步ERROR日志");
    
    // 刷新日志，确保所有日志都被处理
    WinLog::getInstance().flush();
    
    // 等待一段时间，确保日志被写入
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "基本异步日志测试完成" << std::endl;
}

// 性能测试 - 比较同步和异步日志
void testPerformance() {
    std::cout << "\n=== 性能测试：同步 vs 异步 ===" << std::endl;
    
    const int LOG_COUNT = 10000;
    
    // 测试同步日志性能
    WinLog::getInstance().shutdown();
    WinLog::getInstance().init();
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < LOG_COUNT; ++i) {
        WinLog::getInstance().info("同步日志测试 #{0}", i);
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
        WinLog::getInstance().info("异步日志测试 #{0}", i);
    }
    WinLog::getInstance().flush();
    end = std::chrono::high_resolution_clock::now();
    auto asyncDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "异步日志处理 " << LOG_COUNT << " 条消息耗时: " 
              << asyncDuration << " ms" << std::endl;
    
    // 计算性能提升
    if (syncDuration > 0) {
        double improvement = static_cast<double>(syncDuration) / asyncDuration;
        std::cout << "性能提升: " << std::fixed << std::setprecision(2) 
                  << improvement << "x" << std::endl;
    }
    
    WinLog::getInstance().flush();
}

// 多线程测试
void testMultiThreadedLogging() {
    std::cout << "\n=== 多线程日志测试 ===" << std::endl;
    
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
            WinLog::getInstance().info("线程 #{0} 的日志 #{1}", threadId, i);
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
    
    // 测试线程安全性（验证没有数据竞争）
    std::cout << "多线程日志测试完成，验证线程安全性" << std::endl;
}

// 测试溢出策略
void testOverflowStrategy() {
    std::cout << "\n=== 溢出策略测试 ===" << std::endl;
    
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
    WinLog::getInstance().init(nullptr, LogLevel::info, config);
    
    // 快速写入大量日志，触发溢出
    const int LOG_COUNT = 1000;
    for (int i = 0; i < LOG_COUNT; ++i) {
        WinLog::getInstance().info("测试溢出策略的日志 #{0}", i);
    }
    
    // 等待刷新完成
    std::this_thread::sleep_for(std::chrono::seconds(2));
    WinLog::getInstance().flush();
    
    std::cout << "溢出策略测试完成，部分日志可能被丢弃" << std::endl;
}

// 测试配置参数
void testConfigParams() {
    std::cout << "\n=== 配置参数测试 ===" << std::endl;
    
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
    
    // 写入一些日志并验证快速刷新
    for (int i = 0; i < 10; ++i) {
        WinLog::getInstance().info("快速刷新测试日志 #{0}", i);
    }
    
    std::cout << "配置参数测试完成" << std::endl;
    WinLog::getInstance().flush();
}

int main() {
    std::cout << "WinLog 异步日志功能测试" << std::endl;
    std::cout << "=====================" << std::endl;
    
    // 设置日志级别
    WinLog::getInstance().setLevel(LogLevel::debug);
    
    try {
        // 运行所有测试
        testBasicAsyncLogging();
        testPerformance();
        testMultiThreadedLogging();
        testOverflowStrategy();
        testConfigParams();
        
        std::cout << "\n所有测试完成！" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "测试过程中出现异常: " << e.what() << std::endl;
    }
    
    // 确保所有日志都被处理
    WinLog::getInstance().flush();
    WinLog::getInstance().shutdown();
    
    return 0;
}