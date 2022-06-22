# esp32-battery-voltage-logger

A simple C++ program for the esp32 that logs battery voltage to non-volatile storage.

This is useful to calculate a voltage curve that can be used to more accurately convert between battery voltage and battery life percentage.

Values are logged as 12-bit ADC samples (0-4095). These values can be converted to battery voltage with *math*.

### How To Use
1. Fully charge your battery.
2. Compile and upload this program to your esp32.
3. Connect your esp32 to the battery. The esp32 will drain the battery and collect an adc sample about once every half hour.
4. Once the battery is drained, reconnect the esp32 to a computer and open the serial monitor. All collected samples will be printed to the serial monitor.
5. Copy these values to a spreadsheet and graph your voltage curve!
