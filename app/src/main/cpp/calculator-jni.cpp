#include <jni.h>
#include <string>
#include <android/log.h>
#include <stdlib.h>
#include <rpcmem.h>
#include "remote.h"
#include <calculator.h>
#include "HalideWrapper.h"

using namespace std;
class calculator {

public:
    int64 result;
    int nErr;
}; // object;

extern "C"
JNIEXPORT jint JNICALL
Java_com_hexagondemo_calculator_MainCalculator_init(JNIEnv *env, jobject instance,
                                                         jstring skel_location_) {
    const char *skel_location = env->GetStringUTFChars(skel_location_, 0);
    setenv("ADSP_LIBRARY_PATH", skel_location, 1);
    __android_log_print(ANDROID_LOG_INFO, "hexagon", "set env ADSP_LIBRARY_PATH: %s", skel_location);
    env->ReleaseStringUTFChars(skel_location_, skel_location);
    return 0;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_hexagondemo_calculator_MainCalculator_sum(JNIEnv *env, jobject instance,
                                                        jintArray vec_, jint len) {
    jint *vec = env->GetIntArrayElements(vec_, NULL);
    calculator *object = new calculator();
    object->result = 0;
    object->nErr = 0;
    int *test = 0;
    int alloc_len = sizeof(*test) * len;
    remote_handle64 handle= -1;
    char calculator_URI_Domain[128];

    rpcmem_init();
    __android_log_print(ANDROID_LOG_INFO, "hexagon", "MainCalculator_sum enter");

    if (0 == (test = (int*)rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, alloc_len))) {
        object->nErr = 1;
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "rpcmem_alloc failed with nErr = %d", object->nErr);
        goto bail;
    }
    for (int i=0; i<len; i++){
        test[i] = vec[i];
    }

    strlcpy(calculator_URI_Domain, calculator_URI, strlen(calculator_URI) + 1);
    // first try opening handle on CDSP.
    strlcat(calculator_URI_Domain, "&_dom=cdsp", strlen(calculator_URI_Domain)+strlen("&_dom=cdsp") + 1);
    object->nErr = calculator_open(calculator_URI_Domain, &handle);

    // if CDSP is not present, try ADSP.
//    if (object->nErr != 0)
//    {
//        printf("cDSP not detected on this target (error code %d), attempting to use aDSP\n", object->nErr);
//        strlcat(calculator_URI_Domain, "&_dom=adsp", strlen(calculator_URI_Domain)+strlen("&_dom=adsp") + 1);
//        object->nErr = calculator_open(calculator_URI_Domain, &handle);
//    }
    if (object->nErr != 0)
    {
//        printf("Failed to open cdsp");
        __android_log_print(ANDROID_LOG_ERROR, "hexagon", "Failed to open cdsp: %d", object->nErr);
        goto bail;
    }

//    object->nErr = calculator_sum(handle, test, len, &object->result);
    if (object->nErr) {
//        printf("ERROR: Failed to compute calculator_sum\n");
        __android_log_print(ANDROID_LOG_ERROR, "hexagon", "Failed to compute calculator_sum");
    }
    __android_log_print(ANDROID_LOG_INFO, "hexagon", "MainCalculator_sum exit: %d", object->nErr);

bail:
    if (test) { rpcmem_free(test); }
    if (object) { delete object; }
    if (handle) { calculator_close(handle); }
    rpcmem_deinit();
    env->ReleaseIntArrayElements(vec_, vec, 0);
    return object->result;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_hexagondemo_calculator_MainCalculator_remap(JNIEnv *env, jobject instance) {
    calculator *object = new calculator();
    object->result = 0;
    object->nErr = 0;
    remote_handle64 handle= -1;
    char calculator_URI_Domain[128];
    static int BUF_SIZE = 1920*1080*3;
    unsigned int* pRemapTable0 = nullptr;
    unsigned int* pRemapTable1 = nullptr;
    unsigned int* pRemapTable2 = nullptr;
    unsigned int* pRemapTable3 = nullptr;
    unsigned char* inputBuffer = nullptr;
    unsigned char* outBuffer = nullptr;

    rpcmem_init();
    __android_log_print(ANDROID_LOG_INFO, "hexagon", "MainCalculator_remap enter");

    if (0 == (pRemapTable0 = (unsigned int*)rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, BUF_SIZE*sizeof(unsigned int)))) {
        object->nErr = 1;
        object->result = -1;
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "rpcmem_alloc failed with nErr = %d", object->nErr);
        goto bail;
    }
    if (0 == (pRemapTable1 = (unsigned int*)rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, BUF_SIZE*sizeof(unsigned int)))) {
        object->nErr = 1;
        object->result = -2;
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "rpcmem_alloc failed with nErr = %d", object->nErr);
        goto bail;
    }
    if (0 == (pRemapTable2 = (unsigned int*)rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, BUF_SIZE*sizeof(unsigned int)))) {
        object->nErr = 1;
        object->result = -3;
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "rpcmem_alloc failed with nErr = %d", object->nErr);
        goto bail;
    }
    if (0 == (pRemapTable3 = (unsigned int*)rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, BUF_SIZE*sizeof(unsigned int)))) {
        object->nErr = 1;
        object->result = -4;
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "rpcmem_alloc failed with nErr = %d", object->nErr);
        goto bail;
    }
    if (0 == (inputBuffer = (unsigned char*)rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, BUF_SIZE))) {
        object->nErr = 1;
        object->result = -5;
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "rpcmem_alloc failed with nErr = %d", object->nErr);
        goto bail;
    }
    if (0 == (outBuffer = (unsigned char*)rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, BUF_SIZE))) {
        object->nErr = 1;
        object->result = -6;
        __android_log_print(ANDROID_LOG_INFO, "hexagon", "rpcmem_alloc failed with nErr = %d", object->nErr);
        goto bail;
    }

    for (int i=0; i < BUF_SIZE; i++){
        pRemapTable0[i] = i;
        pRemapTable1[i] = i;
        pRemapTable2[i] = i;
        pRemapTable3[i] = i;
        inputBuffer[i] = 1;
        outBuffer[i] = 0;
    }

    strlcpy(calculator_URI_Domain, calculator_URI, strlen(calculator_URI) + 1);
    // first try opening handle on CDSP.
    strlcat(calculator_URI_Domain, "&_dom=cdsp", strlen(calculator_URI_Domain)+strlen("&_dom=cdsp") + 1);
    object->nErr = calculator_open(calculator_URI_Domain, &handle);

    if (object->nErr != 0)
    {
//        printf("Failed to open cdsp");
        __android_log_print(ANDROID_LOG_ERROR, "hexagon", "Failed to open domain %s: %d", calculator_URI_Domain, object->nErr);
        object->result = -7;
        goto bail;
    }

    object->nErr = calculator_remap(
            handle,
            pRemapTable0,
            BUF_SIZE,
            pRemapTable1,
            BUF_SIZE,
            pRemapTable2,
            BUF_SIZE,
            pRemapTable3,
            BUF_SIZE,
            inputBuffer,
            BUF_SIZE,
            outBuffer,
            BUF_SIZE);
    if (object->nErr != 0) {
//        printf("ERROR: Failed to compute calculator_sum\n");
        __android_log_print(ANDROID_LOG_ERROR, "hexagon", "Failed to compute calculator_remap");
        object->result = -8;
//        goto bail;
    }
    __android_log_print(ANDROID_LOG_INFO, "hexagon", "calculator_remap exit: %d", object->nErr);
    for (int i=0; i < BUF_SIZE; i++){
        object->result += outBuffer[i];
    }

    bail:
    if (pRemapTable0) { rpcmem_free(pRemapTable0); }
    if (pRemapTable1) { rpcmem_free(pRemapTable1); }
    if (pRemapTable2) { rpcmem_free(pRemapTable2); }
    if (pRemapTable3) { rpcmem_free(pRemapTable3); }
    if (inputBuffer) { rpcmem_free(inputBuffer); }
    if (outBuffer) { rpcmem_free(outBuffer); }

    if (object) { delete object; }
    if (handle) { calculator_close(handle); }
    rpcmem_deinit();
    return object->result;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_hexagondemo_calculator_MainCalculator_testHalide(JNIEnv *env, jobject instance) {
    using namespace HugoLiuHexagon;
    HalideWrapper* pHalide = new HalideWrapper("/vendor/lib/rfsa/adsp");
    return pHalide -> test();
}