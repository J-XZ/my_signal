cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
if(NOT DEFINED CMAKE_C_COMPILER)
  set(CMAKE_C_COMPILER clang)
endif()

if(NOT DEFINED CMAKE_CXX_COMPILER)
  set(CMAKE_CXX_COMPILER clang++)
endif()

project(my_signal)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(ENABLE_ASAN)
  set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
  set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
endif()

# 头文件目录
set(MY_SIGNAL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(my_signal SHARED my_signal.cc)
target_include_directories(my_signal PUBLIC
  $<BUILD_INTERFACE:${MY_SIGNAL_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:include>
)

# 安装库和头文件
install(TARGETS my_signal
  EXPORT my_signalTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)
install(DIRECTORY ${MY_SIGNAL_INCLUDE_DIR}/ DESTINATION include)

# 导出目标信息以供 find_package 使用
install(EXPORT my_signalTargets
  FILE my_signalConfig.cmake
  NAMESPACE my_signal::
  DESTINATION lib/cmake/my_signal
)

# 导出构建目录的目标信息（供未安装时使用）
export(EXPORT my_signalTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/my_signalConfig.cmake"
  NAMESPACE my_signal::
)
