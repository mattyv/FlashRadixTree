cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(MyProject VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Check if we are NOT compiling on Linux
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_definitions(-DNOT_LINUX)
  # Set the sources that are not for Linux
  set(NOT_LINUX_SOURCES
    SomeAppleSpecificFile.cpp
    SomeOtherNonLinuxSpecificFile.cpp
    # ... other non-Linux-specific source files
  )
endif()

# Add the executable
add_executable(FlashRadixTree main.cpp)

# If you have other .cpp files, add them to the executable. For example:
# add_executable(MyProject main.cpp SplayTree.cpp FlashRadixTree.cpp)

# To include header files, you can use the include_directories command if needed
include_directories(${PROJECT_SOURCE_DIR/M1-Cycles})

# If there are any libraries you need to link to, use the target_link_libraries command
# target_link_libraries(MyProject PRIVATE some_library)

