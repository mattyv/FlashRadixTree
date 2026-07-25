cmake_minimum_required(VERSION 3.16)

project(FlashRadixTree VERSION 0.1.0 LANGUAGES CXX)

add_library(FlashRadixTree INTERFACE)
add_library(FlashRadixTree::FlashRadixTree ALIAS FlashRadixTree)
target_compile_features(FlashRadixTree INTERFACE cxx_std_20)
target_include_directories(FlashRadixTree INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

set(FLASH_RADIX_TREE_BUILD_TESTS_DEFAULT OFF)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(FLASH_RADIX_TREE_BUILD_TESTS_DEFAULT ON)
endif()
option(FLASH_RADIX_TREE_BUILD_TESTS "Build FlashRadixTree tests" ${FLASH_RADIX_TREE_BUILD_TESTS_DEFAULT})
if(FLASH_RADIX_TREE_BUILD_TESTS)
    enable_testing()
    add_executable(FlashRadixTreeTests tests.cpp)
    target_link_libraries(FlashRadixTreeTests PRIVATE FlashRadixTree)
    add_test(NAME FlashRadixTreeTests COMMAND FlashRadixTreeTests)

    add_executable(FlashRadixTreeSplayTests tests.cpp)
    target_link_libraries(FlashRadixTreeSplayTests PRIVATE FlashRadixTree)
    target_compile_definitions(FlashRadixTreeSplayTests PRIVATE USE_SPLAY_TREE=1)
    add_test(NAME FlashRadixTreeSplayTests COMMAND FlashRadixTreeSplayTests)
endif()

option(FLASH_RADIX_TREE_BUILD_BENCHMARKS "Build Apple Silicon counter benchmarks" OFF)
if(FLASH_RADIX_TREE_BUILD_BENCHMARKS)
    if(NOT APPLE OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/M1-Cycles/m1cycles.h")
        message(FATAL_ERROR "Benchmarks require macOS and the M1-Cycles submodule")
    endif()

    add_executable(FlashRadixTreeBenchmark main.cpp)
    target_link_libraries(FlashRadixTreeBenchmark PRIVATE FlashRadixTree)

    add_executable(FlashRadixTreeSplayBenchmark main.cpp)
    target_link_libraries(FlashRadixTreeSplayBenchmark PRIVATE FlashRadixTree)
    target_compile_definitions(FlashRadixTreeSplayBenchmark PRIVATE USE_SPLAY_TREE=1)
endif()

include(GNUInstallDirs)
install(TARGETS FlashRadixTree EXPORT FlashRadixTreeTargets)
install(FILES FlashRadixTree.hpp SplayTree.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT FlashRadixTreeTargets
    FILE FlashRadixTreeConfig.cmake
    NAMESPACE FlashRadixTree::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FlashRadixTree
)
