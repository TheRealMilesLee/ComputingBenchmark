# 项目架构说明

本项目已重构为模块化设计，提高了代码的可维护性和可读性。

## 文件结构

```
ComputingBenchmark/
├── MatrixMul.h           # 头文件 - 包含所有声明和接口
├── MatrixMul_impl.cpp    # 实现文件 - 包含所有函数实现
├── MatrixMul.cpp         # 主程序文件 - 只包含main函数
├── Makefile             # 构建文件 - 支持多文件编译
└── PROJECT_STRUCTURE.md # 本文档
```

## 模块说明

### 1. MatrixMul.h (头文件)
- **CacheInfo 结构体**: 存储CPU缓存信息
- **BenchmarkConfig 结构体**: 基准测试配置参数
- **Timer 类**: 高精度性能计时器
- **函数声明**: 所有公共函数的声明，包含详细的JavaDoc注释

### 2. MatrixMul_impl.cpp (实现文件)
包含所有函数的具体实现：
- `get_cache_info()`: 跨平台获取CPU缓存信息
- `calculate_optimal_block_size()`: 自动计算最优块大小
- `get_cpu_cores()`: 获取CPU核心数
- `print_system_info()`: 打印系统信息
- `parse_args()`: 命令行参数解析
- `Timer` 类方法实现
- `matrix_mul()`: 矩阵乘法核心算法
- `parallel_computing_*()`: 多线程实现

### 3. MatrixMul.cpp (主程序)
- 只包含 `main()` 函数
- 程序入口点和主要流程控制
- 包含详细的程序说明文档

## 优势

### 1. 可维护性
- **职责分离**: 每个文件有明确的职责
- **模块化**: 功能按模块组织，便于维护和扩展
- **接口清晰**: 头文件定义了清晰的API接口

### 2. 可读性
- **JavaDoc注释**: 每个函数、类、结构体都有详细的文档注释
- **代码结构**: 逻辑清晰，易于理解和学习

### 3. 可扩展性
- **接口稳定**: 头文件定义了稳定的接口
- **实现独立**: 可以独立修改实现而不影响其他模块
- **新功能添加**: 容易添加新的算法或优化

### 4. 编译效率
- **增量编译**: 只有修改的文件需要重新编译
- **依赖管理**: Makefile自动处理文件依赖关系

## 文档注释规范

项目采用JavaDoc风格的注释：

```cpp
/**
 * @brief 函数简要描述
 *
 * 详细描述函数的功能、用途和实现细节
 *
 * @param param1 参数1的描述
 * @param param2 参数2的描述
 * @return 返回值的描述
 */
```

## 编译说明

使用更新后的Makefile进行编译：

```bash
# 编译项目
make

# 调试版本
make debug

# 清理编译文件
make clean

# 查看编译信息
make info

# 运行测试
make test
```

## 平台支持

- **操作系统**: Windows, Linux, macOS
- **架构**: x86, x86_64, ARM, ARM64
- **编译器**: GCC, Clang, MSVC (通过MinGW)
