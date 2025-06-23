#ifndef MATRIXMUL_H
#define MATRIXMUL_H

#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <cmath>

#ifdef _WIN32
#  include <windows.h>
#  include <intrin.h>
#elif defined(__linux__)
#  include <unistd.h>
#  include <sys/sysinfo.h>
#  include <fstream>
#elif defined(__APPLE__)
#  include <unistd.h>
#  include <sys/types.h>
#  include <sys/sysctl.h>
#endif

using namespace std;

/**
 * @brief CPU缓存信息结构体
 *
 * 存储CPU的各级缓存大小和缓存行大小信息, 用于优化矩阵乘法的块大小计算
 */
struct CacheInfo
{
  size_t l1_cache_size = 32768; ///< L1缓存大小, 默认32KB
  size_t l2_cache_size = 262144; ///< L2缓存大小, 默认256KB
  size_t l3_cache_size = 8388608; ///< L3缓存大小, 默认8MB
  size_t line_size = 64; ///< 缓存行大小, 默认64字节
};

/**
 * @brief 基准测试配置结构体
 *
 * 包含矩阵乘法性能测试的所有配置参数
 */
struct BenchmarkConfig
{
  size_t matrix_size = 1024; ///< 矩阵大小, 默认1024x1024
  size_t block_size = 0; ///< 块大小, 0表示自动计算
  size_t num_threads = 0; ///< 线程数, 0表示自动检测
  bool verbose = false; ///< 是否详细输出
  size_t iterations = 1; ///< 迭代次数, 默认1次
};

/**
 * @brief 高精度性能计时器类
 *
 * 提供微秒级精度的性能计时功能, 用于准确测量矩阵乘法的执行时间
 */
class Timer
{
private:
  std::chrono::high_resolution_clock::time_point start_time; ///< 开始时间点
  std::chrono::high_resolution_clock::time_point end_time; ///< 结束时间点

public:
  /**
   * @brief 开始计时
   *
   * 记录当前高精度时间点作为计时起始点
   */
  void start();

  /**
   * @brief 停止计时
   *
   * 记录当前高精度时间点作为计时结束点
   */
  void stop();

  /**
   * @brief 获取经过的时间（秒）
   *
   * @return 从开始到结束经过的时间, 单位为秒
   */
  double get_seconds() const;

  /**
   * @brief 获取经过的时间（微秒）
   *
   * @return 从开始到结束经过的时间, 单位为微秒
   */
  long long get_microseconds() const;
};

// 函数声明

/**
 * @brief 获取CPU缓存信息
 *
 * 跨平台获取CPU的L1、L2、L3缓存大小和缓存行大小信息
 * 支持Windows、Linux和macOS系统
 *
 * @return CacheInfo 包含缓存信息的结构体
 */
CacheInfo get_cache_info();

/**
 * @brief 计算最优块大小
 *
 * 基于CPU缓存信息自动计算矩阵乘法的最优块大小
 * 考虑L1缓存大小、缓存行大小等因素
 *
 * @return size_t 计算得出的最优块大小
 */
size_t calculate_optimal_block_size();

/**
 * @brief 获取CPU核心数
 *
 * 跨平台获取系统的CPU核心数, 支持Windows、Linux和macOS
 *
 * @return size_t CPU核心数
 */
size_t get_cpu_cores();

/**
 * @brief 打印系统信息
 *
 * 显示CPU核心数、操作系统、架构、缓存信息等系统详细信息
 */
void print_system_info();

/**
 * @brief 解析命令行参数
 *
 * 解析程序的命令行参数, 设置基准测试的各项配置
 * 支持矩阵大小、块大小、线程数、迭代次数等参数设置
 *
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return BenchmarkConfig 解析后的配置结构体
 */
BenchmarkConfig parse_args(int argc, char *argv[]);

/**
 * @brief 矩阵乘法核心函数
 *
 * 使用分块算法执行矩阵乘法运算, 支持指定行范围的并行计算
 * 采用ikj循环顺序优化缓存访问模式
 *
 * @param src1 源矩阵1
 * @param src2 源矩阵2
 * @param dst 目标结果矩阵
 * @param blockSize 分块大小
 * @param start 起始行索引
 * @param end 结束行索引
 */
void matrix_mul(vector<vector<int>> &src1,
                vector<vector<int>> &src2,
                vector<vector<int>> &dst,
                size_t blockSize,
                size_t start,
                size_t end);

/**
 * @brief 简单多线程矩阵乘法
 *
 * 基于块大小创建线程的简单多线程实现
 * 每个块创建一个线程, 适用于小规模矩阵
 *
 * @param matrix1 输入矩阵1
 * @param matrix2 输入矩阵2
 * @param result 结果矩阵
 * @param block_size 块大小
 */
void parallel_computing_simple_multithread(
    std::vector<std::vector<int>> &matrix1,
    std::vector<std::vector<int>> &matrix2,
    std::vector<std::vector<int>> &result,
    size_t block_size);

/**
 * @brief 优化的多线程矩阵乘法
 *
 * 可控制线程数量的优化多线程实现
 * 将矩阵行均匀分配给指定数量的线程, 避免线程过多的开销
 *
 * @param matrix1 输入矩阵1
 * @param matrix2 输入矩阵2
 * @param result 结果矩阵
 * @param block_size 块大小
 * @param num_threads 线程数量
 */
void parallel_computing_optimized(std::vector<std::vector<int>> &matrix1,
                                  std::vector<std::vector<int>> &matrix2,
                                  std::vector<std::vector<int>> &result,
                                  size_t block_size,
                                  size_t num_threads);

#endif // MATRIXMUL_H
