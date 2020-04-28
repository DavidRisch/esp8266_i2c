## Table of Contents

1. [Chip](#chip)
2. [Electrical connections](#electrical-connections)
    1. [UART](#uart)
    2. [I2C](#i2c)
    3. [User interface](#user-interface)
3. [Explanation of the I2C protocol](#explanation-of-the-i2c-protocol)
    1. [Differences between this implementation and the actual protocol](#differences-between-this-implementation-and-the-actual-protocol)
4. [Example Setup](#example-setup)

## Chip
The software is written for the **ESP8266** microchip, produced by *Espressif Systems*.

More information about how to control the hardware of a ESP8266: [Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp8266-technical_reference_en.pdf)  
Specification of the chip: [Datasheet](https://www.espressif.com/sites/default/files/documentation/0a-esp8266ex_datasheet_en.pdf)  

## Electrical connections
### UART
Atmega2560 D17/TX2 5V (no converter required >3.0V is read as high):  
&nbsp;&nbsp;&nbsp;&nbsp;`esp8266_B GPIO04/D2 3.3V`  
Atmega2560 D16/RX2 5V (with voltage divider to reduce below 3.6V):  
&nbsp;&nbsp;&nbsp;&nbsp;`esp8266_B GPIO05/D1 3.3V`  

### I2C
SCL:  
&nbsp;&nbsp;&nbsp;&nbsp;`esp8266_A GPIO14/D5 <-> esp8266_B GPIO14/D5`  
SDA:  
&nbsp;&nbsp;&nbsp;&nbsp;`esp8266_A GPIO12/D6 <-> esp8266_B GPIO12/D6`  

### User interface
Potentiometer (0V - 3.3V):  
&nbsp;&nbsp;&nbsp;&nbsp;`esp8266_A ADC0/A0`  
Button "Right" (normally open, connected to GND):  
&nbsp;&nbsp;&nbsp;&nbsp;`esp8266_A GPIO05/D1`  
Button "Left" (normally open, connected to GND):  
&nbsp;&nbsp;&nbsp;&nbsp;`esp8266_A GPIO04/D2`  
Button "Home" (normally open, connected to GND):  
&nbsp;&nbsp;&nbsp;&nbsp;`esp8266_A GPIO00/D3`  
LED "Ready" (through resistor):  
&nbsp;&nbsp;&nbsp;&nbsp;`esp8266_A GPIO13/D7`  
LED "Ready" (through resistor):  
&nbsp;&nbsp;&nbsp;&nbsp;`esp8266_A GPIO15/D8`  

## Explanation of the I2C protocol
I²C sends data in different messages. One message can consist of multiple data frames, each consisting of exactly one byte / 8 bit.  
Each data frame is followed by an acknowledgement (ACK) bit.  
A 7-Bit long address is sent at begin of each message to indicate which device is being communicated to.  
A message starts with a start symbol and ends with a stop symbol.  

![I2C Protocol](https://github.com/DavidRisch/esp8266_i2c/blob/master/i2c_protocol.png?raw=true)

1. Start symbol
2. Address (7-Bit)
3. Read-Write-Bit
4. Acknowledgement bit
5. Data frame
6. Acknowledgement bit  
.  
.&nbsp;&nbsp;&nbsp;&nbsp;[Step 5 and 6 can repeat]  
.  
7. Stop symbol

### Differences between this implementation and the actual protocol
Following features are not yet implemented:
- no repeated start
- no fast mode
- no master-to-master connection

## Example Setup
The master is located on the right side and the slave on the left.  

![Setup](https://github.com/DavidRisch/esp8266_i2c/blob/master/setup.jpg?raw=true)
