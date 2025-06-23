#!/bin/bash

clang++ -O3 -pedantic-errors -Weverything -Wno-poison-system-directories -Wthread-safety -Wno-c++98-compat -std=c++23 -o program MatrixMul.cpp

./program
