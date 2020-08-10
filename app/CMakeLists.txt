# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DANDROID -std=c++11 -flax-vector-conversions -O2 -ftree-vectorize  -fexceptions -D__aarch64__ ")

# Use the file `hexagon_helper.cmake` as helper to CMakeLists.txt.
# It contains the definition of functions `buildIDL` and `prepare_hexagon_helper_libraries`.
include(${CMAKE_CURRENT_SOURCE_DIR}/hexagon_helper.cmake)

# Build calculator.idl to produce calculator.h, calculator_stub.c and calculator_skel.c
buildIDL(../calculator_c++_app/app/inc/calculator.idl calculator)

# Prepare the library rpcmem.a to and adsprpc.so produce corresponding
# hexagon_targets, hexagon_incs and hexagon_libs.
prepare_hexagon_helper_libraries(hexagonTargets_1 hexagonIncs_1 hexagonLibs_1 rpcmem.a adsprpc cdsprpc)

set(incs ${CMAKE_CURRENT_BINARY_DIR}/)

set(HEXAGON_SDK_ROOT "/home/hugoliu/Qualcomm/Hexagon_SDK/3.5.2")
set(HALIDE_ROOT  ${HEXAGON_SDK_ROOT}/tools/HALIDE_Tools/2.2.03/Halide)
set(HALIDE_SRC_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/../halide/blur)
include_directories(${HALIDE_ROOT}/include)
include_directories(${HALIDE_ROOT}/support)
include_directories(${HALIDE_SRC_PATH}/bin/arm-64-android)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library(calculator
        SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/calculator-jni.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/calculator_stub.c
        ${CMAKE_CURRENT_BINARY_DIR}/calculator.h
        )

include_directories(${hexagonIncs_1} ${incs})
add_dependencies(calculator ${hexagonTargets_1})
target_link_libraries(calculator ${hexagonLibs_1})

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library(log-lib log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries(calculator ${log-lib})

target_link_libraries(calculator ${HALIDE_SRC_PATH}/bin/arm-64-android/halide_blur.a)