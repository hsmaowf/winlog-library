#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include "../include/winlog.h"

// 模拟一个性能关键的函数，需要低延迟日志
void performCriticalOperation() {
    // 记录开始时间
    WinLog::getInstance().info("开始执行关键操作");
    
    // 模拟耗时操作
    for (int i = 0; i < 5; ++i) {
        // 使用异步日志记录中间状态，不会阻塞主线程
        WinLog::getInstance().debug("操作进度: {0}%", i * 20);
        
        // 实际业务逻辑...
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // 记录完成
    WinLog::getInstance().info("关键操作执行完成");
}

// 模拟多线程场景
void workerThread(int id) {
    for (int i = 0; i < 20; ++i) {
        WinLog::getInstance().info("工作线程 {0} 执行任务 {1}", id, i);
        
        // 随机模拟不同级别的日志
        if (i % 5 == 0) {
            WinLog::getInstance().warn("工作线程 {0} 遇到警告情况", id);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// 展示如何配置异步日志参数
void configureAsyncLogging() {
    // 创建异步配置
    AsyncConfig config;
    
    // 基本配置
    config.enabled = true;  // 启用异步模式
    
    // 队列和批处理配置
    config.queueSize = 5000;      // 队列最大容量
    config.maxBatchSize = 200;    // 每批处理的最大日志数
    config.memoryPoolSize = 5000; // 内存池大小
    
    // 溢出处理策略
    config.dropOnOverflow = true;  // 队列满时丢弃新日志
    
    // 自动刷新配置
    config.flushIntervalMs = 300;  // 自动刷新间隔(毫秒)
    
    // 性能优化选项
    config.useMemoryPool = true;    // 使用内存池
    config.optimizeForThroughput = true; // 优化吞吐量
    
    // 应用配置
    WinLog::getInstance().setAsyncConfig(config);
    
    // 初始化异步日志系统
    if (!WinLog::getInstance().init(nullptr, LogLevel::info, config)) {
        std::cerr << "异步日志初始化失败!" << std::endl;
    }
}

int main() {
    std::cout << "=== WinLog 异步日志示例 ===" << std::endl;
    
    // 1. 配置并初始化异步日志
    configureAsyncLogging();
    
    // 2. 记录一些基本日志
    WinLog::getInstance().info("异步日志系统已初始化");
    WinLog::getInstance().debug("详细调试信息");
    WinLog::getInstance().warn("系统警告信息");
    
    // 3. 运行性能关键的操作，展示低延迟特性
    std::cout << "执行性能关键操作..." << std::endl;
    performCriticalOperation();
    
    // 4. 模拟多线程环境
    std::cout << "启动多线程测试..." << std::endl;
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(workerThread, i);
    }
    
    // 主线程继续执行并记录日志
    for (int i = 0; i < 10; ++i) {
        WinLog::getInstance().info("主线程执行中: 步骤 {0}", i);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    // 5. 等待所有工作线程完成
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    // 6. 模拟错误场景
    WinLog::getInstance().error("模拟错误发生，测试错误日志记录");
    
    // 7. 程序结束前刷新所有日志
    std::cout << "程序即将结束，刷新所有待处理日志..." << std::endl;
    WinLog::getInstance().flush();
    
    // 8. 清理资源
    WinLog::getInstance().shutdown();
    
    std::cout << "示例程序执行完成" << std::endl;
    return 0;
}