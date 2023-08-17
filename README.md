# Jffmpeg

This repository contains the source code of the **jffmpeg** project. [The webiste of this project is here.](https://jffmpeg.sourceforge.net/download.html)

Below is the content of the README file from the project.

## Introduction

JFFMPEG is a JNI wrapper and Java port of the FFMPEG audio/video opensource codec suite.

## Supported Formats

The following table shows the support for different audio and video
encodings (note that some formats require the wrapper onto ffmpeg).

| Format            | Native | Java |
|-------------------|--------|------|
| H263              | *      |      |
| H263/RTP          | *      |      |
| MPEG 1            | *      | *    |
| MPEG 2            | *      | *    |
| MPEG 4            | *      | *    |
| DIVX              | *      | *    |
| DX50              | *      | *    |
| XVID              | *      | *    |
| DIV3              | *      | *    |
| MPG4              | *      | *    |
| MP42              | *      | *    |
| WMV1              | *      |      |
| WMV2              | *      |      |
| MJPG              | *      |      |
| **Audio Formats** |        |      |
| MP3               |        | *    |
| AC3               |        | *    |
| Vorbis            |        | *    |
| **File Formats**  |        |      |
| VOB               |        | *    |
| OGG               |        | *    |

## Installation

When built, JFFMPEG has two files:
- jffmpeg.jar
- libjffmpeg.so

The .jar file should be included on the classpath when JMF is in use. See the JMF documentation on how to add codecs to the JMF classpath.

The libjffmpeg.so native library is optional, but supports a wider range of formats with higher performance.

## Registering Codecs and formats

The Java Media Framework comes with a tool JMFRegistry to add codecs. It is available from the jmf.jar file, or from the "Preferences..." menu option in JMStudio.

Under the "Mime Types" tab register the following MIME types:
1)   video/vob    ->    vob
2)   audio/ogg    ->    ogg

Under the "Plugins" tab register the following "Demultiplexers"

1)   net.sourceforge.jffmpeg.demux.vob.VobDemux
2)   net.sourceforge.jffmpeg.demux.ogg.OggDemux

Under the "Codec" tab, register the following codecs:

1)   net.sourceforge.jffmpeg.VideoDecoder
2)   net.sourceforge.jffmpeg.AudioDecoder
