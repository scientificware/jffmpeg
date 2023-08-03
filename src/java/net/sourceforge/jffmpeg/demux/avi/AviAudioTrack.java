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


/**
 * AVI Audio track
 */
public class AviAudioTrack extends AviTrack implements GPLLicense {
    private int streamNumber;
    private int scale;
    private int rate;
    private int sampleSize;
    
    public AviAudioTrack( AviDemux demux, int streamNumber, int scale, int rate, int sampleSize ) {
        super( demux );
        this.streamNumber = streamNumber;
        this.scale = scale;
        this.rate = rate;
        this.sampleSize = sampleSize;
    }


    public long getRate() {
        return   (bih[  8 ] & 0xff)
              | ((bih[  9 ] & 0xff) << 8)
              | ((bih[ 10 ] & 0xff) << 16)
              | ((bih[ 11 ] & 0xff) << 24);
    }

    public boolean isVideo() {
	return false;
    }

    public String getAudioTag() {
        return new String( new char[] { (char)((streamNumber / 10) + '0'),
				        (char)((streamNumber % 10) + '0'),
				        'w',
                                        'b' } );
    }

    public String toString() {
        return "Stream: " + streamNumber 
             + " Audio Rate " + getRate()
             + " Scale " + scale 
             + " Sample size " + sampleSize;
    }

    public Format getFormat() {
        return new AudioFormat( "mpeglayer3", 
                                getRate(),
                                       16,
                                       2,
                                       0, 1); // endian, int signed
    }
}
