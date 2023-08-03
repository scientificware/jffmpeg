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
 * Superclass for tracks in an AVI file
 */
public abstract class AviTrack implements Track, GPLLicense {
    /**
     * AVI Demultiplexer class
     */
    protected AviDemux demux;

    /**
     * Current position in demultiplexer stream
     */
    private long pos;

    /**
     * Arbitrary binary blob data
     */
    protected byte[] bih;

    /**
     * Constructor - pass in demultiplexer
     */
    public AviTrack( AviDemux demux ) {
        this.demux = demux;
    }

    /**
     * Return Duration (from demultiplexer)
     */
    public Time getDuration() {
        return demux.getDuration();
    }

    /**
     * Return start time of this track (always 0)
     */
    public Time getStartTime() {
        return new Time( 0 );
    }
    
    /**
     * Return if this track is enabled (always)
     */
    public boolean isEnabled() {
        return true;
    }

    /**
     * Provide track disabling functionality
     */    
    public void setEnabled(boolean enabled) {
    }
    
    /**
     * Convert a frame number to a time
     */
    public Time mapFrameToTime(int param) {
        return new Time( param );
    }
    
    /**
     * Convert a time to a frame number
     */
    public int mapTimeToFrame(javax.media.Time time) {
        return 0;
    }

    /**
     * Supply a frame of data to codec
     */
    public void readFrame(Buffer outputBuffer) {
        try {
            pos = demux.readFrame( outputBuffer, isVideo(), pos );
        } catch ( IOException e ) {
        }
    }

    /**
     * Track listener (for push streams)
     */
    public void setTrackListener(TrackListener trackListener) {
    }
    
    /**
     * Binary BLOB
     */
    public void setBih( byte[] bih ) {
        this.bih = bih;
    }

    /**
     * Returns true if this is a video track
     */
    public abstract boolean isVideo();
}
