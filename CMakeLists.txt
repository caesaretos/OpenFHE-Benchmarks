cmake_minimum_required(VERSION 3.16)
project(FHE-Perf-blog)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the specific path for OpenFHE v1.2.3
set(OpenFHE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../myinstalledlib")

message(STATUS "OpenFHE Lib Path: ${OpenFHE_DIR}")

# Find OpenFHE package with exact version and no fallback to system files
find_package(OpenFHE 1.2.3 EXACT REQUIRED NO_DEFAULT_PATH PATHS ${OpenFHE_DIR})

if(NOT OpenFHE_FOUND)
    message(FATAL_ERROR "OpenFHE v1.2.3 not found at the specified location: ${OpenFHE_DIR}")
endif()

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenFHE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3")

# Create executables
add_executable(bench-add-mul bench-add-mul.cpp utils.cpp)
add_executable(bench-boots bench-boots.cpp utils.cpp)
add_executable(bench-add-mul-unencrypted bench-add-mul-unencrypted.cpp utils.cpp)

# List targets
set(BENCHMARK_TARGETS bench-add-mul bench-boots bench-add-mul-unencrypted)

# Set include directories for all benchmark targets
foreach(target_name ${BENCHMARK_TARGETS})
  target_include_directories(${target_name} PRIVATE
    ${OpenFHE_INCLUDE}
    ${OpenFHE_INCLUDE}/third-party/include
    ${OpenFHE_INCLUDE}/core
    ${OpenFHE_INCLUDE}/pke
    ${OpenFHE_INCLUDE}/binfhe
  )
endforeach()

# Set library directories
link_directories(${OpenFHE_LIBRARY_DIRS})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${OpenFHE_LIBRARY_DIRS}")

# Link libraries
target_link_libraries(bench-add-mul PRIVATE ${OpenFHE_SHARED_LIBRARIES})
target_link_libraries(bench-boots PRIVATE ${OpenFHE_SHARED_LIBRARIES})

# Status messages
message(STATUS "Building using include: ${OpenFHE_INCLUDE}")
message(STATUS "Building using lib: ${OpenFHE_LIBRARY_DIRS}")