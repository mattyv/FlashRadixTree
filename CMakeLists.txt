cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(MyProject VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Add the executable without USE_SPLAY_TREE defined
add_executable(FlashRadixTree main.cpp)
# Add another executable with USE_SPLAY_TREE defined
add_executable(FlashRadixTreeWithSplay main.cpp) # Use the same main.cpp file
target_compile_definitions(FlashRadixTreeWithSplay PRIVATE USE_SPLAY_TREE=1)

# Set compiler flags for Release build (including -O3 optimization)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Include header files if needed
#include_directories(${PROJECT_SOURCE_DIR}/M1-Cycles)

# Link libraries if needed
# target_link_libraries(FlashRadixTree PRIVATE some_library)
# target_link_libraries(FlashRadixTreeWithSplay PRIVATE some_library)
