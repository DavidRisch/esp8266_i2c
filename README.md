## Electrical connections
### UART
```
esp8266_B GPIO04/D2 3.3V --> Atmega2560 D17/TX2 5V (no converter required >3.0V is read as high)
esp8266_B GPIO05/D1 3.3V <-- Atmega2560 D16/RX2 5V (with voltage devider to reduce below 3.6V)
```

### I²C
```
# SCL:
esp8266_A GPIO14/D5 <-> esp8266_B GPIO14/D5
# SDA:
esp8266_A GPIO12/D6 <-> esp8266_B GPIO12/D6
```

### User interface
```
esp8266_A ADC0/A0   <-- Potentimeter(0V - 3.3V)
esp8266_A GPIO05/D1 <-- Button "Right" (normally open, connected to GND)
esp8266_A GPIO04/D2 <-- Button "Left"  (normally open, connected to GND)
esp8266_A GPIO00/D3 <-- Button "Home"  (normally open, connected to GND)
esp8266_A GPIO13/D7 --> Led "Ready"    (through resistor)
esp8266_A GPIO15/D8 --> Led "Ready"    (through resistor)
```

## Explanation of the I²C protocol

![alt text]https://github.com/DavidRisch/esp8266_i2c/blob/master/i2c_protocol.png?raw=true)
