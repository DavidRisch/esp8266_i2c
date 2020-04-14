# From: https://github.com/pfalcon/esp-open-sdk

CC = xtensa-lx106-elf-gcc
CFLAGS = -I. -mlongcalls
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -Wl,--end-group -lgcc
LDFLAGS = -Teagle.app.v6.ld

blink-0x00000.bin: blink
	esptool.py elf2image $^

blink: blink.o

blink.o: blink.c
