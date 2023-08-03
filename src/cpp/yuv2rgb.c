/* Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <math.h>
#include <stdlib.h>

#include "yuv2rgb.h"

static YUVTables * get_yuv_tables(int depth, long r_mask, long g_mask, long b_mask);
static void release_yuv_tables(YUVTables *yuv_t);
static void yuv_to_rgb16(YUVTables *tables,
                         unsigned char *lum, unsigned char *cb, unsigned char *cr,
                         unsigned char *out, int cols, int rows);
static void yuv_to_rgb24(YUVTables *tables,
                         unsigned char *lum, unsigned char *cb, unsigned char *cr,
                         unsigned char *out, int cols, int rows);
static void yuv_to_rgb32(YUVTables *tables,
                         unsigned char *lum, unsigned char *cb, unsigned char *cr,
                         unsigned char *out, int cols, int rows);

Converter* yuv2rgb_get_converter(int depth, long r_mask, long g_mask, long b_mask)
{
  Converter *cnv;

  cnv = malloc (sizeof (Converter));
  if (cnv == NULL)
    return NULL;

  cnv->color_tables = get_yuv_tables(depth, r_mask, g_mask, b_mask);
  if (cnv->color_tables == NULL)
    goto fail;

  switch(depth) {
    case 32:
        cnv->convert = yuv_to_rgb32;
        break;
    case 24:
        cnv->convert = yuv_to_rgb24;
        break;
    case 15:
    case 16:
        cnv->convert = yuv_to_rgb16;
      break;
    default:
      goto fail;
  }

  return cnv;

fail:
  free(cnv->color_tables);
  free(cnv);
  return NULL;
}

void release_converter(Converter *cnv) 
{
  if (cnv->color_tables != NULL)
    release_yuv_tables(cnv->color_tables);
  free(cnv);
}

/*
 * How many 1 bits are there in the longword.
 * Low performance, do not call often.
 */

static int number_of_bits_set(unsigned long a)
{
  if(!a) return 0;
  if(a & 1) return 1 + number_of_bits_set(a >> 1);
  return(number_of_bits_set(a >> 1));
}

/*
 * How many 0 bits are there at most significant end of longword.
 * Low performance, do not call often.
 */
static int free_bits_at_top(unsigned long a)
{
  /* assume char is 8 bits */
  if(!a) return sizeof(unsigned long) * 8;
  /* assume twos complement */
  if(((long)a) < 0l) return 0;
  return 1 + free_bits_at_top(a << 1);
}

/*
 * How many 0 bits are there at least significant end of longword.
 * Low performance, do not call often.
 */
static int free_bits_at_bottom(unsigned long a)
{
  /* assume char is 8 bits */
  if(!a) return sizeof(unsigned long) * 8;
  if(a & 1) return 0;
  return 1 + free_bits_at_bottom(a >> 1);
}

/*
 * Initialize the lookup table (in order to get rid of the multiply
 * and other conversions).
 */

static YUVTables *get_yuv_tables(int depth, long r_mask, long g_mask, long b_mask)
{
    int CR, CB, i;
    int *L_tab, *Cr_r_tab, *Cr_g_tab, *Cb_g_tab, *Cb_b_tab;
    long *r_2_pix_alloc;
    long *g_2_pix_alloc;
    long *b_2_pix_alloc;
    int r_num_bits_unset = 8 - number_of_bits_set(r_mask);
    int r_free_bits_at_bottom = free_bits_at_bottom(r_mask);
    int g_num_bits_unset = 8 - number_of_bits_set(g_mask);
    int g_free_bits_at_bottom = free_bits_at_bottom(g_mask);
    int b_num_bits_unset = 8 - number_of_bits_set(b_mask);
    int b_free_bits_at_bottom = free_bits_at_bottom(b_mask);

    YUVTables *tables = (YUVTables *)malloc(sizeof(YUVTables));
    if (tables == NULL)
      return NULL;

    L_tab    = tables->L_tab = (int *)malloc(256*sizeof(int)); 
    Cr_r_tab = tables->Cr_r_tab = (int *)malloc(256*sizeof(int));
    Cr_g_tab = tables->Cr_g_tab = (int *)malloc(256*sizeof(int));
    Cb_g_tab = tables->Cb_g_tab = (int *)malloc(256*sizeof(int));
    Cb_b_tab = tables->Cb_b_tab = (int *)malloc(256*sizeof(int));
    r_2_pix_alloc = (long *)malloc(768*sizeof(long));
    g_2_pix_alloc = (long *)malloc(768*sizeof(long));
    b_2_pix_alloc = (long *)malloc(768*sizeof(long));

    if (L_tab == NULL ||
	Cr_r_tab == NULL ||
	Cr_g_tab == NULL ||
	Cb_g_tab == NULL ||
	Cb_b_tab == NULL ||
	r_2_pix_alloc == NULL ||
	g_2_pix_alloc == NULL ||
	b_2_pix_alloc == NULL)
      return NULL;

    for (i=0; i<256; i++) {
      L_tab[i] = i;
      CB = CR = i - 128;
      Cr_r_tab[i] =  (0.419/0.299) * CR; // 1.402, 1.371
      Cr_g_tab[i] = -(0.299/0.419) * CR; // -0.714, -0.698
      Cb_g_tab[i] = -(0.114/0.331) * CB; // -0.344, -0.336
      Cb_b_tab[i] =  (0.587/0.331) * CB; // 1.772, 1.733
    }

    /*
     * Set up entries 0-255 in rgb-to-pixel value tables.
     */
    for (i = 0; i < 256; i++) {
      r_2_pix_alloc[i + 256] = i >> r_num_bits_unset;
      r_2_pix_alloc[i + 256] <<= r_free_bits_at_bottom;
      g_2_pix_alloc[i + 256] = i >> g_num_bits_unset;
      g_2_pix_alloc[i + 256] <<= g_free_bits_at_bottom;
      b_2_pix_alloc[i + 256] = i >> b_num_bits_unset;
      b_2_pix_alloc[i + 256] <<= b_free_bits_at_bottom;
    }

    /*
     * Spread out the values we have to the rest of the array so that
     * we do not need to check for overflow.
     */
    for (i = 0; i < 256; i++) {
      r_2_pix_alloc[i] = r_2_pix_alloc[256];
      r_2_pix_alloc[i + 512] = r_2_pix_alloc[511];
      g_2_pix_alloc[i] = g_2_pix_alloc[256];
      g_2_pix_alloc[i + 512] = g_2_pix_alloc[511];
      b_2_pix_alloc[i] = b_2_pix_alloc[256];
      b_2_pix_alloc[i + 512] = b_2_pix_alloc[511];
    }

    tables->r_2_pix = r_2_pix_alloc + 256;
    tables->g_2_pix = g_2_pix_alloc + 256;
    tables->b_2_pix = b_2_pix_alloc + 256;

    return tables;
}

static void release_yuv_tables(YUVTables *yuv_t)
{
  free(yuv_t->L_tab);
  free(yuv_t->Cr_r_tab);
  free(yuv_t->Cr_g_tab);
  free(yuv_t->Cb_g_tab);
  free(yuv_t->Cb_b_tab);
  free(yuv_t->r_2_pix - 256);
  free(yuv_t->g_2_pix - 256);
  free(yuv_t->b_2_pix - 256);
  free(yuv_t);
}

/*
 * Convert image into 16 bit color.
 */

static void yuv_to_rgb16(YUVTables *tables,
                         unsigned char *lum, unsigned char *cb, unsigned char *cr,
                         unsigned char *out, int cols, int rows)
{
    int L, CR, CB;
    unsigned short *row1, *row2;
    unsigned char *lum2;
    int x, y;
    int cr_r, crb_g, cb_b;
    int cols_2 = cols>>1;

    row1 = (unsigned short *)out;
    row2 = row1 + cols;
    lum2 = lum + cols;

    for (y=rows>>1; y; y--) {
	for (x=cols_2; x; x--) {

	    CR = *cr++;
	    CB = *cb++;
	    cr_r = tables->Cr_r_tab[CR];
	    crb_g = tables->Cr_g_tab[CR] + tables->Cb_g_tab[CB];
	    cb_b = tables->Cb_b_tab[CB];

            L = tables->L_tab[(int) *lum++];

	    *row1++ = (tables->r_2_pix[L+cr_r] | tables->g_2_pix[L+crb_g] | tables->b_2_pix[L+cb_b]);

            L = tables->L_tab[(int) *lum++];

	    *row1++ = (tables->r_2_pix[L+cr_r] | tables->g_2_pix[L+crb_g] | tables->b_2_pix[L+cb_b]);

	    /*
	     * Now, do second row.
	     */
	    L = tables->L_tab[(int) *lum2++];

	    *row2++ = (tables->r_2_pix[L+cr_r] | tables->g_2_pix[L+crb_g] | tables->b_2_pix[L+cb_b]);

	    L = tables->L_tab[(int) *lum2++];

	    *row2++ = (tables->r_2_pix[L+cr_r] | tables->g_2_pix[L+crb_g] | tables->b_2_pix[L+cb_b]);
	}
        /*
         * These values are at the start of the next line, (due
         * to the ++'s above),but they need to be at the start
         * of the line after that.
         */
	lum = lum2;
	row1 = row2;
	lum2 += cols;
	row2 += cols;
    }
}

/*
 * Convert image into 24 bit color.
 */

static void yuv_to_rgb24(YUVTables *tables,
                         unsigned char *lum, unsigned char *cb, unsigned char *cr,
                         unsigned char *out, int cols, int rows)
{
    int L, CR, CB;
    unsigned char *row1, *row2;
    unsigned char *lum2;
    int x, y;
    int cr_r, crb_g, cb_b;
    int cols_2 = cols>>1;
    int cols_3 = cols*3;
    unsigned char pixels[4];

    row1 = out;
    row2 = row1 + cols_3;
    lum2 = lum + cols;
    for (y=rows>>1; y; y--) {
	for (x=cols_2; x; x--) {

	    CR = *cr++;
	    CB = *cb++;
	    cr_r = tables->Cr_r_tab[CR];
	    crb_g = tables->Cr_g_tab[CR] + tables->Cb_g_tab[CB];
	    cb_b = tables->Cb_b_tab[CB];

            L = tables->L_tab[(int) *lum++];

	    ((int *)pixels)[0] = (tables->r_2_pix[L+cr_r] | tables->g_2_pix[L+crb_g] | tables->b_2_pix[L+cb_b]);
	    *row1++ = pixels[0]; *row1++ = pixels[1]; *row1++ = pixels[2];

            L = tables->L_tab[(int) *lum++];

	    ((int *)pixels)[0] = (tables->r_2_pix[L+cr_r] | tables->g_2_pix[L+crb_g] | tables->b_2_pix[L+cb_b]);
	    *row1++ = pixels[0]; *row1++ = pixels[1]; *row1++ = pixels[2];

	    /* Now, do second row. */

	    L = tables->L_tab[(int) *lum2++];

	    ((int *)pixels)[0] = (tables->r_2_pix[L+cr_r] | tables->g_2_pix[L+crb_g] | tables->b_2_pix[L+cb_b]);
	    *row2++ = pixels[0]; *row2++ = pixels[1]; *row2++ = pixels[2];

	    L = tables->L_tab[(int) *lum2++];

	    ((int *)pixels)[0] = (tables->r_2_pix[L+cr_r] | tables->g_2_pix[L+crb_g] | tables->b_2_pix[L+cb_b]);
	    *row2++ = pixels[0]; *row2++ = pixels[1]; *row2++ = pixels[2];
	}
	lum = lum2;
	row1 = row2;
	lum2 += cols;
	row2 += cols_3;
    }
}

/*
 * Convert image into 32 bit color.
 */

static void yuv_to_rgb32(YUVTables *tables,
                         unsigned char *lum, unsigned char *cb, unsigned char *cr,
                         unsigned char *out, int cols, int rows)
{
    int L, CR, CB;
    unsigned int *row1, *row2;
    unsigned char *lum2;
    int x, y;
    int cr_r, crb_g, cb_b;
    int cols_2 = cols>>1;

    row1 = (unsigned int *)out;
    row2 = row1 + cols;
    lum2 = lum + cols;
    for (y=rows>>1; y; y--) {
	for (x=cols_2; x; x--) {

	    CR = *cr++;
	    CB = *cb++;
	    cr_r = tables->Cr_r_tab[CR];
	    crb_g = tables->Cr_g_tab[CR] + tables->Cb_g_tab[CB];
	    cb_b = tables->Cb_b_tab[CB];

            L = tables->L_tab[(int) *lum++];

	    *row1++ = (tables->r_2_pix[L+cr_r] | tables->g_2_pix[L+crb_g] | tables->b_2_pix[L+cb_b]);

            L = tables->L_tab[(int) *lum++];

	    *row1++ = (tables->r_2_pix[L+cr_r] | tables->g_2_pix[L+crb_g] | tables->b_2_pix[L+cb_b]);

	    /*
	     * Now, do second row.
	     */

	    L = tables->L_tab [(int) *lum2++];

	    *row2++ = (tables->r_2_pix[L+cr_r] | tables->g_2_pix[L+crb_g] | tables->b_2_pix[L+cb_b]);

	    L = tables->L_tab [(int) *lum2++];

	    *row2++ = (tables->r_2_pix[L+cr_r] | tables->g_2_pix[L+crb_g] | tables->b_2_pix[L+cb_b]);
	}
	lum = lum2;
	row1 = row2;
	lum2 += cols;
	row2 += cols;
    }
}
