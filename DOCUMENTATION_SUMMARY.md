# WinLog 文档总览

## 📚 文档结构

WinLog 项目包含完整的开发文档，涵盖从项目介绍到高级使用的各个方面。

### 📖 核心文档

| 文档 | 描述 | 目标读者 |
|------|------|----------|
| [README.md](README.md) | 项目概述和快速开始 | 所有用户 |
| [API_REFERENCE.md](API_REFERENCE.md) | 完整的 API 参考文档 | 开发人员 |
| [USAGE_GUIDE.md](USAGE_GUIDE.md) | 详细使用指南和最佳实践 | 开发人员 |
| [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) | 构建和编译说明 | 构建工程师 |

### 🎯 文档导航

#### 🚀 新手入门
1. 阅读 [README.md](README.md) 了解项目基本信息
2. 运行示例程序体验功能
3. 查看 [USAGE_GUIDE.md](USAGE_GUIDE.md) 学习基本使用

#### 🔧 开发集成
1. 阅读 [API_REFERENCE.md](API_REFERENCE.md) 了解所有接口
2. 参考 [USAGE_GUIDE.md](USAGE_GUIDE.md) 的最佳实践
3. 按照 [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) 构建项目

#### 📋 项目维护
1. 查看 [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) 的自动化构建
2. 了解版本管理和发布流程
3. 参考文档更新和维护指南

## 📊 项目概览

### 项目特点
- ✅ **线程安全**：多线程环境下的安全日志记录
- ✅ **双接口设计**：支持 C++ 类和 C 风格全局函数
- ✅ **多级别支持**：TRACE、DEBUG、INFO、WARN、ERROR、CRITICAL
- ✅ **灵活输出**：同时支持文件和控制台输出
- ✅ **版本管理**：完整的版本信息接口
- ✅ **UTF-8 支持**：完全支持中文和 Unicode

### 技术规格
- **平台**：Windows 7+
- **编译器**：MinGW-w64 / Visual Studio
- **标准**：C++11+
- **架构**：x86_64
- **版本**：1.0.0.1

## 🗂️ 文件结构

```
WinLog/
├── 📄 文档文件/
│   ├── README.md              # 项目概述
│   ├── API_REFERENCE.md       # API 参考
│   ├── USAGE_GUIDE.md         # 使用指南
│   ├── BUILD_INSTRUCTIONS.md  # 构建说明
│   └── DOCUMENTATION_SUMMARY.md # 本文档
├── 📝 源代码/
│   ├── include/winlog.h       # 头文件
│   └── src/winlog.cpp         # 实现文件
├── 🧪 示例程序/
│   ├── examples/example.cpp   # 完整示例
│   └── examples/example.exe   # 示例可执行文件
├── 📦 构建产物/
│   ├── bin/WinLog.dll         # 动态链接库
│   ├── bin/LogExample.exe     # 日志示例
│   └── lib/WinLog.lib         # 导入库
└── ⚙️ 工具脚本/
    ├── run_with_dependencies.bat # 自动运行脚本
    └── run_example.bat          # 示例运行脚本
```

## 🚀 快速体验

### 1. 运行示例程序
```cmd
# 使用自动依赖脚本
run_with_dependencies.bat

# 或者直接运行
run_example.bat
```

### 2. 查看输出
示例程序会生成 `application.log` 文件，包含所有日志输出。

### 3. 集成到项目
```cpp
#include "winlog.h"

// 初始化
WinLog::getInstance().init("myapp.log", LogLevel::info);

// 记录日志
logInfo("程序启动成功");
logError("发生错误: %s", errorMsg);

// 关闭
WinLog::getInstance().shutdown();
```

## 📋 使用场景

### ✅ 适用场景
- Windows 桌面应用程序
- 服务端后台程序
- 调试和开发工具
- 日志分析和监控
- 多线程应用程序

### ⚠️ 注意事项
- 仅支持 Windows 平台
- 需要 C++11 或更高版本
- 文件路径需要适当权限
- 高并发场景注意性能影响

## 🔗 相关资源

### 内部链接
- [项目概述](README.md) - 了解项目基本信息
- [API 参考](API_REFERENCE.md) - 查看完整接口文档
- [使用指南](USAGE_GUIDE.md) - 学习详细使用方法
- [构建说明](BUILD_INSTRUCTIONS.md) - 掌握构建和编译

### 外部资源
- [MinGW-w64 官网](https://www.mingw-w64.org/)
- [CMake 文档](https://cmake.org/documentation/)
- [Windows DLL 编程](https://docs.microsoft.com/en-us/windows/win32/dlls/dlls)

## 📈 文档维护

### 更新频率
- 主要版本发布时更新所有文档
- 功能变更时更新相关章节
- 定期检查和修正错误

### 贡献指南
- 发现文档错误请提交 Issue
- 欢迎改进文档内容和结构
- 提供使用反馈和建议

### 版本历史
- **v1.0.0** (2024-01) - 初始版本，包含完整文档

---

## 💡 使用建议

1. **新手用户**：从 README.md 开始，逐步深入了解
2. **开发人员**：重点阅读 API_REFERENCE.md 和 USAGE_GUIDE.md
3. **构建工程师**：主要参考 BUILD_INSTRUCTIONS.md
4. **项目维护者**：定期更新文档，确保信息准确性

**📞 支持**：如有文档相关问题，欢迎提交反馈和建议！