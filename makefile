FLAGS=`pkg-config --cflags --libs libdrm`
FLAGS+=-Wall -O0 -g
FLAGS+=-D_FILE_OFFSET_BITS=64

all:
	gcc hdmi_info.c -o hdmi_info $(FLAGS)