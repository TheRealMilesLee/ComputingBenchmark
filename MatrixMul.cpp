#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#  include <windows.h>
#elif defined(__linux__)
#  include <unistd.h>
#  include <sys/sysinfo.h>
#elif defined(__APPLE__)
#  include <unistd.h>
#  include <sys/types.h>
#  include <sys/sysctl.h>
#endif

using namespace std;

// 获取 CPU 核心数
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

// 打印系统信息
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

  cout << "==================" << endl << endl;
}

// 解析命令行参数
struct BenchmarkConfig
{
  size_t matrix_size = 1024;
  size_t block_size = 64;
  size_t num_threads = 0; // 0 表示自动检测
  bool verbose = false;
  size_t iterations = 1;
};

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
      cout << "  -b, --block <N>      块大小 (默认: 64)" << endl;
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

  return config;
}

// 性能计时器类
class Timer
{
private:
  std::chrono::high_resolution_clock::time_point start_time;
  std::chrono::high_resolution_clock::time_point end_time;

public:
  void start()
  {
    start_time = std::chrono::high_resolution_clock::now();
  }

  void stop()
  {
    end_time = std::chrono::high_resolution_clock::now();
  }

  double get_seconds() const
  {
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);
    return static_cast<double>(duration.count()) / 1000000.0;
  }

  long long get_microseconds() const
  {
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);
    return duration.count();
  }
};

void matrix_mul(vector<vector<int>> &src1,
                vector<vector<int>> &src2,
                vector<vector<int>> &dst,
                size_t blockSize,
                size_t start,
                size_t end);

void parallel_computing_simple_multithread(
    std::vector<std::vector<int>> &matrix1,
    std::vector<std::vector<int>> &matrix2,
    std::vector<std::vector<int>> &result,
    size_t block_size);

void parallel_computing_optimized(std::vector<std::vector<int>> &matrix1,
                                  std::vector<std::vector<int>> &matrix2,
                                  std::vector<std::vector<int>> &result,
                                  size_t block_size,
                                  size_t num_threads);
int main(int argc, char *argv[])
{
  BenchmarkConfig config = parse_args(argc, argv);

  print_system_info();

  cout << "=== 测试配置 ===" << endl;
  cout << "矩阵大小: " << config.matrix_size << "x" << config.matrix_size
       << endl;
  cout << "块大小: " << config.block_size << endl;
  cout << "线程数: " << config.num_threads << endl;
  cout << "迭代次数: " << config.iterations << endl;
  cout << "内存使用量约: " << fixed << setprecision(2)
       << (3.0 * config.matrix_size * config.matrix_size * sizeof(int))
          / (1024.0 * 1024.0)
       << " MB" << endl;
  cout << "==================" << endl << endl;

  // Initialize matrices
  if (config.verbose)
  {
    cout << "初始化矩阵..." << endl;
  }

  vector<vector<int>> src1(config.matrix_size,
                           vector<int>(config.matrix_size));
  vector<vector<int>> src2(config.matrix_size,
                           vector<int>(config.matrix_size));
  vector<vector<int>> dst_single(config.matrix_size,
                                 vector<int>(config.matrix_size, 0));
  vector<vector<int>> dst_multi(config.matrix_size,
                                vector<int>(config.matrix_size, 0));

  // 初始化数据，使用更好的模式来避免cache miss
  for (size_t row = 0; row < config.matrix_size; row++)
  {
    for (size_t col = 0; col < config.matrix_size; col++)
    {
      src1[row][col] = static_cast<int>((row * 31 + col * 17) % 100);
      src2[row][col] = static_cast<int>((row * 17 + col * 31) % 100);
    }
  }

  Timer timer;
  double total_single_time = 0.0;
  double total_multi_time = 0.0;

  cout << "开始性能测试..." << endl;

  // 运行多次迭代取平均值
  for (size_t iter = 0; iter < config.iterations; iter++)
  {
    if (config.verbose && config.iterations > 1)
    {
      cout << "迭代 " << (iter + 1) << "/" << config.iterations << endl;
    }

    // 重置结果矩阵
    for (size_t i = 0; i < config.matrix_size; i++)
    {
      fill(dst_single[i].begin(), dst_single[i].end(), 0);
      fill(dst_multi[i].begin(), dst_multi[i].end(), 0);
    }

    // 单线程测试
    timer.start();
    matrix_mul(src1, src2, dst_single, config.block_size, 0, src1.size());
    timer.stop();
    total_single_time += timer.get_seconds();

    if (config.verbose)
    {
      cout << "  单线程时间: " << fixed << setprecision(4)
           << timer.get_seconds() << " 秒" << endl;
    }

    // 多线程测试
    timer.start();
    parallel_computing_optimized(
        src1, src2, dst_multi, config.block_size, config.num_threads);
    timer.stop();
    total_multi_time += timer.get_seconds();

    if (config.verbose)
    {
      cout << "  多线程时间: " << fixed << setprecision(4)
           << timer.get_seconds() << " 秒" << endl;
    }
  }

  // 计算平均时间
  double avg_single_time = total_single_time / config.iterations;
  double avg_multi_time = total_multi_time / config.iterations;
  double speedup = avg_single_time / avg_multi_time;
  double efficiency = speedup / config.num_threads;

  // 计算性能指标 (GFLOPS)
  double operations =
      2.0 * config.matrix_size * config.matrix_size * config.matrix_size;
  double gflops_single = operations / (avg_single_time * 1e9);
  double gflops_multi = operations / (avg_multi_time * 1e9);

  cout << endl << "=== 性能结果 ===" << endl;
  cout << fixed << setprecision(4);
  cout << "单线程平均时间: " << avg_single_time << " 秒" << endl;
  cout << "多线程平均时间: " << avg_multi_time << " 秒" << endl;
  cout << "加速比: " << speedup << "x" << endl;
  cout << "效率: " << (efficiency * 100) << "%" << endl;
  cout << "单线程性能: " << gflops_single << " GFLOPS" << endl;
  cout << "多线程性能: " << gflops_multi << " GFLOPS" << endl;
  cout << "==================" << endl;

  // 验证结果正确性（可选）
  if (config.verbose)
  {
    cout << "验证结果正确性..." << endl;
    bool correct = true;
    for (size_t i = 0; i < min(size_t(10), config.matrix_size) && correct;
         i++)
    {
      for (size_t j = 0; j < min(size_t(10), config.matrix_size) && correct;
           j++)
      {
        if (dst_single[i][j] != dst_multi[i][j])
        {
          correct = false;
        }
      }
    }
    cout << "结果验证: " << (correct ? "通过" : "失败") << endl;
  }

  return 0;
}

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

// 优化的多线程实现，可以控制线程数量
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
