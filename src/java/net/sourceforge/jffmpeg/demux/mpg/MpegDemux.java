/*
 * Java port of ffmpeg MPEG demultiplexer.
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
 * 1a39e335700bec46ae31a38e2156a898
 */
package net.sourceforge.jffmpeg.demux.mpg;

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
import javax.media.Format;
import javax.media.format.AudioFormat;
import javax.media.format.VideoFormat;
import javax.media.TrackListener;

import java.io.IOException;
import java.util.HashMap;
import java.util.Enumeration;

import java.awt.Dimension;

/**
 * MPEG file demultiplexer.
 */
public class MpegDemux implements Demultiplexer, Positionable {
    /**
     * Stream packets identifiers
     */
    public static final int PACKET_START_CODE_MASK   = 0xffffff00;
    public static final int PACKET_START_CODE_PREFIX = 0x00000100;

    public static final int SEQUENCE_START_CODE      = 0x000001b3;
    public static final int EXT_START_CODE           = 0x000001b5;
    public static final int SEQUENCE_END_CODE        = 0x000001b7;
    public static final int GOP_START_CODE           = 0x000001b8;
    public static final int ISO_11172_END_CODE       = 0x000001b9;
    public static final int PACK_START_CODE          = 0x000001ba;
    public static final int SYSTEM_HEADER_START_CODE = 0x000001bb;
    public static final int PROGRAM_STREAM_MAP       = 0x000001bc;
    public static final int PRIVATE_STREAM_1         = 0x000001bd;
    public static final int PADDING_STREAM           = 0x000001be;
    public static final int PRIVATE_STREAM_2         = 0x000001bf;

    /**
     * Underlying file Input Stream
     */
    private PullSourceStream in;

    /**
     * Seekable file stream
     */
    private Seekable seek;

    /**
     * Media tracks in this file
     */
    private HashMap tracks = new HashMap();


    /**
     * Open Demultiplexer resources
     */
    public void open() throws javax.media.ResourceUnavailableException {
        System.out.println( "MPEGDemux open()" );
    }
    
    /**
     * Close Demultiplexer resources
     */
    public void close() {
        System.out.println( "MPEGDemux close()" );
    }    
    
    /**
     * Reset Demultiplexer
     */
    public void reset() {
        System.out.println( "MPEGDemux reset()" );
    }


    /**
     * Start parsing MPEG file
     */    
    public void start() throws IOException {
        System.out.println( "MPEGDemux start()" );

        MpegVideoTrack test = new MpegVideoTrack( this, 0x1e0 );
        test.parseHeader();
        test.readFrame( new Buffer() );
        test.readFrame( new Buffer() );
        test.readFrame( new Buffer() );
    }
    
    /**
     * Shut down MPEG file
     */
    public void stop() {
        System.out.println( "MPEGDemux stop()" );
    }

    /**
     * Get demultiplexer controls
     */
    public Object getControl( String control ) {
        System.out.println( "MPEGDemux getControl()" );
        return null;
    }
    
    /**
     * Get demultiplexer controls
     */
    public Object[] getControls() {
        System.out.println( "MPEGDemux getControls()" );
        return new Object[0];
    }


    /**
     * Return duration of the MPEG file
     */
    public Time getDuration() {
        System.out.println( "MPEGDemux getDuration()" );
        return new Time( 0 );
    }
    
    /**
     * Current position in the MPEG file
     */
    public Time getMediaTime() {
        System.out.println( "MPEGDemux getMediaTime()" );
        return new Time( 0 );
    }

    /**
     * Can this stream be repositioned?
     */
    public boolean isPositionable() {
        return seek.isRandomAccess();
    }
    
    /**
     * Can this stream be arbitrarily repositioned?
     */
    public boolean isRandomAccess() {
        return seek.isRandomAccess();
    }
    
    /**
     * Reposition stream
     */
    public Time setPosition( javax.media.Time newTime, int parameter ) {
        System.out.println( "MPEGDemux setPosition()" );
        return newTime;
    }

    /**
     * Demultiplexer identifier
     */
    public String getName() {
        return "JFFMPEG MPEG File Demultiplexer";
    }
    
    /**
     * Supported input formats
     */
    public ContentDescriptor[] getSupportedInputContentDescriptors() {
        System.out.println( "MPEGDemux getSupportedInputContentDescriptors()" );
        return new ContentDescriptor[] {
            new FileTypeDescriptor( "video.mpeg" )
        };
    }

    /**
     * Get the Tracks currently being parsed by the demultiplexer
     */
    public Track[] getTracks() throws IOException, BadHeaderException {
        System.out.println( "MPEGDemux getTracks() " + tracks.size() );
        return (Track[])tracks.values().toArray( new Track[0] );
    }
    
    /**
     * Set the Input Stream
     */
    public void setSource( DataSource inputDataSource ) throws java.io.IOException, 
                                                               javax.media.IncompatibleSourceException {
        System.out.println( "MPEGDemux getSource()" );
        if ( !(inputDataSource instanceof PullDataSource) ) {
            throw new javax.media.IncompatibleSourceException();
        }

        in = ((PullDataSource)inputDataSource).getStreams()[0];
        if (    in instanceof Seekable 
             && in.getContentLength() != in.LENGTH_UNKNOWN) {
            seek = (Seekable)in;
        }
    }

    /**
     * Read packet
     */
    private static final int PACKET_READ_SIZE = 1024;
    protected synchronized long readPacket( Buffer output, 
                                            int id, 
                                            long pos ) throws IOException {
        /**
         * Set buffer size
         */
        byte[] buffer = (byte[])output.getData();
        if ( buffer == null || buffer.length < 100000 ) {
            buffer = new byte[ 1000000 ];
            output.setData( buffer );
        }

        /**
         * Seek to start of packet
         */
        pos = seek.seek( pos );
        int j = output.getLength();

        /**
         * Read packet header
         */
        in.read( buffer, j, 4 ); j += 4; pos += 4;

        /**
         * Read until next packet header
         */
        int p = 0;
        int i = 0xffffffff;
        while ( (i|0xff) != 0x1ff ) {
            if ( (p % PACKET_READ_SIZE) == 0 ) { 
                in.read( buffer, j, PACKET_READ_SIZE ); 
            }
            j++; pos++; p++;
            i = ((i << 8) | (0xff & buffer[j-1])) & 0xffffffff;
        }
        pos -= 4;

        /**
         * Set to output
         */
        output.setData( buffer );
        output.setLength( j - 4 );
        return pos;
    }

    /**
     * Skip past DTS packet
     * - Check if this packet should be added as a new stream
     */
    protected synchronized long skipDTSPacket( long pos ) throws IOException {
        readDTSHeader( null, pos);

        pos = seek.seek( pos + 4 );
        in.read( scratch, 0, 2 ); pos += 2;
        int len = ((scratch[0]&0xff)<<8) | (scratch[1]&0xff);
        return pos + len;
    }

    /**
     * Read Presentation Timestamp
     */
    private long get_pts( int c ) throws IOException {
        long pts;
        int val;

        if (c < 0) {
            in.read( scratch, 0, 1 );
            c = scratch[0]&0xff;
        }

        in.read( scratch, 0, 4 );
        pts = ((long)((c >> 1) & 0x07)) << 30;
        val = ((scratch[0]&0xff)<<8) | (scratch[1]&0xff);
        pts |= ((long)(val >> 1)) << 15;
        val = ((scratch[2]&0xff)<<8) | (scratch[3]&0xff);
        pts |= ((long)(val >> 1));
        return pts;
    }

    /**
     * Read a DTS packet header
     */
    protected synchronized long readDTSHeader( Buffer output, 
                                               long pos ) throws IOException {
        return readDTSPacket( output, 0, pos, true );
    }

    /**
     * Read a DTS packet
     */
    protected synchronized long readDTSPacket( Buffer output, 
                                               int streamId,
                                               long pos ) throws IOException {
        return readDTSPacket( output, streamId, pos, false );
    }
    /**
     * Read a DTS packet
     */
    private synchronized long readDTSPacket( Buffer output, 
                                             int streamId,
                                             long pos,
                                             boolean onlyHeader )
                                                   throws IOException {
        /**
         * Check buffer size 
         */
        byte[] buffer = null;
        if ( output != null ) {
            buffer = (byte[])output.getData();
            if ( buffer == null || buffer.length < 100000 ) {
                buffer = new byte[ 1000000 ];
            }
        }

        /**
         * Seek to start of packet
         */
        pos = seek.seek( pos );

        /**
         * Read packet header
         */
        int startcode = peekPacket( pos );
        in.read( scratch, 0, 4 ); pos += 4;

        /**
         * Read packet length
         */
        in.read( scratch, 0, 2 ); pos += 2;
        int len = ((scratch[0]&0xff)<<8) | (scratch[1]&0xff);

        /* Set up Display and Presentation Timestamp */
        long dts = -1;
        long pts = -1;

        /**
         * Skip padding
         */
        int c;
        do {
            in.read( scratch, 0, 1 ); pos++; len--;
            c = scratch[0] & 0xff;
        } while ( c == 0xff );

        /**
         * Parse header bits
         */
        if ((c & 0xc0) == 0x40) {
            in.read( scratch, 0, 2 ); pos += 2; len -= 2;
            c = scratch[1] & 0xff;
        }
        if ((c & 0xf0) == 0x20) {
            /* Display and Presentation Timestamps are the same */
            dts = get_pts(c);
            pts = dts;
            len -= 4; pos += 4;
        } else if ((c & 0xf0) == 0x30) {
            /* Display and Presentation Timestamps are different */
            pts = get_pts(c);
            dts = get_pts(-1);
            len -= 9; pos += 4;
        } else if ((c & 0xc0) == 0x80) {
            /* Mpeg 2 Header extension */
            if ((c & 0x30) != 0) {
                throw new IOException( "Encrypted streams are not handled" );
            }

            in.read( scratch, 0, 2 ); pos += 2; len -= 2;
            int flags = scratch[0]&0xff;
            int header_len = scratch[1]&0xff;
            if (header_len > len) {
                throw new IOException( "Invalid Header" );
            }
            if ((flags & 0xc0) == 0x80) {
                dts = get_pts(-1);
                pts = dts;
                if (header_len < 5) {
                    throw new IOException( "Invalid Header" );
                }
                header_len -= 5;
                len -= 5; pos += 5;
            } 
            if ((flags & 0xc0) == 0xc0) {
                pts = get_pts(-1);
                dts = get_pts(-1);
                if (header_len < 10) {
                    throw new IOException( "Invalid Header" );
                }
                header_len -= 10;
                len -= 10; pos += 10;
            }
            len -= header_len; pos += header_len;
            while (header_len > 0) {
                in.read( scratch, 0, 1 );
                header_len--;
            }
        } else if( c != 0x0f ) {
            throw new IOException( "Invalid Header" );
        }

        /* Special case for 0x1bd */
        if ( startcode == PRIVATE_STREAM_1 ) {
            if (len < 1) {
                throw new IOException( "Invalid Header" );
            }
            in.read( scratch, 0, 1 ); 
            len--; pos += 1;

            startcode = scratch[0] & 0xff;
            if (startcode >= 0x80 && startcode <= 0xbf) {
                /* audio: skip header */
                if (len < 3) {
                    throw new IOException( "Invalid Header" );
                }
                in.read( scratch, 0, 3 ); len -= 3; pos += 3;
            }

            /* Fudge "extended" startcode */
            if (streamId != startcode && !onlyHeader ) {
                onlyHeader = true;
                pos += len;
            }
        }


        if ( !onlyHeader ) {
            /**
             * Read until next packet header
             */
            int j = output.getLength();
            in.read( buffer, j, len ); j += len; pos += len;

            output.setData( buffer );
            output.setLength( j );

            /* Handle Timestamp */
            if ( pts >= 0 && startcode == 0x1e0) {
                System.out.println( dts + "  --  " + pts );
                output.setTimeStamp( pts * 1000000 );
            }
        } else {
            /**
             * We are skipping this packet
             *  -- Should we create a track to handle it?
             */
            if ( !tracks.containsKey( new Integer( startcode ) ) ) {
		System.out.println( "Add track " + startcode );
                if (startcode >= 0x1e0 && startcode <= 0x1ef) {
                    MpegTrack track = new MpegVideoTrack( this, startcode );
                    /* Mpeg2 video */
                    tracks.put( new Integer( startcode ), 
                                track );
                    track.parseHeader();
                } else if (startcode >= 0x1c0 && startcode <= 0x1df) {
                    MpegTrack track = new MpegAudioTrack( this, startcode );
                    /* Mpeg Layer 2 Audio */
                    tracks.put( new Integer( startcode ), 
                                track );
                    track.parseHeader();
                } else if (startcode >= 0x80 && startcode <= 0x9f) {
                    MpegTrack track = new MpegAC3AudioTrack( this, 
                                             PRIVATE_STREAM_1,
                                             startcode );
                    /* AC3 */
                    tracks.put( new Integer( startcode ), 
                                track );
                    track.parseHeader();
                } else if (startcode >= 0xa0 && startcode <= 0xbf) {
                    /* PCM */
                }
            }
        }

        return pos;
    }

    /**
     * Look at the next packet and retrieve ID
     */
    private byte[] scratch = new byte[ 4 ];
    protected synchronized int peekPacket( long pos ) throws IOException {
        pos = seek.seek( pos );
        in.read( scratch, 0, 4 );
        int startcode = ((scratch[0]&0xff)<<24)|((scratch[1]&0xff)<<16)
                       |((scratch[2]&0xff)<<8) | (scratch[3]&0xff);
        seek.seek( pos );
        return startcode;
    }

    /**
     * Seek to the next packet
     */
    protected synchronized long skipPacket( long pos ) throws IOException {
        pos = seek.seek( pos + 1 );
        int i = 0xffffffff;
        while ( ((i|0xff) != 0x1ff) && (i != 0) ) {
            in.read( scratch, 0, 1 );
            i = ((i << 8) | (0xff & scratch[0])) & 0xffffffff;
            pos++;
        }
        return pos - 4;
    }

}
