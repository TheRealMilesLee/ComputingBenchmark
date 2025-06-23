#!/bin/bash
# 跨平台性能测试脚本

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 检测操作系统
detect_os() {
  if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="Linux"
    EXE_SUFFIX=""
  elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macOS"
    EXE_SUFFIX=""
  elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    OS="Windows"
    EXE_SUFFIX=".exe"
  else
    OS="Unknown"
    EXE_SUFFIX=""
  fi
}

# 检测编译器
detect_compiler() {
  if command -v clang++ &>/dev/null; then
    COMPILER="clang++"
  elif command -v g++ &>/dev/null; then
    COMPILER="g++"
  else
    echo -e "${RED}错误: 未找到 C++ 编译器${NC}"
    exit 1
  fi
}

# 编译程序
compile_program() {
  echo -e "${BLUE}正在编译程序...${NC}"

  if [ -f "Makefile" ]; then
    echo "使用 Makefile 编译"
    make clean
    make
    # 查找对应平台的程序文件
    if [ "$OS" == "macOS" ] && [ -f "program-macos" ]; then
      PROGRAM="./program-macos"
    elif [ "$OS" == "Linux" ] && [ -f "program-linux" ]; then
      PROGRAM="./program-linux"
    elif [ "$OS" == "Windows" ] && [ -f "program-windows.exe" ]; then
      PROGRAM="./program-windows.exe"
    elif [ -f "program.exe" ]; then
      PROGRAM="./program.exe"
    elif [ -f "program" ]; then
      PROGRAM="./program"
    else
      echo -e "${RED}未找到编译后的程序文件${NC}"
      exit 1
    fi
  else
    echo "手动编译"
    $COMPILER -O3 -pedantic-errors -Weverything -Wno-poison-system-directories -Wthread-safety -Wno-c++98-compat -std=c++23 -pthread -o program$EXE_SUFFIX MatrixMul.cpp
    PROGRAM="./program$EXE_SUFFIX"
  fi

  if [ ! -f "$PROGRAM" ]; then
    echo -e "${RED}编译失败${NC}"
    exit 1
  fi

  echo -e "${GREEN}编译成功: $PROGRAM${NC}"
}

# 系统信息
print_system_info() {
  echo -e "${YELLOW}=== 系统环境 ===${NC}"
  echo "操作系统: $OS"
  echo "编译器: $COMPILER"
  echo "程序: $PROGRAM"

  if command -v nproc &>/dev/null; then
    echo "CPU 核心数: $(nproc)"
  elif command -v sysctl &>/dev/null; then
    echo "CPU 核心数: $(sysctl -n hw.ncpu)"
  fi

  echo ""
}

# 运行性能测试
run_benchmarks() {
  echo -e "${YELLOW}=== 性能测试套件 ===${NC}"

  echo -e "${BLUE}1. 快速测试 (512x512)${NC}"
  $PROGRAM -s 512 -i 3
  echo ""

  echo -e "${BLUE}2. 标准测试 (1024x1024)${NC}"
  $PROGRAM -s 1024 -i 3
  echo ""

  echo -e "${BLUE}3. 扩展性测试${NC}"
  echo "单线程:"
  $PROGRAM -s 1024 -t 1 -i 2
  echo "多线程:"
  $PROGRAM -s 1024 -i 2
  echo ""

  if [ "$1" == "full" ]; then
    echo -e "${BLUE}4. 大矩阵测试 (2048x2048)${NC}"
    $PROGRAM -s 2048 -i 1
    echo ""

    echo -e "${BLUE}5. 块大小优化测试${NC}"
    for block_size in 32 64 128 256; do
      echo "块大小 $block_size:"
      $PROGRAM -s 1024 -b $block_size -i 1
    done
    echo ""
  fi
}

# 主函数
main() {
  echo -e "${GREEN}矩阵乘法跨平台性能测试${NC}"
  echo "=============================="

  detect_os
  detect_compiler
  print_system_info

  # 检查是否需要编译
  if [ ! -f "program-macos" ] && [ ! -f "program-linux" ] && [ ! -f "program-windows.exe" ] && [ ! -f "program$EXE_SUFFIX" ]; then
    compile_program
  else
    # 查找已存在的程序文件
    if [ "$OS" == "macOS" ] && [ -f "program-macos" ]; then
      PROGRAM="./program-macos"
    elif [ "$OS" == "Linux" ] && [ -f "program-linux" ]; then
      PROGRAM="./program-linux"
    elif [ "$OS" == "Windows" ] && [ -f "program-windows.exe" ]; then
      PROGRAM="./program-windows.exe"
    elif [ -f "program.exe" ]; then
      PROGRAM="./program.exe"
    elif [ -f "program" ]; then
      PROGRAM="./program"
    else
      compile_program
    fi
  fi

  # 运行测试
  if [ "$1" == "full" ]; then
    run_benchmarks full
  else
    run_benchmarks
  fi

  echo -e "${GREEN}测试完成！${NC}"
}

# 帮助信息
show_help() {
  echo "用法: $0 [选项]"
  echo "选项:"
  echo "  full    运行完整测试套件"
  echo "  help    显示帮助信息"
  echo ""
  echo "示例:"
  echo "  $0          # 运行标准测试"
  echo "  $0 full     # 运行完整测试"
}

# 处理命令行参数
case "${1:-}" in
"help" | "-h" | "--help")
  show_help
  ;;
"full")
  main full
  ;;
"")
  main
  ;;
*)
  echo "未知选项: $1"
  show_help
  exit 1
  ;;
esac
