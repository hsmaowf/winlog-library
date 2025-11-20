# WinLog - Windows 日志库

WinLog 是一个轻量级、高性能的 Windows 平台日志库，提供线程安全的日志记录功能，支持多种日志级别和灵活的输出配置。

## 🎯 项目特点

- **跨语言支持**：支持 C++ 类和 C 风格全局函数两种接口
- **线程安全**：内部使用互斥锁确保多线程环境下的安全操作
- **多级别日志**：支持 TRACE、DEBUG、INFO、WARN、ERROR、CRITICAL 六个日志级别
- **灵活输出**：同时支持文件输出和控制台输出
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