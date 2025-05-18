# 设置cmake最低版本
cmake_minimum_required(VERSION 3.15)
# project命令用于指定cmake工程的名称
project(Daheng_Driver)
# 查找依赖包
find_package(OpenCV REQUIRED)
find_package(YAML-CPP REQUIRED)
# 将.cpp/.c/.cc文件生成可执行文件
add_executable(${PROJECT_NAME} Daheng_Driver.cpp)
# # 为target连接库的头文件路径（被link库生成CMakeLists.txt中定义的头文件路径）和对应定义的函数库路径
target_link_libraries(
    ${PROJECT_NAME} 
    ${OpenCV_LIBRARIES}
    gxiapi
    yaml-cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ./include)
# target_include_directories(text PUBLIC /home/may/Galaxy_camera/inc)
