geiger
======

Arduino project to count pulses from an 'Arduino compatible' Geiger counter radiation detection kit.

* geiger: geiger counter using an AVR-based Arduino, transmitting counts using a NRF24L01+ transceiver
* geiger_esp: geiger counter using an ESP8266, sending counts directly over wifi using MQTT

The particular geiger counter board that this was developed with is:
https://github.com/SensorsIot/Geiger-Counter-RadiationD-v1.1-CAJOE-

The counter is connected to a Wemos D1 mini ESP8266 board using the 3-pin connector (Vin, +5V, GND)
The Wemos D1 mini receives the tick and power signals from the geiger board on pins D4, GND, +5V.

Resistor R19 (470k) was bypassed with a 1k resistor to get a more reliable reading on the ESP8266.
A side effect is that can hear the counter tick while the ESP8266 is being programmed.

