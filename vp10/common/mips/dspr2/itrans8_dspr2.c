/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include <stdio.h>

#include "./vpx_config.h"
#include "./vp10_rtcd.h"
#include "vp10/common/common.h"
#include "vp10/common/blockd.h"
#include "aom_dsp/mips/inv_txfm_dspr2.h"
#include "aom_dsp/txfm_common.h"
#include "aom_ports/mem.h"

#if HAVE_DSPR2
void vp10_iht8x8_64_add_dspr2(const int16_t *input, uint8_t *dest,
                              int dest_stride, int tx_type) {
  int i, j;
  DECLARE_ALIGNED(32, int16_t, out[8 * 8]);
  int16_t *outptr = out;
  int16_t temp_in[8 * 8], temp_out[8];
  uint32_t pos = 45;

  /* bit positon for extract from acc */
  __asm__ __volatile__("wrdsp    %[pos],    1    \n\t" : : [pos] "r"(pos));

  switch (tx_type) {
    case DCT_DCT:  // DCT in both horizontal and vertical
      idct8_rows_dspr2(input, outptr, 8);
      idct8_columns_add_blk_dspr2(&out[0], dest, dest_stride);
      break;
    case ADST_DCT:  // ADST in vertical, DCT in horizontal
      idct8_rows_dspr2(input, outptr, 8);

      for (i = 0; i < 8; ++i) {
        iadst8_dspr2(&out[i * 8], temp_out);

        for (j = 0; j < 8; ++j)
          dest[j * dest_stride + i] = clip_pixel(
              ROUND_POWER_OF_TWO(temp_out[j], 5) + dest[j * dest_stride + i]);
      }
      break;
    case DCT_ADST:  // DCT in vertical, ADST in horizontal
      for (i = 0; i < 8; ++i) {
        iadst8_dspr2(input, outptr);
        input += 8;
        outptr += 8;
      }

      for (i = 0; i < 8; ++i) {
        for (j = 0; j < 8; ++j) {
          temp_in[i * 8 + j] = out[j * 8 + i];
        }
      }
      idct8_columns_add_blk_dspr2(&temp_in[0], dest, dest_stride);
      break;
    case ADST_ADST:  // ADST in both directions
      for (i = 0; i < 8; ++i) {
        iadst8_dspr2(input, outptr);
        input += 8;
        outptr += 8;
      }

      for (i = 0; i < 8; ++i) {
        for (j = 0; j < 8; ++j) temp_in[j] = out[j * 8 + i];

        iadst8_dspr2(temp_in, temp_out);

        for (j = 0; j < 8; ++j)
          dest[j * dest_stride + i] = clip_pixel(
              ROUND_POWER_OF_TWO(temp_out[j], 5) + dest[j * dest_stride + i]);
      }
      break;
    default: printf("vp10_short_iht8x8_add_dspr2 : Invalid tx_type\n"); break;
  }
}
#endif  // #if HAVE_DSPR2
