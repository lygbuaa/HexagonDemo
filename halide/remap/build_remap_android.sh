#!/bin/sh
set -ex

export HALIDE_ROOT=$HEXAGON_SDK_ROOT/tools/HALIDE_Tools/2.2.03/Halide
INCLUDES="-I $HALIDE_ROOT/include -I $HALIDE_ROOT/support"
HL_LIBS="$HALIDE_ROOT/lib/libHalide.a"

rm -fr ./bin
mkdir -p bin/arm-64-android
g++ -D _GLIBCXX_USE_CXX11_ABI=0 -std=c++11 $INCLUDES  -g -fno-rtti halide_remap_generator.cpp $HALIDE_ROOT/tools/GenGen.cpp -g $HL_LIBS -o bin/halide_remap.generator -ldl -lpthread -lz
export LD_LIBRARY_PATH=$HALIDE_ROOT/lib/host:${HL_HEXAGON_TOOLS_SIM}/lib/iss/:.

bin/halide_remap.generator -g halide_remap -o ./bin/arm-64-android target=arm-64-android-hvx_128