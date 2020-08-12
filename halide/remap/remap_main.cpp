#include <cmath>
#include <cstdint>
#include <cstdio>
#ifdef __SSE2__
#include <emmintrin.h>
#elif __ARM_NEON
#include <arm_neon.h>
#endif

#include "halide_benchmark.h"
#include "HalideBuffer.h"

#include "HalideRuntimeHexagonHost.h"

using namespace Halide::Runtime;
using namespace Halide::Tools;

double t;

Buffer<uint8_t> remap(Buffer<uint8_t> in, Buffer<uint16_t> tableX, Buffer<uint16_t> tableY) {
    const uint16_t inW = in.width();
    const uint16_t inH = in.height();
    const uint16_t inC = in.channels();
    Buffer<uint8_t> out(inW, inH, inC);

    t = benchmark(10, 1, [&]() {
        for(uint16_t y = 0; y < inH; y++){
            for(uint16_t x = 0; x < inW; ++x){
                uint16_t nnx = tableX(x, y);
                uint16_t nny = tableY(x, y);
                for(uint16_t c = 0; c < inC; ++c){
                    out(x, y, c) = (in(nnx, nny, c) + in(nnx+1, nny, c) + in(nnx, nny+1, c) + in(nnx+1, nny+1, c)) << 2;
                }
            }
        }
    });

    return out;
}

#include "halide_remap.h"

Buffer<uint8_t> test_halide_remap(Buffer<uint8_t> in, Buffer<uint16_t> tableXY) {
    const uint16_t inW = in.width();
    const uint16_t inH = in.height();
    const uint16_t inC = in.channels();
    Buffer<uint8_t> out(inW, inH, inC);

    // Call it once to initialize the halide runtime stuff
    halide_remap(in, tableXY, out);
//    halide_remap(in, out);
    // Copy-out result if it's device buffer and dirty.
    out.copy_to_host();

    t = benchmark(10, 1, [&]() {
        // Compute the same region of the output as blur_fast (i.e., we're
        // still being sloppy with boundary conditions)
        halide_remap(in, tableXY, out);
//        halide_remap(in, out);
        // Sync device execution if any.
        out.device_sync();
    });

    out.copy_to_host();

    return out;
}

int main(int argc, char **argv) {
#ifndef HALIDE_RUNTIME_HEXAGON
    const int width = 1920;
    const int height = 1080;
#else
    // The Hexagon simulator can't allocate as much memory as the above wants.
    const int width = 1920;
    const int height = 1080;
#endif
    const int channel = 3;


#ifdef TEST_HEX_IFACES
    int err_list[] = {halide_error_code_out_of_memory, halide_error_code_copy_to_host_failed, 0};
    halide_hexagon_set_error_fault_mask(NULL, false, err_list);

    halide_hexagon_set_malloc_tracing(NULL, 1);

    const long long int grow_min = 256*1024*1024;
    halide_hexagon_mem_set_grow_size(NULL, grow_min, 0x7fffffff);
#endif

    Buffer<uint8_t> input(width, height, channel);
    Buffer<uint16_t> tableX(width, height);
    Buffer<uint16_t> tableY(width, height);
    Buffer<uint16_t> tableXY(width, height, 2);

    const uint16_t inW = input.width();
    const uint16_t inH = input.height();
    const uint16_t inC = input.channels();
    printf("input image params, w: %d, h: %d, c: %d\n", inW, inH, inC);

    const uint16_t dim0Stride = input.dim(0).stride();
    const uint16_t dim1Stride = input.dim(1).stride();
    const uint16_t dim2Stride = input.dim(2).stride();
    printf("input image stride, dim0: %d, dim1: %d, dim2: %d\n", dim0Stride, dim1Stride, dim2Stride);

    uint16_t tableXMax = 0;
    uint16_t tableYMax = 0;

    for (uint16_t y = 0; y < inH; y++) {
        for (uint16_t x = 0; x < inW; x++) {
            tableX(x, y) = (x > inW-2) ? (inW-2) : x;
            tableY(x, y) = (y > inH-2) ? (inH-2) : y;

            tableXY(x, y, 0) = (x > inW-2) ? (inW-2) : x;
            tableXY(x, y, 1) = (y > inH-2) ? (inH-2) : y;
            for(uint16_t c = 0; c < inC; c++){
                input(x, y, c) = rand() & 0xff;
            }
            if(tableX(x, y) > tableXMax){
                tableXMax = tableX(x, y);
            }
            if(tableY(x, y) > tableYMax){
                tableYMax = tableY(x, y);
            }
        }
    }

    printf("tableX max: %d, tableY max: %d\n", tableXMax, tableYMax);

    printf("remap cpu start\n");
    Buffer<uint8_t> remapCpu = remap(input, tableX, tableY);
    double cpu_time = t;
    printf("remap cpu done, cpu_time: %f\n", cpu_time);

    Buffer<uint8_t> remapHalide = test_halide_remap(input, tableXY);
    double hexagon_time = t;
    printf("remap hexagon done, hexagon_time: %f\n", hexagon_time);

    uint32_t diffCnt = 0;
    for (uint16_t y = 0; y < inH; y++) {
        for (uint16_t x = 0; x < inW; x++) {
            for(uint16_t c = 0; c < inC; c++){
//                if(y == 0){
//                    printf("at (%d, %d, %d), remapHalide: %d\n", x, y, c, (int)remapHalide(x, y, c));
//                }
                if(remapHalide(x, y, c) != remapCpu(x, y, c)){
                    ++diffCnt;
                    printf("diff at (%d, %d, %d), remapHalide: %d, remapCpu: %d", x, y, c, (int)remapHalide(x, y, c), (int)remapCpu(x, y, c));
                }
            }
        }
    }
    printf("diffCnt: %d\n", diffCnt);

    return 0;
}
