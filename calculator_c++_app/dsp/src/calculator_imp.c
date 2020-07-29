/*==============================================================================
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
  All rights reserved. Qualcomm Proprietary and Confidential.
==============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "HAP_farf.h"
#include "calculator.h"
#include "AEEStdErr.h"

int calculator_open(const char*uri, remote_handle64* handle) {
   void *tptr = NULL;
  /* can be any value or ignored, rpc layer doesn't care
   * also ok
   * *handle = 0;
   * *handle = 0xdeadc0de;
   */
   tptr = (void *)malloc(1);
   *handle = (remote_handle64)tptr;
   assert(*handle);
   return 0;
}

/**
 * @param handle, the value returned by open
 * @retval, 0 for success, should always succeed
 */
int calculator_close(remote_handle64 handle) {
   if (handle)
      free((void*)handle);
   return 0;
}

AEEResult calculator_sum(remote_handle64 h, const int* vec, int vecLen, int64* res)
{
  int ii = 0;
  *res = 0;
  for (ii = 0; ii < vecLen; ++ii) {
    *res = *res + vec[ii];
  }
  FARF(RUNTIME_HIGH, "===============     DSP: sum result %lld ===============", *res);
  return AEE_SUCCESS;
}

AEEResult calculator_remap(remote_handle64 h,
                     const unsigned int* pRemapTable0, int pRemapTable0Len,
                     const unsigned int* pRemapTable1, int pRemapTable1Len,
                     const unsigned int* pRemapTable2, int pRemapTable2Len,
                     const unsigned int* pRemapTable3, int pRemapTable3Len,
                     const unsigned char* inputBuffer, int inputBufferLen,
                     unsigned char* outBuffer, int outBufferLen
                     )
{
    for(unsigned int i=0; i<inputBufferLen; ++i){
        outBuffer[i] = (inputBuffer[pRemapTable0[i]] + inputBuffer[pRemapTable1[i]] + inputBuffer[pRemapTable2[i]] + inputBuffer[pRemapTable3[i]]) >> 2;
    }
    return AEE_SUCCESS;
}

/*
int calculator_diff(const int* vec, int vecLen, int64* res)
{
  int ii = 0;
  *res = vec[0];
  for (ii = 1; ii < vecLen; ++ii) {
    *res = *res - vec[ii];
  }
  FARF(RUNTIME_HIGH, "===============     DSP: diff result %lld ===============", *res);
  return 0;
}
*/

