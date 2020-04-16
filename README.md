## Electrical connections
### UART
```
esp8266 GPIO4/D2 3.3V --> Atmega2560 D17/TX2 5V (no converter required >3.0V is high)
esp8266 GPIO5/D1 3.3V <-- Atmega2560 D16/RX2 5V (with voltage devider to reduce below 3.6V)
```
