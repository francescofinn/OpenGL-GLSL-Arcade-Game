cmake_minimum_required(VERSION 3.22)
project(lab10cpp)

set(CMAKE_CXX_STANDARD 11)

add_executable(lab10cpp main.cpp
        glad.c)

target_link_directories(lab10cpp PUBLIC "C:/tools/msys64/mingw64/include")
target_link_libraries(lab10cpp -g -L"C:/tools/msys64/mingw64/lib" -lGL -lglut -lglfw -lX11 -lpthread -lXi -lXrandr -ldl)