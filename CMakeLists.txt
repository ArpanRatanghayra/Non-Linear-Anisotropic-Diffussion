cmake_minimum_required (VERSION 3.1)
project (pm)

# Handle OpenCL
find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIRS})
link_directories(${OpenCL_LIBRARY})

# set the C++ standard to C++ 11
set(CMAKE_CXX_STANDARD 11)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")

# Print the list of known compile features for this version of CMake
message ("cxx Flags:" ${CMAKE_CXX_FLAGS})
message("List of compile features: ${CMAKE_CXX_COMPILE_FEATURES}")

file (GLOB HEADERS "${PROJECT_SOURCE_DIR}/include/*.hpp")
file (GLOB SOURCES "${PROJECT_SOURCE_DIR}/source/*.cpp" "${PROJECT_SOURCE_DIR}/source/*.c")
file (GLOB KERNEL_SOURCES "${PROJECT_SOURCE_DIR}/kernel/*.cl")

source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})
source_group("kernels" FILES ${KERNEL_SOURCES})

add_definitions(-DCL_SILENCE_DEPRECATION)
add_executable (pm ${HEADERS} ${SOURCES} ${KERNEL_SOURCES})

# Properties->General->Output Directory
set_target_properties(pm PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# Set the direcoties that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
target_include_directories(pm PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries (pm ${OpenCL_LIBRARY})