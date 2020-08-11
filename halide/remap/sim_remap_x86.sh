#!/bin/sh
set -ex

export HALIDE_ROOT=$HEXAGON_SDK_ROOT/tools/HALIDE_Tools/2.2.03/Halide
INCLUDES="-I $HALIDE_ROOT/include -I $HALIDE_ROOT/support"
HL_LIBS="$HALIDE_ROOT/lib/libHalide.a"
export HLREV=`ls $HEXAGON_SDK_ROOT/tools/HEXAGON_Tools/ | head`
export HL_HEXAGON_TOOLS=$HEXAGON_SDK_ROOT/tools/HEXAGON_Tools/$HLREV/Tools
export HL_HEXAGON_TOOLS_SIM=${HL_HEXAGON_TOOLS_SIM:-"$HL_HEXAGON_TOOLS"}
export HL_HEXAGON_SIM_REMOTE=${HL_HEXAGON_SIM_REMOTE:-"$HALIDE_ROOT/bin/hexagon_sim_remote"}
export HL_HEXAGON_SIM_CYCLES=${HL_HEXAGON_SIM_CYCLES:-"1"}
export CXX=${CXX:-"g++"}
export PATH=$HL_HEXAGON_TOOLS/bin:$PATH
#############################################################################

rm -fr ./bin
mkdir -p bin/host

echo HL_LIBS=$HL_LIBS
echo INCLUDES=$INCLUDES
export LD_LIBRARY_PATH=$HALIDE_ROOT/lib/host:${HL_HEXAGON_TOOLS_SIM}/lib/iss/:.

${CXX}  -D _GLIBCXX_USE_CXX11_ABI=0 -std=c++11 $INCLUDES  -g -fno-rtti halide_remap_generator.cpp $HALIDE_ROOT/tools/GenGen.cpp -g $HL_LIBS -o bin/halide_remap.generator -ldl -lpthread -lz

bin/halide_remap.generator -g halide_remap -o ./bin/host target=host-hvx_128
#bin/halide_remap.generator -g halide_remap -o ./bin/host target=host
${CXX} -fopenmp -std=c++11 $INCLUDES -Wall -I ./bin/host -O3 remap_main.cpp ./bin/host/halide_remap.a -o bin/host/remap_main -lpthread -ldl -lm
./bin/host/remap_main
