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
public class HuffmanCodes11 extends HuffmanCodes {
    
    /** Creates a new instance of HuffmanCodes1 */
    public HuffmanCodes11() {
        codes = new long[] {
             0x0003, 0x0004, 0x000a, 0x0018, 0x0022, 0x0021, 0x0015, 0x000f,
             0x0005, 0x0003, 0x0004, 0x000a, 0x0020, 0x0011, 0x000b, 0x000a,
             0x000b, 0x0007, 0x000d, 0x0012, 0x001e, 0x001f, 0x0014, 0x0005,
             0x0019, 0x000b, 0x0013, 0x003b, 0x001b, 0x0012, 0x000c, 0x0005,
             0x0023, 0x0021, 0x001f, 0x003a, 0x001e, 0x0010, 0x0007, 0x0005,
             0x001c, 0x001a, 0x0020, 0x0013, 0x0011, 0x000f, 0x0008, 0x000e,
             0x000e, 0x000c, 0x0009, 0x000d, 0x000e, 0x0009, 0x0004, 0x0001,
             0x000b, 0x0004, 0x0006, 0x0006, 0x0006, 0x0003, 0x0002, 0x0000,
        };
        codesSize = new long[] {
              2,  3,  5,  7,  8,  9,  8,  9,
              3,  3,  4,  6,  8,  8,  7,  8,
              5,  5,  6,  7,  8,  9,  8,  8,
              7,  6,  7,  9,  8, 10,  8,  9,
              8,  8,  8,  9,  9, 10,  9, 10,
              8,  8,  9, 10, 10, 11, 10, 11,
              8,  7,  7,  8,  9, 10, 10, 10,
              8,  7,  8,  9, 10, 10, 10, 10,
        };
        xsize = 8;
        generateVLCCodes();
    }
}
