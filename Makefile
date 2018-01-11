CC = gcc
LD = ld

TARGET = camera_test

CFLAGS := -Wall -O2 
CFLAGS += -I/usr/include/gtk-2.0 
CFLAGS += -I/usr/include/glib-2.0
CFLAGS += -I/usr/lib/glib-2.0/include
CFLAGS += -I/usr/lib/gtk-2.0/include
CFLAGS += -I/usr/include/cairo
CFLAGS += -I/usr/include/pango-1.0
CFLAGS += -I/usr/include/atk-1.0
CFLAGS += -I/usr/include/gdk-pixbuf-2.0

OBJS := main.o yuv422_rgb.o v4l2.o window.o
all: $(OBJS)
	$(CC)  -o $(TARGET) $(OBJS)  `pkg-config --cflags --libs gtk+-2.0`

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<  `pkg-config --cflags --libs gtk+-2.0`
clean:
	rm -rf *.o $(TARGET)

