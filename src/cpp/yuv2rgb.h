/*
 * Copyright (c) 1995 The Regents of the University of California.
 * All rights reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef __YUV2RGB_H__
#define __YUV2RGB_H__

typedef struct _YUVTables YUVTables;

struct _YUVTables {
/*
  int gammaCorrectFlag;
  double gammaCorrect;
  int chromaCorrectFlag;
  double chromaCorrect;
*/

  int *L_tab, *Cr_r_tab, *Cr_g_tab, *Cb_g_tab, *Cb_b_tab;

  /*
   *  We define tables that convert a color value between -256 and 512
   *  into the R, G and B parts of the pixel. The normal range is 0-255.
   **/

  long *r_2_pix, *g_2_pix, *b_2_pix;
};


typedef void (*ConvertFunction) (YUVTables *tables,
                                 unsigned char *lum, unsigned char *cb, unsigned char *cr,
                                 unsigned char *out, int cols, int rows);

typedef struct _Converter Converter;

struct _Converter {
  YUVTables *color_tables;
  ConvertFunction convert;
};


Converter* yuv2rgb_get_converter(int depth, long red_mask, long green_mask, long blue_mask);
void release_converter(Converter *cnv);

#define yuv2rgb(cnv, lum, cb, cr, rgb, cols, rows) \
        (cnv)->convert((cnv)->color_tables, (lum), (cb), (cr), (rgb), (cols), (rows))

#endif
