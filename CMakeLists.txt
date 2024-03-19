cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(MyProject VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(FlashRadixTree main.cpp)

# If you have other .cpp files, add them to the executable. For example:
# add_executable(MyProject main.cpp SplayTree.cpp FlashRadixTree.cpp)

# To include header files, you can use the include_directories command if needed
include_directories(${PROJECT_SOURCE_DIR/M1-Cycles})

# If there are any libraries you need to link to, use the target_link_libraries command
# target_link_libraries(MyProject PRIVATE some_library)

