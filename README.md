Arduino-Temp-Hum-Logger
=================

Arduino temperature and humidity logger is a wireless temperature and humidity measurement unit which will send measured
temperature and humidity values via 433MHz RF link to Arduino gateway. The Arduino gateway will pass the measurement
values to Domoticz via MQTT protocol.

Arduino-Temp-Hum-Logger sketch will need following HW and SW libraries to work:

**HW**

* Arduino
* DHT22/RHT03 temperature and humidity sensor
* DS1307 RTC
* TX433N RF transmitter

**Libraries**

* DHT for DHT22/RHT03 temperature and humidity sensor
* Wire for I2C communication
* VirtualWire for RF communication
* RTClib for RTC functionality
* Narcoleptic for sleep functionality

**HW connections**

DS1307 RTC is connected to Arduino via I2C bus.
Temperature and humidity readings are read from DHT22/RHT03 digital temperature and humidity sensor via
MaxDetect's 1-wire bus. TX433N RF transmitter is connected to the Arduino via single GPIO pin.

This repository includes also breadboard and schematics pictures in PDF and Fritzing formats.

**Functionality of the sketch**

Sketch reads current time from the RTC clock and checks is it time to perform temperature and humidity measurement functions.
After the temperature and humidity is measured, values will be added to the data string which is sent to Arduino based gateway via 433MHz RF link 
which will then send the data to the MQTT server. So this sketch doesn't send temperature and humidity data directly to the MQTT server.

There is also a sleep function in main loop which will decrease overall power consumption.

It's possible to print amount of free RAM memory of Arduino via serial port by uncommenting `#define RAM_DEBUG` line