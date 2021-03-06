#ifndef HALIDE_WRAPPER_H
#define HALIDE_WRAPPER_H

#include <string>
#include <stdlib.h>
#include <android/log.h>
#include <assert.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

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
#include "halide_remap.h"
//#include "halide_blur.h"

namespace HugoLiuHexagon {

using namespace Halide::Runtime;
using namespace Halide::Tools;

class HalideWrapper
{
public:

private:
    double t;

public:
    HalideWrapper(const char *skel_location){
        setenv("ADSP_LIBRARY_PATH", skel_location, 1);
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "set env ADSP_LIBRARY_PATH: %s", skel_location);
    }

    ~HalideWrapper(){
    }

#if 0
    Buffer<uint16_t> blur(Buffer<uint16_t> in) {
        Buffer<uint16_t> tmp(in.width()-8, in.height());
        Buffer<uint16_t> out(in.width()-8, in.height()-2);

        t = benchmark(10, 1, [&]() {
            for (int y = 0; y < tmp.height(); y++)
                for (int x = 0; x < tmp.width(); x++)
                    tmp(x, y) = (in(x, y) + in(x+1, y) + in(x+2, y))/3;

            for (int y = 0; y < out.height(); y++)
                for (int x = 0; x < out.width(); x++)
                    out(x, y) = (tmp(x, y) + tmp(x, y+1) + tmp(x, y+2))/3;
        });

        return out;
    }

    Buffer<uint16_t> blur_fast(Buffer<uint16_t> in) {
        Buffer<uint16_t> out(in.width()-8, in.height()-2);

        t = benchmark(10, 1, [&]() {
#ifdef __SSE2__
            __m128i one_third = _mm_set1_epi16(21846);
#pragma omp parallel for
        for (int yTile = 0; yTile < out.height(); yTile += 32) {
            __m128i tmp[(128/8) * (32 + 2)];
            for (int xTile = 0; xTile < out.width(); xTile += 128) {
                __m128i *tmpPtr = tmp;
                for (int y = 0; y < 32+2; y++) {
                    const uint16_t *inPtr = &(in(xTile, yTile+y));
                    for (int x = 0; x < 128; x += 8) {
                        __m128i a = _mm_load_si128((const __m128i*)(inPtr));
                        __m128i b = _mm_loadu_si128((const __m128i*)(inPtr+1));
                        __m128i c = _mm_loadu_si128((const __m128i*)(inPtr+2));
                        __m128i sum = _mm_add_epi16(_mm_add_epi16(a, b), c);
                        __m128i avg = _mm_mulhi_epi16(sum, one_third);
                        _mm_store_si128(tmpPtr++, avg);
                        inPtr+=8;
                    }
                }
                tmpPtr = tmp;
                for (int y = 0; y < 32; y++) {
                    __m128i *outPtr = (__m128i *)(&(out(xTile, yTile+y)));
                    for (int x = 0; x < 128; x += 8) {
                        __m128i a = _mm_load_si128(tmpPtr+(2*128)/8);
                        __m128i b = _mm_load_si128(tmpPtr+128/8);
                        __m128i c = _mm_load_si128(tmpPtr++);
                        __m128i sum = _mm_add_epi16(_mm_add_epi16(a, b), c);
                        __m128i avg = _mm_mulhi_epi16(sum, one_third);
                        _mm_store_si128(outPtr++, avg);
                    }
                }
            }
        }
#elif __ARM_NEON
            uint16x4_t one_third = vdup_n_u16(21846);
#pragma omp parallel for
            for (int yTile = 0; yTile < out.height(); yTile += 32) {
                uint16x8_t tmp[(128/8) * (32 + 2)];
                for (int xTile = 0; xTile < out.width(); xTile += 128) {
                    uint16_t *tmpPtr = (uint16_t*)tmp;
                    for (int y = 0; y < 32+2; y++) {
                        const uint16_t *inPtr = &(in(xTile, yTile+y));
                        for (int x = 0; x < 128; x += 8) {
                            uint16x8_t a = vld1q_u16(inPtr);
                            uint16x8_t b = vld1q_u16(inPtr+1);
                            uint16x8_t c = vld1q_u16(inPtr+2);
                            uint16x8_t sum = vaddq_u16(vaddq_u16(a, b), c);
                            uint16x4_t sumlo = vget_low_u16(sum);
                            uint16x4_t sumhi = vget_high_u16(sum);
                            uint16x4_t avglo = vshrn_n_u32(vmull_u16(sumlo, one_third), 16);
                            uint16x4_t avghi = vshrn_n_u32(vmull_u16(sumhi, one_third), 16);
                            uint16x8_t avg = vcombine_u16(avglo, avghi);
                            vst1q_u16(tmpPtr, avg);
                            tmpPtr+=8;
                            inPtr+=8;
                        }
                    }
                    tmpPtr = (uint16_t*)tmp;
                    for (int y = 0; y < 32; y++) {
                        uint16_t *outPtr = &(out(xTile, yTile+y));
                        for (int x = 0; x < 128; x += 8) {
                            uint16x8_t a = vld1q_u16(tmpPtr+(2*128));
                            uint16x8_t b = vld1q_u16(tmpPtr+128);
                            uint16x8_t c = vld1q_u16(tmpPtr);
                            uint16x8_t sum = vaddq_u16(vaddq_u16(a, b), c);
                            uint16x4_t sumlo = vget_low_u16(sum);
                            uint16x4_t sumhi = vget_high_u16(sum);
                            uint16x4_t avglo = vshrn_n_u32(vmull_u16(sumlo, one_third), 16);
                            uint16x4_t avghi = vshrn_n_u32(vmull_u16(sumhi, one_third), 16);
                            uint16x8_t avg = vcombine_u16(avglo, avghi);
                            vst1q_u16(outPtr, avg);
                            tmpPtr+=8;
                            outPtr+=8;
                        }
                    }
                }
            }
#else
            // No intrinsics enabled, do a naive thing.
        for (int y = 0; y < out.height(); y++) {
            for (int x = 0; x < out.width(); x++) {
                int tmp[3] = {
                    (in(x, y) + in(x+1, y) + in(x+2, y))/3,
                    (in(x, y+1) + in(x+1, y+1) + in(x+2, y+1))/3,
                    (in(x, y+2) + in(x+1, y+2) + in(x+2, y+2))/3,
                };
                out(x, y) = (tmp[0] + tmp[1] + tmp[2])/3;
            }
        }
#endif
        });

        return out;
    }

    Buffer<uint16_t> blur_halide(Buffer<uint16_t> in) {
        Buffer<uint16_t> out(in.width()-8, in.height()-2);

        // Call it once to initialize the halide runtime stuff
        halide_blur(in, out);
        // Copy-out result if it's device buffer and dirty.
        out.copy_to_host();

        t = benchmark(10, 1, [&]() {
            // Compute the same region of the output as blur_fast (i.e., we're
            // still being sloppy with boundary conditions)
            halide_blur(in, out);
            // Sync device execution if any.
            out.device_sync();
        });

        out.copy_to_host();

        return out;
    }

    int testBlur(){
#ifndef HALIDE_RUNTIME_HEXAGON
        const int width = 6408;
        const int height = 4802;
#else
        // The Hexagon simulator can't allocate as much memory as the above wants.
    const int width = 648;
    const int height = 482;
#endif

#ifdef TEST_HEX_IFACES
        int err_list[] = {halide_error_code_out_of_memory, halide_error_code_copy_to_host_failed, 0};
    halide_hexagon_set_error_fault_mask(NULL, false, err_list);

    halide_hexagon_set_malloc_tracing(NULL, 1);

    const long long int grow_min = 256*1024*1024;
    halide_hexagon_mem_set_grow_size(NULL, grow_min, 0x7fffffff);
#endif

        Buffer<uint16_t> input(width, height);

        for (int y = 0; y < input.height(); y++) {
            for (int x = 0; x < input.width(); x++) {
                input(x, y) = rand() & 0xfff;
            }
        }

//        printf("blur test 1");
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "test blur start");
        Buffer<uint16_t> blurry = blur(input);
        double slow_time = t;

//        printf("blur test 2, show_time: %f", slow_time);
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "test blur slow_time: %f", slow_time);

        Buffer<uint16_t> speedy = blur_fast(input);
        double fast_time = t;

//        printf("blur test 3, fast_time: %f", fast_time);
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "test blur fast_time: %f", fast_time);

        Buffer<uint16_t> halide = blur_halide(input);
        double halide_time = t;

        __android_log_print(ANDROID_LOG_INFO, "hexagon", "test blur halide_time: %f", halide_time);
//        printf("times: %f %f %f\n", slow_time, fast_time, halide_time);

        for (int y = 64; y < input.height() - 64; y++) {
            for (int x = 64; x < input.width() - 64; x++) {
                if (blurry(x, y) != speedy(x, y) || blurry(x, y) != halide(x, y)) {
//                    printf("difference at (%d,%d): %d %d %d\n", x, y, blurry(x, y), speedy(x, y), halide(x, y));
                    __android_log_print(ANDROID_LOG_INFO, "hexagon", "difference at (%d,%d): %d %d %d\n", x, y, blurry(x, y), speedy(x, y), halide(x, y));
//                    abort();
                }
            }
        }

        return 0;
    }
#endif

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

    int test_remap() {
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
//        printf("input image params, w: %d, h: %d, c: %d\n", inW, inH, inC);
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "input image params, w: %d, h: %d, c: %d\n", inW, inH, inC);


        const uint16_t dim0Stride = input.dim(0).stride();
        const uint16_t dim1Stride = input.dim(1).stride();
        const uint16_t dim2Stride = input.dim(2).stride();
//        printf("input image stride, dim0: %d, dim1: %d, dim2: %d\n", dim0Stride, dim1Stride, dim2Stride);
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "input image stride, dim0: %d, dim1: %d, dim2: %d\n", dim0Stride, dim1Stride, dim2Stride);

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

//        printf("tableX max: %d, tableY max: %d\n", tableXMax, tableYMax);
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "tableX max: %d, tableY max: %d\n", tableXMax, tableYMax);

//        printf("remap cpu start\n");
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "remap cpu start");
        Buffer<uint8_t> remapCpu = remap(input, tableX, tableY);
        double cpu_time = t;
//        printf("remap cpu done, cpu_time: %f\n", cpu_time);
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "remap cpu done, cpu_time: %f\n", cpu_time);

        Buffer<uint8_t> remapHalide = test_halide_remap(input, tableXY);
        double hexagon_time = t;
//        printf("remap hexagon done, hexagon_time: %f\n", hexagon_time);
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "remap hexagon done, hexagon_time: %f\n", hexagon_time);

        uint32_t diffCnt = 0;
        for (uint16_t y = 0; y < inH; y++) {
            for (uint16_t x = 0; x < inW; x++) {
                for(uint16_t c = 0; c < inC; c++){
//                if(y == 0){
//                    printf("at (%d, %d, %d), remapHalide: %d\n", x, y, c, (int)remapHalide(x, y, c));
//                }
                    if(remapHalide(x, y, c) != remapCpu(x, y, c)){
                        ++diffCnt;
//                        printf("diff at (%d, %d, %d), remapHalide: %d, remapCpu: %d", x, y, c, (int)remapHalide(x, y, c), (int)remapCpu(x, y, c));
                        __android_log_print(ANDROID_LOG_INFO, "hexagon", "diff at (%d, %d, %d), remapHalide: %d, remapCpu: %d", x, y, c, (int)remapHalide(x, y, c), (int)remapCpu(x, y, c));
                    }
                }
            }
        }
//        printf("diffCnt: %d\n", diffCnt);
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "diffCnt: %d\n", diffCnt);

        return 0;
    }

};
}

#endif//HALIDE_WRAPPER_H