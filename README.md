# WinLog - Windows 日志库

WinLog 是一个轻量级、高性能的 Windows 平台日志库，提供线程安全的日志记录功能，支持多种日志级别和灵活的输出配置。

## 🎯 项目特点

- **跨语言支持**：支持 C++ 类和 C 风格全局函数两种接口
- **线程安全**：内部使用互斥锁确保多线程环境下的安全操作
- **多级别日志**：支持 TRACE、DEBUG、INFO、WARN、ERROR、CRITICAL 六个日志级别
- **灵活输出**：同时支持文件输出和控制台输出
- **异步日志**：支持高性能异步日志记录模式
- **高级内存池**：优化的线程本地缓存内存池，大幅提升多线程环境下的性能
- **版本管理**：提供完整的版本信息接口
- **单例模式**：确保日志库全局唯一实例
- **UTF-8 支持**：完全支持中文和 Unicode 字符

## 📁 项目结构

```
WinLog/
├── include/
│   └── winlog.h              # 头文件 - API 接口定义
├── src/
│   └── winlog.cpp            # 源代码 - 日志库实现
├── examples/
│   ├── example.cpp           # 完整功能示例
│   └── example.exe           # 示例可执行文件
├── bin/
│   ├── WinLog.dll            # 动态链接库
│   └── LogExample.exe        # 日志示例程序
├── lib/
│   └── WinLog.lib            # 导入库
├── run_with_dependencies.bat # 自动运行脚本（带依赖）
├── run_example.bat           # 示例运行脚本
├── BUILD_GUIDE.md            # 构建指南
└── README.md                 # 本文档
```

## 🚀 快速开始

### 1. 运行示例程序

```bash
# 使用自动依赖脚本运行
run_with_dependencies.bat

# 或者直接运行示例
run_example.bat
```

### 2. 基本使用示例

```cpp
#include "winlog.h"

int main() {
    // 初始化日志库
    WinLog::getInstance().init("application.log", LogLevel::info);
    
    // 使用类方法记录日志
    WinLog::getInstance().info("程序启动成功");
    WinLog::getInstance().warn("这是一个警告消息");
    WinLog::getInstance().error("发生错误: %s", errorMessage);
    
    // 使用全局函数记录日志
    logInfo("使用全局函数记录信息");
    logError("使用全局函数记录错误");
    
    // 设置日志级别
    WinLog::getInstance().setLevel(LogLevel::warn);
    
    // 关闭日志库
    WinLog::getInstance().shutdown();
    
    return 0;
}
```

## 📋 日志级别

| 级别 | 数值 | 描述 |
|------|------|------|
| TRACE | 0 | 最详细的跟踪信息 |
| DEBUG | 1 | 调试信息 |
| INFO | 2 | 一般信息 |
| WARN | 3 | 警告信息 |
| ERROR | 4 | 错误信息 |
| CRITICAL | 5 | 严重错误信息 |
| OFF | 6 | 关闭所有日志 |

## 🔧 版本信息

- **当前版本**：1.0.0.1
- **版本号**：1.0.0.1
- **版本数字**：0x01000001

## 🎯 内存池优化特性

WinLog 实现了高性能的内存池系统，专为异步日志场景优化，主要特性包括：

- **线程本地缓存 (TLS)**：每个线程维护独立的对象缓存，减少线程间竞争
- **批量分配/释放**：支持批量操作，减少锁竞争频率
- **无锁原子统计**：使用原子操作跟踪内存使用情况，无性能开销
- **动态扩缩容**：根据实际需求自动调整内存池大小
- **对象复用**：高效复用已分配对象，减少内存碎片
- **性能统计监控**：实时跟踪内存分配、复用情况，提供详细的性能指标

### 内存池配置选项

```cpp
AsyncConfig config;
config.enabled = true;            // 启用异步日志
config.queueSize = 100000;        // 队列大小
config.maxBatchSize = 1000;       // 最大批处理大小
config.memoryPoolSize = 50000;    // 内存池初始大小
config.useMemoryPool = true;      // 启用内存池
config.dropOnOverflow = false;    // 队列溢出策略
config.flushIntervalMs = 500;     // 刷新间隔（毫秒）
```

### 性能统计功能

WinLog 提供了详细的性能统计功能，可以实时监控内存池的使用情况：

```cpp
// 重置统计数据
WinLog::getInstance().resetStats();

// 获取当前统计信息
WinLog::Stats stats = WinLog::getInstance().getStats();

// 使用统计数据
printf("分配请求: %llu\n", stats.allocationRequests);
printf("释放请求: %llu\n", stats.deallocationRequests);
printf("对象复用: %llu\n", stats.objectReuseCount);
printf("内存池大小: %llu\n", stats.currentPoolSize);
printf("内存节省: %.2f%%\n", stats.memorySavingsPercent);
```

### 性能提升

- **单线程性能**：启用内存池后，日志处理速度提升约 1.5-2x
- **多线程性能**：在多线程环境下，性能提升更加显著，可达 3-5x
- **内存使用**：减少约 40-60% 的内存分配/释放操作
- **系统开销**：降低 GC 压力和系统调用频率
- **可观测性**：通过统计数据实时监控系统运行状态

## 📄 许可证

本项目采用 MIT 许可证，详见 LICENSE 文件。

## 🤝 贡献

欢迎提交 Issue 和 Pull Request 来改进这个项目。

## 📞 联系方式

如有问题或建议，请通过以下方式联系：
- 提交 Issue
- 发送邮件

---

**WinLog** - 让 Windows 日志记录变得简单高效！