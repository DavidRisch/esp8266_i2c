#!/usr/bin/python3

import serial

with serial.Serial('/dev/ttyUSB0', 115200) as ser:
    while 1:
        val = ord(ser.read())
        if val in [ord("\n"), ord("\r")] or (ord(" ") <= val <= ord("~")):
            val = chr(val)
        else:
            val = "�"
        print(val, end="")
