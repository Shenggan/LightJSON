cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME LightJSON_Example)
project(${PROJECT_NAME})
set(CMAKE_CXX_FLAGS "--std=c++11 -g -Wall -O2")

# include_directories(${LightJSON_INCLUDE_PATH})
add_executable(c_style c_style.cc)
target_link_libraries(c_style lightjson)

add_executable(class_style class_style.cc)
target_link_libraries(class_style lightjson)