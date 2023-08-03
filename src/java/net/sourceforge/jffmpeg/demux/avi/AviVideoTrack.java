/*
 * Java port of Xine AVI demultiplexer.
 * Contains Xine code (GPL)
 *
 * Copyright (c) 2003-2005 Jonathan Hueber.
 *
 * This AVI demultiplexer is only used for debugging.
 *
 * 1a39e335700bec46ae31a38e2156a898
 */
package net.sourceforge.jffmpeg.demux.avi;

import java.io.*;
import java.net.URL;
import java.util.HashMap;

import javax.media.Demultiplexer;
import javax.media.protocol.Positionable;
import javax.media.protocol.Seekable;
import javax.media.Time;
import javax.media.protocol.ContentDescriptor;
import javax.media.protocol.FileTypeDescriptor;
import javax.media.Track;
import javax.media.protocol.DataSource;
import javax.media.protocol.PullDataSource;
import javax.media.protocol.PullSourceStream;
import javax.media.MediaLocator;
import javax.media.BadHeaderException;
import javax.media.Buffer;
import javax.media.Track;
import javax.media.TrackListener;
import javax.media.format.VideoFormat;
import javax.media.format.AudioFormat;
import javax.media.Format;
import java.awt.Dimension;

import java.util.Iterator;

import net.sourceforge.jffmpeg.GPLLicense;


public class AviVideoTrack extends AviTrack implements GPLLicense {
    private int streamNumber;
    private String compressor;
    private float rate;

    public AviVideoTrack( AviDemux demux, int streamNumber, String compressor, int scale, int rate ) {
        super( demux );
        this.streamNumber = streamNumber;
        this.compressor = compressor;
        this.rate = ((float)rate)/((float)scale);
    }

    public boolean isVideo() {
	return true;
    }

    public String toString() {
        return "Stream: " + streamNumber + " Compressor " + compressor
             + " Rate " + rate;
    }

    public int getWidth() {
        return AviDemux.str2ulong( bih, 8 );
    }

    public int getHeight() {
        return AviDemux.str2ulong( bih, 12 );
    }

    public String getVideoTag() {
        return new String( new char[] { (char)((streamNumber / 10) + '0'),
				        (char)((streamNumber % 10) + '0'),
				        'd',
                                        'b' } );
    }        

    public Format getFormat() {
        return new VideoFormat( compressor, 
                                new Dimension(getWidth(), getHeight()), 
                                10000, 
                                (new byte[0]).getClass(), 
                                rate );
    }

    /**
     * Also known as biClrUsed
     */
    public int getPaletteCount() {
        return AviDemux.str2ulong( bih, 32 );
    }
}

