cmake_minimum_required (VERSION 3.15)

project(ScopeChecker CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

find_package(Clang REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LLVM_CMAKE_DIR}")

include(AddLLVM)

configure_file(src/config.hpp.in ${PROJECT_BINARY_DIR}/config.hpp)
include_directories(${PROJECT_BINARY_DIR})
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
add_definitions(${CLANG_DEFINITIONS})

if(NOT LLVM_ENABLE_RTTI)
  add_definitions(-fno-rtti)
endif()

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PUBLIC)
target_link_libraries(${PROJECT_NAME}Standalone PUBLIC)

#add_executable(dummy test/test.cpp)
