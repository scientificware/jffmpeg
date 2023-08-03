#
#  Copyright (c) 2002 Francisco Javier Cabello
#  Copyright (c) 2004 Guilhem Tardy (www.salyens.com)
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
# 
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
# 
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#
# Makefile for jffmpeg.
#


# Edit config.mak to set your own variables
include config.mak

DIR_CLASSES := $(OUTPUT)/classes
DIR_LIB     := $(OUTPUT)/dist
DIR_O       := $(OUTPUT)/cpp

OBJ_JAVA := java
OBJ_CPP  := decoder.o encoder.o yuv2rgb.o

JC  := javac
JAR := jar
CPP := gcc

JAR_FILE  := jffmpeg.jar
CFLAGS := -Wall -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE
LD_FLAGS := -shared

ifeq ($(TARGET_OS),win32)
# Assume Win32 running under MinGW
LIBNAME  := jffmpeg.dll
LIB_EXT  := dll
LIB_PREFIX := lib
CP_SEP := \;
LD_FLAGS := $(LD_FLAGS) -Wl,--add-stdcall-alias
else
# Assume normal GNU environment
LIBNAME  := libjffmpeg.so
LIB_EXT := a
LIB_PREFIX := lib
CP_SEP := :
endif

# LIBAVFORMAT_DIR  := $(FFMPEG_DIR)/libavformat
# LIBAVFORMAT_LIB  := $(LIBAVFORMAT_DIR)/$(LIB_PREFIX)avformat.$(LIB_EXT)
LIBAVCODEC_DIR  := $(FFMPEG_DIR)/libavcodec
LIBAVCODEC_LIB  := $(LIBAVCODEC_DIR)/$(LIB_PREFIX)avcodec.$(LIB_EXT)
# FFMPEG_LIBS  := $(LIBAVFORMAT_LIB) $(LIBAVCODEC_LIB)
FFMPEG_LIBS  := $(LIBAVCODEC_LIB)

INCLUDE_DIRS := -I. "-I$(JAVA_HOME)/include" "-I$(JAVA_HOME)/include/${TARGET_OS}" "-I$(FFMPEG_DIR)" -I$(DIR_O)

all: $(OBJ_JAVA) java $(OBJ_CPP) jffmpeg_lib

jffmpeg_lib: $(OBJ_CPP) Makefile
	$(CPP) $(LD_FLAGS) -o $(DIR_LIB)/$(LIBNAME) $(DIR_O)/*.o $(FFMPEG_LIBS)
	strip $(DIR_LIB)/$(LIBNAME)

%.o: src/cpp/%.c
	$(CPP) $(INCLUDE_DIRS) $(CFLAGS) -c -o $(DIR_O)/$@ $<

%.o: src/cpp/%.S
	$(CPP) $(INCLUDE_DIRS) $(CFLAGS) -c -o $(DIR_O)/$@ $<

java: Makefile
	exec $(ANT_HOME)/bin/ant -DJAVA_HOME=$(JAVA_HOME) -DJMF_HOME=$(JMF_HOME) -DFFMPEG_DIR=$(FFMPEG_DIR) -DCLASSPATH=$(CLASSPATH)

install:
	cp $(DIR_LIB)/$(JAR_FILE) "$(JMF_HOME)/lib"
	cp $(DIR_LIB)/$(LIBNAME) "$(JMF_HOME)/lib"

uninstall:
	$(RM) "$(JMF_HOME)/lib/$(JAR_FILE)"
	$(RM) "$(JMF_HOME)/lib/$(LIBNAME)"

clean:
	$(RM) -R *.class *.o
	$(RM) -R $(LIBNAME) $(JAR_FILE)
	$(RM) -R $(OUTPUT)

