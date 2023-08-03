/*
 * Java port of ffmpeg mp3 decoder.
 * Copyright (c) 2003 Jonathan Hueber.
 *
 * Copyright (c) 2000, 2001 Fabrice Bellard.
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
 *
 * See Credits file and Readme for details
 */
package net.sourceforge.jffmpeg.codecs.audio.mpeg.mp3.data;

/**
 *
 */
public class Table {

    public static int[] getSlenTable1() {
        return new int[] { 0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4 };
    }

    public static int[] getSlenTable2() {
        return new int[] { 0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3 };
    }
    
    public static int[][] getBandSizeLong() {
        return new int[][] {
            { 4, 4, 4, 4, 4, 4, 6, 6, 8, 8, 10,
              12, 16, 20, 24, 28, 34, 42, 50, 54, 76, 158, }, /* 44100 */
            { 4, 4, 4, 4, 4, 4, 6, 6, 6, 8, 10,
              12, 16, 18, 22, 28, 34, 40, 46, 54, 54, 192, }, /* 48000 */
            { 4, 4, 4, 4, 4, 4, 6, 6, 8, 10, 12,
              16, 20, 24, 30, 38, 46, 56, 68, 84, 102, 26, }, /* 32000 */
            { 6, 6, 6, 6, 6, 6, 8, 10, 12, 14, 16,
              20, 24, 28, 32, 38, 46, 52, 60, 68, 58, 54, }, /* 22050 */
            { 6, 6, 6, 6, 6, 6, 8, 10, 12, 14, 16,
              18, 22, 26, 32, 38, 46, 52, 64, 70, 76, 36, }, /* 24000 */
            { 6, 6, 6, 6, 6, 6, 8, 10, 12, 14, 16,
              20, 24, 28, 32, 38, 46, 52, 60, 68, 58, 54, }, /* 16000 */
            { 6, 6, 6, 6, 6, 6, 8, 10, 12, 14, 16,
              20, 24, 28, 32, 38, 46, 52, 60, 68, 58, 54, }, /* 11025 */
            { 6, 6, 6, 6, 6, 6, 8, 10, 12, 14, 16,
              20, 24, 28, 32, 38, 46, 52, 60, 68, 58, 54, }, /* 12000 */
            { 12, 12, 12, 12, 12, 12, 16, 20, 24, 28, 32,
              40, 48, 56, 64, 76, 90, 2, 2, 2, 2, 2, }, /* 8000 */
        };
    }
    
    public static int[][] getBandSizeShort() {
        return new int[][] {
            { 4, 4, 4, 4, 6, 8, 10, 12, 14, 18, 22, 30, 56, }, /* 44100 */
            { 4, 4, 4, 4, 6, 6, 10, 12, 14, 16, 20, 26, 66, }, /* 48000 */
            { 4, 4, 4, 4, 6, 8, 12, 16, 20, 26, 34, 42, 12, }, /* 32000 */
            { 4, 4, 4, 6, 6, 8, 10, 14, 18, 26, 32, 42, 18, }, /* 22050 */
            { 4, 4, 4, 6, 8, 10, 12, 14, 18, 24, 32, 44, 12, }, /* 24000 */
            { 4, 4, 4, 6, 8, 10, 12, 14, 18, 24, 30, 40, 18, }, /* 16000 */
            { 4, 4, 4, 6, 8, 10, 12, 14, 18, 24, 30, 40, 18, }, /* 11025 */
            { 4, 4, 4, 6, 8, 10, 12, 14, 18, 24, 30, 40, 18, }, /* 12000 */
            { 8, 8, 8, 12, 16, 20, 24, 28, 36, 2, 2, 2, 26, }, /* 8000 */
        };
    }
    
    public static int[][] getPreTab() {
        return new int[][] {
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 3, 3, 2, 0 },
        };
    }
    
    public static int[][] getBandIndexLong() {
        int[][] band_size_long = getBandSizeLong();
        int[][] band_index_long = new int[9][23];
        for ( int i = 0; i < 9; i++ ) {
            int k = 0;
            for ( int j = 0; j < 22; j++ ) {
                band_index_long[i][j] = k;
                k += band_size_long[i][j];
            }
            band_index_long[i][22] = k;
        }
        return band_index_long;
    }
    
    
    public static int[][] getHuffData() {
        return new int[][] {
            { 0, 0 },
            { 1, 0 },
            { 2, 0 },
            { 3, 0 },
            { 0, 0 },
            { 4, 0 },
            { 5, 0 },
            { 6, 0 },
            { 7, 0 },
            { 8, 0 },
            { 9, 0 },
            { 10, 0 },
            { 11, 0 },
            { 12, 0 },
            { 0, 0 },
            { 13, 0 },
            { 14, 1 },
            { 14, 2 },
            { 14, 3 },
            { 14, 4 },
            { 14, 6 },
            { 14, 8 },
            { 14, 10 },
            { 14, 13 },
            { 15, 4 },
            { 15, 5 },
            { 15, 6 },
            { 15, 7 },
            { 15, 8 },
            { 15, 9 },
            { 15, 11 },
            { 15, 13 }
        };
    }
    
    public static int[][][] getBitrateTable() {
        return new int[][][] {
            { {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448 },
              {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384 },
              {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 } },
            { {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256},
              {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160},
              {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}
            }
        };
    }
    
    public static int[] getMpaEnTable() {
        return new int[] {
             0,    -1,    -1,    -1,    -1,    -1,    -1,    -2,
            -2,    -2,    -2,    -3,    -3,    -4,    -4,    -5,
            -5,    -6,    -7,    -7,    -8,    -9,   -10,   -11,
           -13,   -14,   -16,   -17,   -19,   -21,   -24,   -26,
           -29,   -31,   -35,   -38,   -41,   -45,   -49,   -53,
           -58,   -63,   -68,   -73,   -79,   -85,   -91,   -97,
          -104,  -111,  -117,  -125,  -132,  -139,  -147,  -154,
          -161,  -169,  -176,  -183,  -190,  -196,  -202,  -208,
           213,   218,   222,   225,   227,   228,   228,   227,
           224,   221,   215,   208,   200,   189,   177,   163,
           146,   127,   106,    83,    57,    29,    -2,   -36,
           -72,  -111,  -153,  -197,  -244,  -294,  -347,  -401,
          -459,  -519,  -581,  -645,  -711,  -779,  -848,  -919,
          -991, -1064, -1137, -1210, -1283, -1356, -1428, -1498,
         -1567, -1634, -1698, -1759, -1817, -1870, -1919, -1962,
         -2001, -2032, -2057, -2075, -2085, -2087, -2080, -2063,
          2037,  2000,  1952,  1893,  1822,  1739,  1644,  1535,
          1414,  1280,  1131,   970,   794,   605,   402,   185,
           -45,  -288,  -545,  -814, -1095, -1388, -1692, -2006,
         -2330, -2663, -3004, -3351, -3705, -4063, -4425, -4788,
         -5153, -5517, -5879, -6237, -6589, -6935, -7271, -7597,
         -7910, -8209, -8491, -8755, -8998, -9219, -9416, -9585,
         -9727, -9838, -9916, -9959, -9966, -9935, -9863, -9750,
         -9592, -9389, -9139, -8840, -8492, -8092, -7640, -7134,
          6574,  5959,  5288,  4561,  3776,  2935,  2037,  1082,
            70,  -998, -2122, -3300, -4533, -5818, -7154, -8540,
         -9975,-11455,-12980,-14548,-16155,-17799,-19478,-21189,
        -22929,-24694,-26482,-28289,-30112,-31947,-33791,-35640,
        -37489,-39336,-41176,-43006,-44821,-46617,-48390,-50137,
        -51853,-53534,-55178,-56778,-58333,-59838,-61289,-62684,
        -64019,-65290,-66494,-67629,-68692,-69679,-70590,-71420,
        -72169,-72835,-73415,-73908,-74313,-74630,-74856,-74992,
         75038,
        };
    }
    
    /* number of lsf scale factors for a given size */
    public static int[][][] getLsfNsfTable() {
        return new int[][][] {
            { {  6,  5,  5, 5 }, {  9,  9,  9, 9 }, {  6,  9,  9, 9 } },
            { {  6,  5,  7, 3 }, {  9,  9, 12, 6 }, {  6,  9, 12, 6 } },
            { { 11, 10,  0, 0 }, { 18, 18,  0, 0 }, { 15, 18,  0, 0 } },
            { {  7,  7,  7, 0 }, { 12, 12, 12, 0 }, {  6, 15, 12, 0 } }, 
            { {  6,  6,  6, 3 }, { 12,  9,  9, 6 }, {  6, 12,  9, 6 } },
            { {  8,  8,  5, 0 }, { 15, 12,  9, 0 }, {  6, 18,  9, 0 } }
        };
    }
}
