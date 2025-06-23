@echo off
setlocal EnableDelayedExpansion

rem 矩阵乘法跨平台性能测试 - Windows 版本

echo 矩阵乘法跨平台性能测试
echo ==============================

rem 检测编译器
where g++.exe >nul 2>&1
if %errorlevel% equ 0 (
    set COMPILER=g++.exe
    echo 编译器: g++
) else (
    where clang++.exe >nul 2>&1
    if %errorlevel% equ 0 (
        set COMPILER=clang++.exe
        echo 编译器: clang++
    ) else (
        echo 错误: 未找到 C++ 编译器
        pause
        exit /b 1
    )
)

rem 系统信息
echo 操作系统: Windows
echo CPU 核心数: %NUMBER_OF_PROCESSORS%
echo.

rem 编译程序
if not exist program.exe (
    echo 正在编译程序...
    %COMPILER% -std=c++17 -O3 -march=native -mtune=native -pthread -static -Wall -Wextra -o program.exe MatrixMul.cpp
    if %errorlevel% neq 0 (
        echo 编译失败
        pause
        exit /b 1
    )
    echo 编译成功
    echo.
)

rem 运行测试
echo === 性能测试套件 ===
echo.

echo 1. 快速测试 (512x512)
program.exe -s 512 -i 3
echo.

echo 2. 标准测试 (1024x1024)
program.exe -s 1024 -i 3
echo.

echo 3. 扩展性测试
echo 单线程:
program.exe -s 1024 -t 1 -i 2
echo 多线程:
program.exe -s 1024 -i 2
echo.

if "%1"=="full" (
    echo 4. 大矩阵测试 (2048x2048)
    program.exe -s 2048 -i 1
    echo.

    echo 5. 块大小优化测试
    for %%s in (32 64 128 256) do (
        echo 块大小 %%s:
        program.exe -s 1024 -b %%s -i 1
    )
    echo.
)

echo 测试完成！
pause
