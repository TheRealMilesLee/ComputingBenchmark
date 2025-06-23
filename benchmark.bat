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
if exist "Makefile.win" (
    echo 使用 Makefile 编译
    if exist "program-windows.exe" del "program-windows.exe"
    mingw32-make -f Makefile.win
    if exist "program-windows.exe" (
        set PROGRAM=program-windows.exe
        echo 编译成功: %PROGRAM%
    ) else (
        echo 编译失败
        pause
        exit /b 1
    )
) else (
    if not exist program.exe (
        echo 正在编译程序...
        %COMPILER% -O3 -pedantic-errors -Weverything -Wno-poison-system-directories -Wthread-safety -Wno-c++98-compat -std=c++23 -pthread -static -o program.exe MatrixMul.cpp
        if %errorlevel% neq 0 (
            echo 编译失败
            pause
            exit /b 1
        )
        echo 编译成功
        set PROGRAM=program.exe
    ) else (
        set PROGRAM=program.exe
    )
)
echo.

rem 运行测试
echo === 性能测试套件 ===
echo.

echo 1. 快速测试 (512x512)
%PROGRAM% -s 512 -i 3
echo.

echo 2. 标准测试 (1024x1024)
%PROGRAM% -s 1024 -i 3
echo.

echo 3. 扩展性测试
echo 单线程:
%PROGRAM% -s 1024 -t 1 -i 2
echo 多线程:
%PROGRAM% -s 1024 -i 2
echo.

if "%1"=="full" (
    echo 4. 大矩阵测试 (2048x2048)
    %PROGRAM% -s 2048 -i 1
    echo.

    echo 5. 块大小优化测试
    for %%s in (32 64 128 256) do (
        echo 块大小 %%s:
        %PROGRAM% -s 1024 -b %%s -i 1
    )
    echo.
)

echo 测试完成！
pause
