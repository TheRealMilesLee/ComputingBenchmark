#!/bin/bash

clang++ -std=c++17 -O3 -march=native -mtune=native -pthread -Wall -Wextra -o program-macos MatrixMul.cpp

./program-macos
