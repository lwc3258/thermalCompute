cmake_minimum_required(VERSION 2.8)
project(cameraCalibration)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

MESSAGE("Build type: " ${CMAKE_BUILD_TYPE})


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall  -O3 -march=native -Wno-write-strings")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall   -O3 -march=native -Wno-write-strings")

# Check C++11 or C++0x support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
   add_definitions(-DCOMPILEDWITHC11)
   message(STATUS "Using flag -std=c++11.")
elseif(COMPILER_SUPPORTS_CXX0X)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
   add_definitions(-DCOMPILEDWITHC0X)
   message(STATUS "Using flag -std=c++0x.")
else()
   message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()


find_package(OpenCV 2.4.9 REQUIRED)

include_directories(
${PROJECT_SOURCE_DIR}
${PROJECT_SOURCE_DIR}/include
#${OpenCV_INCLUDE_DIR}
${PROJECT_SOURCE_DIR}/include/opencv
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/release/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/release)

#add_library(${PROJECT_NAME}  SHARED #STATIC
#	src/cameraCalibration.cpp
#
#)

#target_link_libraries(${PROJECT_NAME}
#${opencv_libs}
#${PROJECT_SOURCE_DIR}/lib/libpng16.so.16
#)

# Build

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(computeShift
src/computeShift.cpp)
target_link_libraries(computeShift
${OpenCV_LIBS}
)

add_executable(computeScale
src/computeScale.cpp)
target_link_libraries(computeScale
${OpenCV_LIBS}
)
add_executable(houghcircles
src/houghcircles.cpp)
target_link_libraries(houghcircles
${OpenCV_LIBS}
)
#add_executable(matchmethod_orb_akaze_brisk
#src/matchmethod_orb_akaze_brisk.cpp)
##target_link_libraries(matchmethod_orb_akaze_brisk
#${opencv_libs}
#${PROJECT_NAME}
#)

