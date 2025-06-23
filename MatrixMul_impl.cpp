#include "MatrixMul.h"

/**
 * @brief 获取CPU缓存信息
 *
 * 跨平台获取CPU的L1、L2、L3缓存大小和缓存行大小信息。
 * 支持Windows、Linux和macOS系统, 采用不同的系统API实现:
 * - Windows: 使用GetLogicalProcessorInformation API
 * - Linux: 读取/sys/devices/system/cpu/目录下的缓存信息文件
 * - macOS: 使用sysctlbyname系统调用
 *
 * @return CacheInfo 包含缓存层次结构信息的结构体
 * @see CacheInfo
 */
CacheInfo get_cache_info()
{
  CacheInfo cache;

#ifdef _WIN32
  // Windows 实现
  DWORD buffer_size = 0;
  GetLogicalProcessorInformation(nullptr, &buffer_size);

  if (buffer_size > 0)
  {
    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(
        buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));

    if (GetLogicalProcessorInformation(buffer.data(), &buffer_size))
    {
      for (const auto &info : buffer)
      {
        if (info.Relationship == RelationCache)
        {
          switch (info.Cache.Level)
          {
            case 1:
              cache.l1_cache_size = info.Cache.Size;
              cache.line_size = info.Cache.LineSize;
              break;
            case 2:
              cache.l2_cache_size = info.Cache.Size;
              break;
            case 3:
              cache.l3_cache_size = info.Cache.Size;
              break;
          }
        }
      }
    }
  }

#elif defined(__linux__)
  // Linux 实现
  std::ifstream file;

  // 尝试读取L1缓存大小
  file.open("/sys/devices/system/cpu/cpu0/cache/index0/size");
  if (file.is_open())
  {
    std::string size_str;
    file >> size_str;
    file.close();

    // 解析大小 (例如 "32K" 或 "256K")
    if (!size_str.empty())
    {
      size_t multiplier = 1;
      if (size_str.back() == 'K')
      {
        multiplier = 1024;
        size_str.pop_back();
      }
      else if (size_str.back() == 'M')
      {
        multiplier = 1024 * 1024;
        size_str.pop_back();
      }
      cache.l1_cache_size = std::stoi(size_str) * multiplier;
    }
  }

  // 尝试读取L2缓存大小
  file.open("/sys/devices/system/cpu/cpu0/cache/index2/size");
  if (file.is_open())
  {
    std::string size_str;
    file >> size_str;
    file.close();

    if (!size_str.empty())
    {
      size_t multiplier = 1;
      if (size_str.back() == 'K')
      {
        multiplier = 1024;
        size_str.pop_back();
      }
      else if (size_str.back() == 'M')
      {
        multiplier = 1024 * 1024;
        size_str.pop_back();
      }
      cache.l2_cache_size = std::stoi(size_str) * multiplier;
    }
  }

  // 尝试读取缓存行大小
  file.open("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size");
  if (file.is_open())
  {
    file >> cache.line_size;
    file.close();
  }

#elif defined(__APPLE__)
  // macOS 实现
  size_t size = sizeof(size_t);

  // 获取L1缓存大小
  if (sysctlbyname("hw.l1dcachesize", &cache.l1_cache_size, &size, NULL, 0)
      != 0)
  {
    cache.l1_cache_size = 32768; // 默认32KB
  }

  // 获取L2缓存大小
  size = sizeof(size_t);
  if (sysctlbyname("hw.l2cachesize", &cache.l2_cache_size, &size, NULL, 0)
      != 0)
  {
    cache.l2_cache_size = 262144; // 默认256KB
  }

  // 获取L3缓存大小
  size = sizeof(size_t);
  if (sysctlbyname("hw.l3cachesize", &cache.l3_cache_size, &size, NULL, 0)
      != 0)
  {
    cache.l3_cache_size = 8388608; // 默认8MB
  }

  // 获取缓存行大小
  size = sizeof(size_t);
  if (sysctlbyname("hw.cachelinesize", &cache.line_size, &size, NULL, 0) != 0)
  {
    cache.line_size = 64; // 默认64字节
  }
#endif

  return cache;
}

/**
 * @brief 计算最优的矩阵分块大小
 *
 * 基于CPU缓存信息自动计算矩阵乘法的最优块大小。算法考虑以下因素：
 * 1. 使用L1缓存大小的1/3(因为矩阵乘法需要访问三个矩阵块)
 * 2. 根据int类型大小(4字节)计算可存储的元素数量
 * 3. 计算正方形矩阵块的边长
 * 4. 对齐到缓存行大小的倍数以优化内存访问
 * 5. 限制在合理范围内(32-512)
 *
 * @return size_t 计算得出的最优块大小
 * @see CacheInfo
 * @see get_cache_info()
 */
size_t calculate_optimal_block_size()
{
  CacheInfo cache = get_cache_info();

  // 使用L1缓存大小的一部分来计算块大小
  // 考虑到矩阵乘法需要访问三个矩阵块, 我们使用L1缓存的1/3
  size_t available_cache = cache.l1_cache_size / 3;

  // 每个元素是int类型(4字节), 计算可以存储多少个元素
  size_t elements_per_cache = available_cache / sizeof(int);

  // 计算正方形矩阵块的边长
  size_t block_size = static_cast<size_t>(sqrt(elements_per_cache));

  // 确保块大小是缓存行大小的倍数, 以优化内存访问
  size_t line_elements = cache.line_size / sizeof(int);
  block_size =
      ((block_size + line_elements - 1) / line_elements) * line_elements;

  // 限制块大小范围, 避免过小或过大
  if (block_size < 32) block_size = 32;
  if (block_size > 512) block_size = 512;

  return block_size;
}

/**
 * @brief 获取系统CPU核心数
 *
 * 跨平台获取系统的物理CPU核心数量。使用不同的系统API实现：
 * - Windows: 使用GetSystemInfo API获取处理器数量
 * - Linux: 使用get_nprocs()函数
 * - macOS: 使用sysctl系统调用, 优先获取可用CPU数, 回退到总CPU数
 * - 其他系统: 使用C++11标准的hardware_concurrency()
 *
 * @return size_t CPU核心数量, 用于确定最优线程数
 */
size_t get_cpu_cores()
{
#ifdef _WIN32
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  return sysinfo.dwNumberOfProcessors;
#elif defined(__linux__)
  return get_nprocs();
#elif defined(__APPLE__)
  int nm[2];
  size_t len = 4;
  uint32_t count;
  nm[0] = CTL_HW;
  nm[1] = HW_AVAILCPU;
  sysctl(nm, 2, &count, &len, NULL, 0);
  if (count < 1)
  {
    nm[1] = HW_NCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);
    if (count < 1)
    {
      count = 1;
    }
  }
  return count;
#else
  return std::thread::hardware_concurrency();
#endif
}

/**
 * @brief 打印详细的系统信息
 *
 * 显示当前系统的硬件和软件信息, 包括：
 * - CPU核心数和硬件并发数
 * - 操作系统类型(Windows/Linux/macOS)
 * - C++标准版本
 * - CPU架构(x86/x86_64/ARM/ARM64)
 * - 各级缓存大小(L1/L2/L3)
 * - 缓存行大小
 * - 自动计算的最优块大小
 *
 * 这些信息有助于理解性能测试结果和优化参数选择。
 *
 * @see get_cpu_cores()
 * @see get_cache_info()
 * @see calculate_optimal_block_size()
 */
void print_system_info()
{
  cout << "=== 系统信息 ===" << endl;
  cout << "CPU 核心数: " << get_cpu_cores() << endl;
  cout << "硬件并发数: " << std::thread::hardware_concurrency() << endl;

#ifdef _WIN32
  cout << "操作系统: Windows" << endl;
#elif defined(__linux__)
  cout << "操作系统: Linux" << endl;
#elif defined(__APPLE__)
  cout << "操作系统: macOS" << endl;
#else
  cout << "操作系统: 未知" << endl;
#endif

#ifdef __cplusplus
  cout << "C++ 标准: " << __cplusplus << endl;
#endif

  // 显示CPU架构信息
#if defined(__x86_64__) || defined(_M_X64)
  cout << "CPU 架构: x86_64" << endl;
#elif defined(__i386__) || defined(_M_IX86)
  cout << "CPU 架构: x86" << endl;
#elif defined(__aarch64__) || defined(_M_ARM64)
  cout << "CPU 架构: ARM64" << endl;
#elif defined(__arm__) || defined(_M_ARM)
  cout << "CPU 架构: ARM" << endl;
#else
  cout << "CPU 架构: 未知" << endl;
#endif

  // 显示缓存信息
  CacheInfo cache = get_cache_info();
  cout << "L1 缓存大小: " << (cache.l1_cache_size / 1024) << " KB" << endl;
  cout << "L2 缓存大小: " << (cache.l2_cache_size / 1024) << " KB" << endl;
  cout << "L3 缓存大小: " << (cache.l3_cache_size / 1024 / 1024) << " MB"
       << endl;
  cout << "缓存行大小: " << cache.line_size << " 字节" << endl;
  cout << "最优块大小: " << calculate_optimal_block_size() << endl;

  cout << "==================" << endl << endl;
}

/**
 * @brief 解析和处理命令行参数
 *
 * 解析程序的命令行参数并设置基准测试的各项配置。支持的参数包括：
 * - -s, --size: 矩阵大小
 * - -b, --block: 块大小(0表示自动计算)
 * - -t, --threads: 线程数(0表示自动检测)
 * - -i, --iterations: 迭代次数
 * - -v, --verbose: 详细输出模式
 * - -h, --help: 显示帮助信息
 *
 * 如果某些参数未指定或为0, 将自动使用系统检测的最优值。
 *
 * @param argc 命令行参数个数
 * @param argv 命令行参数字符串数组
 * @return BenchmarkConfig 解析后的配置结构体
 * @see BenchmarkConfig
 * @see get_cpu_cores()
 * @see calculate_optimal_block_size()
 */
BenchmarkConfig parse_args(int argc, char *argv[])
{
  BenchmarkConfig config;

  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--size") == 0)
    {
      if (i + 1 < argc)
      {
        config.matrix_size = static_cast<size_t>(atoi(argv[++i]));
      }
    }
    else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--block") == 0)
    {
      if (i + 1 < argc)
      {
        config.block_size = static_cast<size_t>(atoi(argv[++i]));
      }
    }
    else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--threads") == 0)
    {
      if (i + 1 < argc)
      {
        config.num_threads = static_cast<size_t>(atoi(argv[++i]));
      }
    }
    else if (strcmp(argv[i], "-i") == 0
             || strcmp(argv[i], "--iterations") == 0)
    {
      if (i + 1 < argc)
      {
        config.iterations = static_cast<size_t>(atoi(argv[++i]));
      }
    }
    else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
    {
      config.verbose = true;
    }
    else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
    {
      cout << "矩阵乘法性能测试程序" << endl;
      cout << "用法: " << argv[0] << " [选项]" << endl;
      cout << "选项:" << endl;
      cout << "  -s, --size <N>       矩阵大小 (默认: 1024)" << endl;
      cout << "  -b, --block <N>      块大小 (默认: 自动计算)" << endl;
      cout << "  -t, --threads <N>    线程数 (默认: 自动检测)" << endl;
      cout << "  -i, --iterations <N> 迭代次数 (默认: 1)" << endl;
      cout << "  -v, --verbose        详细输出" << endl;
      cout << "  -h, --help           显示帮助" << endl;
      exit(0);
    }
  }

  if (config.num_threads == 0)
  {
    config.num_threads = get_cpu_cores();
  }

  if (config.block_size == 0)
  {
    config.block_size = calculate_optimal_block_size();
  }

  return config;
}

/**
 * @brief 开始高精度计时
 *
 * 记录当前的高精度时间点作为计时的起始点。
 * 使用std::chrono::high_resolution_clock获得最高精度的时间测量。
 *
 * @see stop()
 * @see get_seconds()
 * @see get_microseconds()
 */
void Timer::start()
{
  start_time = std::chrono::high_resolution_clock::now();
}

/**
 * @brief 停止高精度计时
 *
 * 记录当前的高精度时间点作为计时的结束点。
 * 必须在调用start()之后调用, 用于计算时间间隔。
 *
 * @see start()
 * @see get_seconds()
 * @see get_microseconds()
 */
void Timer::stop()
{
  end_time = std::chrono::high_resolution_clock::now();
}

/**
 * @brief 获取经过的时间(秒)
 *
 * 计算从start()到stop()之间经过的时间, 以秒为单位。
 * 提供高精度的时间测量, 适用于性能基准测试。
 *
 * @return double 经过的时间, 单位为秒, 支持小数精度
 * @see start()
 * @see stop()
 * @see get_microseconds()
 */
double Timer::get_seconds() const
{
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      end_time - start_time);
  return static_cast<double>(duration.count()) / 1000000.0;
}

/**
 * @brief 获取经过的时间(微秒)
 *
 * 计算从start()到stop()之间经过的时间, 以微秒为单位。
 * 提供更高精度的时间测量, 适用于精确的性能分析。
 *
 * @return long long 经过的时间, 单位为微秒
 * @see start()
 * @see stop()
 * @see get_seconds()
 */
long long Timer::get_microseconds() const
{
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      end_time - start_time);
  return duration.count();
}

/**
 * @brief 分块矩阵乘法核心算法
 *
 * 实现高效的分块矩阵乘法算法, 支持指定行范围的计算。
 * 算法特点：
 * 1. 使用ikj循环顺序优化缓存访问模式
 * 2. 采用分块策略减少缓存miss
 * 3. 支持部分行范围计算, 便于并行处理
 * 4. 使用边界检查确保处理边缘情况
 *
 * 算法复杂度: O(n³), 其中n为矩阵大小
 *
 * @param src1 源矩阵1, 左操作数
 * @param src2 源矩阵2, 右操作数
 * @param dst 目标结果矩阵, 存储计算结果
 * @param blockSize 分块大小, 影响缓存效率
 * @param start 起始行索引(包含)
 * @param end 结束行索引(不包含)
 *
 * @pre src1, src2, dst必须是相同大小的方阵
 * @pre start < end <= src1.size()
 * @pre blockSize > 0
 *
 * @note 使用累加操作(+=), 调用前需要确保dst已正确初始化
 */
void matrix_mul(vector<vector<int>> &src1,
                vector<vector<int>> &src2,
                vector<vector<int>> &dst,
                size_t blockSize,
                size_t start,
                size_t end)
{
  // Perform matrix multiplication for the given block range using block ik
  // method
  for (size_t iblock = start; iblock < end; iblock += blockSize)
  {
    for (size_t kblock = 0; kblock < src2.size(); kblock += blockSize)
    {
      for (size_t jblock = 0; jblock < dst.size(); jblock += blockSize)
      {
        for (size_t i = iblock; i < min(iblock + blockSize, src1.size()); i++)
        {
          for (size_t k = kblock; k < min(kblock + blockSize, src2.size());
               k++)
          {
            for (size_t j = jblock; j < min(jblock + blockSize, dst.size());
                 j++)
            {
              dst[i][j] += src1[i][k] * src2[k][j];
            }
          }
        }
      }
    }
  }
}

/**
 * @brief 简单的多线程矩阵乘法实现
 *
 * 基于块大小创建线程的简单多线程实现。每个块创建一个独立线程,
 * 适用于中小规模矩阵或线程创建开销相对较小的场景。
 *
 * 特点：
 * - 线程数量等于矩阵大小除以块大小
 * - 每个线程处理一个连续的行块
 * - 所有线程并行执行, 最后统一等待完成
 *
 * @param matrix1 输入矩阵1(左操作数)
 * @param matrix2 输入矩阵2(右操作数)
 * @param result 结果矩阵, 存储计算结果
 * @param block_size 每个线程处理的行块大小
 *
 * @pre matrix1, matrix2, result必须是相同大小的方阵
 * @pre block_size > 0
 * @pre result已正确初始化为0
 *
 * @warning 线程数量可能很大, 适合CPU核心数较多的系统
 *
 * @see parallel_computing_optimized() 更优化的线程控制版本
 * @see matrix_mul() 底层矩阵乘法实现
 */
void parallel_computing_simple_multithread(
    std::vector<std::vector<int>> &matrix1,
    std::vector<std::vector<int>> &matrix2,
    std::vector<std::vector<int>> &result,
    size_t block_size)
{
  std::vector<std::thread> threads;

  for (size_t i = 0; i < matrix1.size(); i += block_size)
  {
    threads.push_back(std::thread(
        [&matrix1, &matrix2, &result, block_size, i]()
        {
          size_t end = std::min(i + block_size, matrix1.size());
          matrix_mul(matrix1, matrix2, result, block_size, i, end);
        }));
  }

  for (auto &t : threads)
  {
    t.join();
  }

  threads.clear();
}

/**
 * @brief 优化的多线程矩阵乘法实现
 *
 * 可控制线程数量的优化多线程实现。将矩阵行均匀分配给指定数量的线程,
 * 避免线程过多导致的上下文切换开销和资源竞争。
 *
 * 优化特点：
 * 1. 线程数量可控, 通常设置为CPU核心数
 * 2. 每个线程处理大致相等的行数, 负载均衡
 * 3. 减少线程创建和销毁开销
 * 4. 更好的CPU缓存利用率
 *
 * 算法流程：
 * 1. 计算每个线程应处理的行数
 * 2. 为每个线程分配连续的行范围
 * 3. 并行执行矩阵乘法计算
 * 4. 等待所有线程完成
 *
 * @param matrix1 输入矩阵1(左操作数)
 * @param matrix2 输入矩阵2(右操作数)
 * @param result 结果矩阵, 存储计算结果
 * @param block_size 分块大小, 用于缓存优化
 * @param num_threads 线程数量, 建议等于CPU核心数
 *
 * @pre matrix1, matrix2, result必须是相同大小的方阵
 * @pre block_size > 0
 * @pre num_threads > 0 && num_threads <= 系统最大线程数
 * @pre result已正确初始化为0
 *
 * @see parallel_computing_simple_multithread() 简单版本
 * @see matrix_mul() 底层矩阵乘法实现
 * @see get_cpu_cores() 获取推荐线程数
 */
void parallel_computing_optimized(std::vector<std::vector<int>> &matrix1,
                                  std::vector<std::vector<int>> &matrix2,
                                  std::vector<std::vector<int>> &result,
                                  size_t block_size,
                                  size_t num_threads)
{
  std::vector<std::thread> threads;
  size_t matrix_size = matrix1.size();
  size_t rows_per_thread = (matrix_size + num_threads - 1) / num_threads;

  for (size_t t = 0; t < num_threads; t++)
  {
    size_t start_row = t * rows_per_thread;
    if (start_row >= matrix_size) break;

    size_t end_row = std::min((t + 1) * rows_per_thread, matrix_size);

    threads.push_back(std::thread(
        [&matrix1, &matrix2, &result, block_size, start_row, end_row]()
        {
          matrix_mul(
              matrix1, matrix2, result, block_size, start_row, end_row);
        }));
  }

  for (auto &t : threads)
  {
    t.join();
  }
}
