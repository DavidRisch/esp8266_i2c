#!/usr/bin/python3

import sys

import serial

dev = '/dev/ttyUSB0'
if (len(sys.argv) >= 2):
    dev = sys.argv[1]
baudrate = 230400
print("listening on", dev, "at", baudrate)

with serial.Serial(dev, baudrate) as ser:
    while 1:
        val = ord(ser.read())
        if val in [ord("\n"), ord("\r"), ord("\t")] or (ord(" ") <= val <= ord("~")):
            val = chr(val)
        else:
            val = "�"
        print(val, end="")
