# ------------------------------------------------------------------
# MQTT Client makefile
# ------------------------------------------------------------------
# Author:    taida
# Copyright: NXP B.V. 2016. All rights reserved
# ------------------------------------------------------------------
#-这个是最后一个makefile了

GTK_CFLAGS = $(shell pkg-config --cflags gtk+-2.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-2.0)

#-LDFLAGS += -lpthread
#CC=arm-none-linux-gnueabi-gcc
CC=gcc

TARGET = v4l2t
#-包含头文件的地方
INCLUDES = -I.
OBJECTS = main.o


%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -Wall -g -c $< -o $@ $(GTK_CFLAGS)

all: clean build

build: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET) $(GTK_LIBS)
#	cp $(TARGET) ../../swupdate/images/usr/bin/

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
#	-rm -f ../../swupdate/images/usr/bin/$(TARGET)

