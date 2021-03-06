cmake_minimum_required (VERSION 3.1)
project (MarchingCubesServer)

set (CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.c)
add_executable(MarchingCubesServer ${SOURCES})
include_directories(src)

if(MSVC)
    set(CMAKE_CXX_FLAGS "-W3 /EHsc")
elseif(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-Wall")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "-Wall")
endif()

#make VERBOSE=1

cmake_policy(SET CMP0012 NEW)
find_package(OpenMP REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(websocketpp REQUIRED)
link_directories(${OPENCL_LIB_DIR})
target_link_libraries(MarchingCubesServer ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} websocketpp::websocketpp)
target_link_libraries(MarchingCubesServer OpenCL)

include_directories(${Boost_INCLUDE_DIR})
include_directories(${WEBSOCKETPP_INCLUDE_DIR})
include_directories(${OPENCL_INCLUDE_DIR})

if(OPENMP_FOUND)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
