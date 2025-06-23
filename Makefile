# 跨平台矩阵乘法性能测试 Makefile

# 检测操作系统
UNAME_S := $(shell uname -s)

# 默认编译器设置
CXX := g++
CXXFLAGS := -O3 -pedantic-errors -Weverything -Wno-poison-system-directories -Wthread-safety -Wno-c++98-compat -std=c++23 -pthread
LDFLAGS :=
SOURCE := MatrixMul.cpp

# 根据操作系统设置目标文件名和编译选项
ifeq ($(UNAME_S),Linux)
    TARGET := program-linux
    PLATFORM := LINUX
endif

ifeq ($(UNAME_S),Darwin)
    TARGET := program-macos
    PLATFORM := MACOS
    # 检查是否有 clang++，macOS 优先使用 clang++
    ifeq ($(shell command -v clang++ 2> /dev/null),)
        CXX := g++
    else
        CXX := clang++
    endif
endif

# Windows (MSYS2/MinGW/Cygwin)
ifneq (,$(findstring MINGW,$(UNAME_S)))
    TARGET := program-windows.exe
    CXXFLAGS += -static
    PLATFORM := WINDOWS
endif

ifneq (,$(findstring MSYS,$(UNAME_S)))
    TARGET := program-windows.exe
    CXXFLAGS += -static
    PLATFORM := WINDOWS
endif

ifneq (,$(findstring CYGWIN,$(UNAME_S)))
    TARGET := program-windows.exe
    CXXFLAGS += -static
    PLATFORM := WINDOWS
endif

# 如果无法检测到操作系统，默认使用通用设置
ifndef TARGET
    TARGET := program
    PLATFORM := UNKNOWN
endif

# 添加平台定义
CXXFLAGS += -D$(PLATFORM)

# 颜色定义（用于输出）
GREEN := \033[0;32m
YELLOW := \033[1;33m
RED := \033[0;31m
NC := \033[0m # No Color

.PHONY: all clean info test benchmark help debug

# 默认目标
all: $(TARGET)

# 编译目标
$(TARGET): $(SOURCE)
	@echo "$(GREEN)正在编译 $(TARGET)...$(NC)"
	@echo "$(YELLOW)平台: $(PLATFORM)$(NC)"
	@echo "$(YELLOW)编译器: $(CXX)$(NC)"
	@echo "$(YELLOW)编译选项: $(CXXFLAGS)$(NC)"
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)
	@echo "$(GREEN)编译完成: $(TARGET)$(NC)"

# 调试版本
debug: CXXFLAGS := -g -O0 -pedantic-errors -Weverything -Wno-poison-system-directories -Wthread-safety -Wno-c++98-compat -std=c++23 -pthread -DDEBUG
debug: $(TARGET)

# 清理
clean:
	@echo "$(YELLOW)清理编译文件...$(NC)"
	rm -f program-* program.exe program *.o *.obj
	@echo "$(GREEN)清理完成$(NC)"

# 显示编译信息
info:
	@echo "=== 编译环境信息 ==="
	@echo "操作系统: $(UNAME_S)"
	@echo "目标文件: $(TARGET)"
	@echo "编译器: $(CXX)"
	@echo "编译选项: $(CXXFLAGS)"
	@echo "平台定义: $(PLATFORM)"
	@echo "源文件: $(SOURCE)"
	@echo "===================="

# 运行快速测试
test: $(TARGET)
	@echo "$(GREEN)运行快速测试...$(NC)"
	./$(TARGET) -s 512 -i 3 -v

# 运行性能基准测试
benchmark: $(TARGET)
	@echo "$(GREEN)运行性能基准测试...$(NC)"
	@echo "1. 快速测试 (512x512)"
	./$(TARGET) -s 512 -i 3
	@echo ""
	@echo "2. 标准测试 (1024x1024)"
	./$(TARGET) -s 1024 -i 3
	@echo ""
	@echo "3. 扩展性测试"
	@echo "单线程:"
	./$(TARGET) -s 1024 -t 1 -i 2
	@echo "多线程:"
	./$(TARGET) -s 1024 -i 2

# 运行完整基准测试
benchmark-full: $(TARGET)
	@echo "$(GREEN)运行完整基准测试...$(NC)"
	@echo "1. 快速测试 (512x512)"
	./$(TARGET) -s 512 -i 3
	@echo ""
	@echo "2. 标准测试 (1024x1024)"
	./$(TARGET) -s 1024 -i 3
	@echo ""
	@echo "3. 扩展性测试"
	@echo "单线程:"
	./$(TARGET) -s 1024 -t 1 -i 2
	@echo "多线程:"
	./$(TARGET) -s 1024 -i 2
	@echo ""
	@echo "4. 大矩阵测试 (2048x2048)"
	./$(TARGET) -s 2048 -i 1
	@echo ""
	@echo "5. 块大小优化测试"
	@for bs in 32 64 128 256; do \
		echo "块大小 $$bs:"; \
		./$(TARGET) -s 1024 -b $$bs -i 1; \
	done

# 生成 HTML 报告
report: $(TARGET)
	@if [ -f "report_generator.py" ]; then \
		echo "$(GREEN)生成性能报告...$(NC)"; \
		python3 report_generator.py full performance_report.html; \
	else \
		echo "$(RED)错误: 未找到 report_generator.py$(NC)"; \
	fi

# 帮助信息
help:
	@echo "可用的 make 目标:"
	@echo "  all              - 编译程序 (默认)"
	@echo "  debug            - 编译调试版本"
	@echo "  clean            - 清理编译文件"
	@echo "  info             - 显示编译环境信息"
	@echo "  test             - 运行快速测试"
	@echo "  benchmark        - 运行标准基准测试"
	@echo "  benchmark-full   - 运行完整基准测试"
	@echo "  report           - 生成 HTML 性能报告"
	@echo "  help             - 显示此帮助信息"
	@echo ""
	@echo "示例:"
	@echo "  make             # 编译程序"
	@echo "  make clean       # 清理文件"
	@echo "  make test        # 快速测试"
	@echo "  make benchmark   # 性能测试"
