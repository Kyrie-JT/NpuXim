cmake_minimum_required(VERSION 3.10)

# 项目名称
project(mini-simulator)

# 设置 C++ 标准 (这里使用 C++17)
set(CMAKE_CXX_STANDARD 17)

# 添加可执行文件：生成的程序名 源文件
add_executable(mini-simulator main.cpp Core.cc Simulator.cc)