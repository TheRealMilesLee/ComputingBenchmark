#include "MatrixMul.h"

/**
 * @brief 矩阵乘法性能基准测试主程序
 *
 * 这是一个全面的矩阵乘法性能测试程序, 具有以下特性：
 * - 自动检测系统硬件信息(CPU核心数、缓存大小等)
 * - 自动计算最优的矩阵分块大小
 * - 支持单线程和多线程性能对比
 * - 提供详细的性能指标分析(GFLOPS、加速比、效率等)
 * - 跨平台支持(Windows、Linux、macOS)
 * - 跨架构支持(x86、x86_64、ARM、ARM64)
 *
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return int 程序退出状态码, 0表示成功
 */
int main(int argc, char *argv[])
{
  // 解析命令行参数
  BenchmarkConfig config = parse_args(argc, argv);

  // 显示系统信息
  print_system_info();

  // 显示测试配置
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

  // 初始化矩阵
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

  // 初始化数据, 使用更好的模式来避免cache miss
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

  // 计算平均时间和性能指标
  double avg_single_time = total_single_time / config.iterations;
  double avg_multi_time = total_multi_time / config.iterations;
  double speedup = avg_single_time / avg_multi_time;
  double efficiency = speedup / config.num_threads;

  // 计算性能指标 (GFLOPS)
  double operations =
      2.0 * config.matrix_size * config.matrix_size * config.matrix_size;
  double gflops_single = operations / (avg_single_time * 1e9);
  double gflops_multi = operations / (avg_multi_time * 1e9);

  // 显示性能结果
  cout << endl << "=== 性能结果 ===" << endl;
  cout << fixed << setprecision(4);
  cout << "单线程平均时间: " << avg_single_time << " 秒" << endl;
  cout << "多线程平均时间: " << avg_multi_time << " 秒" << endl;
  cout << "加速比: " << speedup << "x" << endl;
  cout << "效率: " << (efficiency * 100) << "%" << endl;
  cout << "单线程性能: " << gflops_single << " GFLOPS" << endl;
  cout << "多线程性能: " << gflops_multi << " GFLOPS" << endl;
  cout << "==================" << endl;

  // 验证结果正确性(可选)
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
