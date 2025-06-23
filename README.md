# 矩阵乘法性能测试程序

这是一个跨平台的多线程矩阵乘法性能测试程序，用于评估不同机器的性能表现。

## 特性

- 🚀 高性能的分块矩阵乘法算法
- 🧵 可配置的多线程并行计算
- 📊 详细的性能指标（GFLOPS、加速比、效率）
- 🔧 命令行参数配置
- 🌍 跨平台支持（Linux、macOS、Windows）
- ⚡ 编译器优化支持

## 编译

### 使用 Makefile（推荐）

不同平台会生成对应的可执行文件：
- **macOS**: `program-macos`
- **Linux**: `program-linux`
- **Windows**: `program-windows.exe`

```bash
# 编译发布版本
make

# 编译调试版本
make debug

# 显示编译信息
make info

# 查看所有可用选项
make help
```

#### Windows 系统
```bash
# 使用 Windows 专用 Makefile
mingw32-make -f Makefile.win

# 或使用批处理脚本
benchmark.bat
```

### 手动编译

#### Linux/macOS
```bash
# Linux
g++ -O3 -pedantic-errors -Weverything -Wno-poison-system-directories -Wthread-safety -Wno-c++98-compat -std=c++23 -pthread -o program-linux MatrixMul.cpp

# macOS
clang++ -O3 -pedantic-errors -Weverything -Wno-poison-system-directories -Wthread-safety -Wno-c++98-compat -std=c++23 -pthread -o program-macos MatrixMul.cpp
```

#### Windows (MinGW)
```bash
g++ -O3 -pedantic-errors -Weverything -Wno-poison-system-directories -Wthread-safety -Wno-c++98-compat -std=c++23 -pthread -static -o program-windows.exe MatrixMul.cpp
```

## 使用方法

### 基本用法
```bash
# macOS
./program-macos

# Linux
./program-linux

# Windows
program-windows.exe
```

### 带参数运行
```bash
# 指定矩阵大小为 2048x2048（以 macOS 为例）
./program-macos -s 2048

# 指定线程数为 8
./program-macos -t 8

# 指定块大小为 128
./program-macos -b 128

# 运行 5 次取平均值
./program-macos -i 5

# 详细输出
./program-macos -v

# 组合参数
./program-macos -s 1024 -t 4 -b 64 -i 3 -v
```

### 命令行参数

| 参数 | 长格式 | 描述 | 默认值 |
|------|--------|------|--------|
| `-s` | `--size` | 矩阵大小 (NxN) | 1024 |
| `-b` | `--block` | 分块大小 | 64 |
| `-t` | `--threads` | 线程数量 | 自动检测 |
| `-i` | `--iterations` | 迭代次数 | 1 |
| `-v` | `--verbose` | 详细输出 | 关闭 |
| `-h` | `--help` | 显示帮助 | - |

## 快速测试

### 使用 Makefile
```bash
# 运行快速测试
make test

# 运行完整性能测试
make benchmark

# 运行完整基准测试（包括大矩阵）
make benchmark-full

# 生成 HTML 报告
make report
```

### 使用自动化脚本
```bash
# Linux/macOS
./benchmark.sh          # 标准测试
./benchmark.sh full     # 完整测试

# Windows
benchmark.bat           # 标准测试
benchmark.bat full      # 完整测试
```

### 手动测试
```bash
# 小规模快速测试（以 macOS 为例）
./program-macos -s 512 -i 3 -v

# 标准测试
./program-macos -s 1024 -i 5

# 大规模测试
./program-macos -s 2048 -i 1
```

## 性能调优建议

### 最佳实践

1. **矩阵大小**: 建议使用 2 的幂次，如 512, 1024, 2048, 4096
2. **块大小**: 通常 64-128 效果较好，可以根据 CPU 缓存大小调整
3. **线程数**: 默认自动检测，也可以手动指定为 CPU 核心数
4. **迭代次数**: 建议至少 3 次以获得稳定结果

### 不同场景的推荐设置

```bash
# CPU 性能评估
./program -s 2048 -i 3

# 内存带宽测试
./program -s 4096 -b 32 -i 1

# 多核扩展性测试
./program -s 1024 -t 1 -i 3  # 单线程
./program -s 1024 -t 4 -i 3  # 4线程
./program -s 1024 -t 8 -i 3  # 8线程
```

## 输出说明

程序会输出以下信息：

- **系统信息**: CPU 核心数、操作系统等
- **测试配置**: 矩阵大小、线程数等
- **性能结果**:
  - 单线程/多线程平均时间
  - 加速比 (Speedup)
  - 并行效率 (Efficiency)
  - 性能指标 (GFLOPS)

## 兼容性

### 支持的编译器
- GCC 11.0+（支持 C++23）
- Clang 12.0+（支持 C++23）
- MSVC 2022+（支持 C++23）

### 支持的操作系统
- Linux (Ubuntu, CentOS, Arch, etc.)
- macOS 10.14+
- Windows 10+ (MinGW/MSYS2)

### C++ 标准
- C++23 或更高版本

## 故障排除

### 编译错误
```bash
# 检查编译器版本
g++ --version
clang++ --version

# 检查 C++23 支持
g++ -std=c++23 --version
clang++ -std=c++23 --version
```

### 运行时错误
```bash
# 检查系统资源
free -h         # Linux
vm_stat         # macOS
```

### 性能异常
- 确保系统负载较低
- 关闭不必要的后台程序
- 检查温度限制
- 尝试不同的块大小

## 贡献

欢迎提交 Issue 和 Pull Request！

## 许可证

MIT License
