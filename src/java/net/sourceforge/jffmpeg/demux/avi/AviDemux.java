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
 * AVI file demultiplexer.  Effectively this simply maintains a HashMap
 * of data buffers representing the audio and video streams in the VOB file.
 *
 * Some timing information is tracked here
 */
public class AviDemux implements Demultiplexer, Positionable, GPLLicense {
    /**
     * Maximum number of audio streams that this class supports
     */
    private final int MAX_AUDIO_STREAMS = 1;

    /**
     * Position of the end of the headers (start of media)
     * Position of the end of the file (end of media)
     */
    private long endOfHeader;
    private long endOfFile;

    /**
     * Number of tracks in the AVI file
     */
    private int numberOfAudioChannels = 0;

    /**
     * Media Tracks in the AVI file 
     */
    private AviTrack[] track = null;

    /**
     * Video identifier
     */
    private String streamVideoTag;

    /**
     * AVI input stream
     */
    private PullSourceStream dataSource;

    /**
     * Seekable Input Stream
     */
    private Seekable seekSource;


    /**
     * Duration in seconds
     */
    private double duration;

    /**
     * Return name of Demultiplexer
     */
    public String getName() {
        return "JFFMPEG AVI file demultiplexer";
    }
    
    /**
     * File formats supported by this demultiplexer.
     *     video.x_msvideo
     */
    public ContentDescriptor[] getSupportedInputContentDescriptors() {
        //System.out.println( "AVI file getSupportedInputContentDescriptors()" );
        return new ContentDescriptor[] {
            new FileTypeDescriptor( "video.x_msvideo" )
        };
    }

    /**
     * Return a list of Media Tracks in the AVI file
     */
    public Track[] getTracks() throws IOException, BadHeaderException {
        //System.out.println( "AVI file getTracks()" );
        return track;
    }
    
    /**
     * Get GUI controls for this demultiplexer
     */
    public Object getControl(String str) {
        //System.out.println( "AVI file getControl()" );
        return null;
    }
    
    /**
     * Get GUI controls for this demultiplexer
     */
    public Object[] getControls() {
        //System.out.println( "AVI file getControls()" );
        return new Object[0];
    }


    /**
     * Returns the duration of this AVI file
     */
    public Time getDuration() {
        //System.out.println( "AVI file getDuration() " + duration );
        return new Time( duration );
    }
    
    /**
     * Returns the current position in the AVI file
     */
    public Time getMediaTime() {
        //System.out.println( "AVI file getMediaTime()" );
        return new Time( ((double)seekSource.tell() - endOfHeader)
       	                /((double)(endOfFile - endOfHeader)) );
    }
    
    /**
     * Reposition video stream
     */
    public Time setPosition(javax.media.Time time, int param) {
        //System.out.println( "AVI file setPosition()" );
        return getMediaTime();
    }
    
    /**
     * Can we reposition this stream?
     */
    public boolean isPositionable() {
        //System.out.println( "AVI file isPositionable()" );
        return seekSource.isRandomAccess();
    }
    
    /**
     * Can we reposition this stream?
     */
    public boolean isRandomAccess() {
        //System.out.println( "AVI file isRandomAccess()" );
        return seekSource.isRandomAccess(); 
    }

    /**
     * Open the AVI file
     */    
    public void open() throws javax.media.ResourceUnavailableException {
        //System.out.println( "AVI file open()" );
    }
    
    /**
     * Close the AVI file.
     */    
    public void close() {
        //System.out.println( "AVI file close()" );
    }    
    
    /**
     * Reset demultiplexer
     */    
    public void reset() {
        //System.out.println( "AVI file reset()" );
    }
        
    /**
     * Set the InputDataSource
     */    
    public void setSource( DataSource inputDataSource ) 
                              throws java.io.IOException, 
                                     javax.media.IncompatibleSourceException {
        //System.out.println( "AVI file setSource()" );
        if ( !(inputDataSource instanceof PullDataSource) ) {
            throw new javax.media.IncompatibleSourceException();
        }

        dataSource = ((PullDataSource)inputDataSource).getStreams()[0];
        if ( dataSource instanceof Seekable ) {
            seekSource = (Seekable)dataSource;
        }

        /* Find end of stream */
        endOfFile = dataSource.getContentLength();

    }
    
    /**
     * Start parsing the Demultiplexer
     */
    public void start() throws java.io.IOException {
        //System.out.println( "AVI file start()" );
        if ( track != null ) return;
        track = new AviTrack[ 1 + MAX_AUDIO_STREAMS ];


        /**
         * Parse header identifier
         *   RIFF
         *   xxxx
         *   AVI 
         */
        String id = new String( readBuffer( 4 ), "ASCII" );
        readBuffer( 4 );
        String type = new String( readBuffer( 4 ), "ASCII" );

        if (   !"RIFF".equalsIgnoreCase( id ) 
            || !"AVI ".equalsIgnoreCase( type ) ) {
            throw new IOException( "Not AVI file" );
        }

        /**
         * Extract header data 
         *     LIST <length> command [data]
         *     xxxx <length> [data]
         */
        byte[] hdrl = null;
        byte[] idx = null;

        while ( true ) {
            String command = new String( readBuffer( 4 ), "ASCII" );
            int length = (readBytes(4) + 1) &~1;

            if ( "LIST".equalsIgnoreCase( command ) ) {
                String subcommand = new String( readBuffer( 4 ), "ASCII" ); 
                length -= 4;
                if ( "movi".equalsIgnoreCase( subcommand ) ) {
                    break;
		}
                if ( "hdrl".equalsIgnoreCase( subcommand ) ) {
                    hdrl = readBuffer( length );
		}
                if ( "idx1".equalsIgnoreCase( subcommand ) ) {
                    idx = readBuffer( length );
		}
                if ( "iddx".equalsIgnoreCase( subcommand ) ) {
                    idx = readBuffer( length );
		}
	    } else {
                readBuffer( length );
            }
        }

        /**
         * Parse hdrl
         *    LIST <length> xxxx
         *    strh <length> vids 4CID xxxx xxxx xxxx scale rate
         *    strh <length> auds 4CID xxxx xxxx xxxx scale rate xxxxxxxx samplesize
         *    strf <length> BIH
         */
        int streamNumber = 0;
        int lastTagID = 0;
        for ( int i = 0; i < hdrl.length; ) {
            String command = new String( hdrl, i, 4 );
            int    size    = str2ulong( hdrl, i + 4 );

            if ( "LIST".equalsIgnoreCase( command ) ) {
                i += 12;
                continue;
            }

            String command2 = new String( hdrl, i+8, 4 );
            if ( "strh".equalsIgnoreCase( command ) ) {
                lastTagID = 0;
                if ( "vids".equalsIgnoreCase( command2 ) ) {
                    AviVideoTrack video;
                    String compressor = new String( hdrl, i+12, 4);
                    int scale = str2ulong( hdrl, i+28 );
                    int rate  = str2ulong( hdrl, i+32 );
                    int length = str2ulong( hdrl, i+40 );
                    duration = ((double)length * scale)/((double)rate);
                    video = new AviVideoTrack( this, streamNumber++, compressor, scale, rate ); 
                    track[0] = video;
                    lastTagID = 0;
                    streamVideoTag = video.getVideoTag();
                }
                if ( "auds".equalsIgnoreCase( command2 ) ) {
                    AviAudioTrack audio;
                    int scale = str2ulong( hdrl, i+28);
                    int rate = str2ulong( hdrl, i+32);
                    int sampleSize = str2ulong( hdrl, i+52);
                    audio = new AviAudioTrack( this, streamNumber++, scale, rate, sampleSize );
                    track[ 1 + numberOfAudioChannels ] = audio;
                    lastTagID = 1 + numberOfAudioChannels;
                    numberOfAudioChannels++;
                }
            }

            if ( "strf".equalsIgnoreCase( command ) ) {
                /**
                 * Additional track information
                 */
                byte[] information = new byte[ size - 4 ];
                System.arraycopy( hdrl, i + 4, 
                                  information, 0, information.length );
                track[ lastTagID ].setBih( information );
            }
            i += size + 8;
	}
        endOfHeader = seekSource.tell();
    }

    /**
     * Shut down demultiplexer
     */
    public void stop() {
        //System.out.println( "AVI file stop()" );
    }  


    /**
     * Read a Frame
     */
    public synchronized long readFrame( Buffer buffer, boolean video, long position ) throws IOException {
        boolean isVideo;

        if ( position < endOfHeader ) position = endOfHeader;
        seekSource.seek( position );
        do {
            isVideo = getChunk( buffer );
            /**
             * Skip padding
             */
            if ( (buffer.getLength() & 1) == 1 ) readBuffer( 1 );
        } while ( isVideo != video );
        return seekSource.tell();
    }
        

    /**
     * Get a Chunk
     *     LIST <size> xxxx COMM <size>
     *     RIFF <size> xxxx COMM <size>
     *     VIDD
     */
    private boolean getChunk( Buffer output ) throws IOException {
        String command = new String( readBuffer( 4 ), "ASCII" ).toUpperCase();
        int size = readBytes(4);

	/**
         * Skip LIST and RIFF
         */ 
        while (   "LIST".equals( command )
               || "RIFF".equals( command ) ) { 
            readBuffer( 4 );
            command = new String( readBuffer( 4 ), "ASCII" ).toUpperCase();
            size = readBytes(4);
        }

        /**
         * Read data chunk
         */
        output.setData( readBuffer(size) );
        output.setLength( size );

        /**
         * Look for ##db ##dc ##wb [video]
         */
        String videoTag = streamVideoTag.substring(0, 3);
        if ( command.substring(0, 3).equalsIgnoreCase( videoTag ) &&
             (command.charAt(3) == 'B' || command.charAt(3) == 'C') ) {
	    /**
             * Video
             */
            return true; 
        }

        /**
         * Match Audio track
         */
        for ( int i = 0; i < numberOfAudioChannels; i++ ) {
//            if ( command.equalsIgnoreCase( audio[i].getAudioTag() ) ) {
                /**
                 * Audio
                 */
                return false; 
//          }
        }

        /**
         * Something else
         */
        throw new IOException( "Not header " + command );
    }
    


    /**
     * str2ulong
     */
    public static final int str2ulong( byte[] data, int i ) {
        return    (data[ i ] & 0xff) 
               | ((data[ i + 1 ] & 0xff) << 8 )
	       | ((data[ i + 2 ] & 0xff) << 16 )
	       | ((data[ i + 3 ] & 0xff) << 24 );
    }

    /**
     * Read a byte array 
     */
    private final byte[] readBuffer( int size ) throws IOException {
        byte[] buffer = new byte[ size ];

        int read = 0;
        while ( read < size ) {
            int next = dataSource.read( buffer, read, size - read );
            if ( next < 0 ) throw new IOException( "End of Stream" );
            read += next;
	}
        return buffer;
    }


    /**
     * Read unsigned byte
     */
    private final int readByte() throws IOException {
        byte[] data = new byte[ 1 ];
        dataSource.read( data, 0, 1 );
        return data[0];
    }


    /**
     * Read up to 4 bytes
     */
    private final int readBytes( int number ) throws IOException {
        byte[] buffer = new byte[ number ];
        int read = dataSource.read( buffer, 0, number );

        if ( read != buffer.length ) {
            if ( read < 0 ) throw new IOException( "End of Stream" );
            for ( int i = read; i < buffer.length; i++ ) buffer[ i ] = (byte)readByte();
        }
        
	/**
         * Create integer
         */
        switch ( number ) {
            case 1: return (buffer[ 0 ] & 0xff);
	    case 2: return (buffer[ 0 ] & 0xff) | ((buffer[ 1 ] & 0xff) << 8);
	    case 3: return (buffer[ 0 ] & 0xff) | ((buffer[ 1 ] & 0xff) << 8) | ((buffer[ 2 ] & 0xff) << 16);
	    case 4: return (buffer[ 0 ] & 0xff) | ((buffer[ 1 ] & 0xff) << 8) | ((buffer[ 2 ] & 0xff) << 16) | ((buffer[ 3 ] & 0xff) << 24);
	    default: throw new IOException( "Illegal Read quantity" );
	}
    }
}
