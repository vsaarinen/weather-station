# An Arduino weather station

A solar-powered station that sends weather data (temperature, humidity, pressure, rainfall) via GPRS.

Built with:

- [Arduino Uno](https://www.arduino.cc/en/Main/arduinoBoardUno)
- [WiFi shield](https://www.arduino.cc/en/Main/ArduinoWiFiShield) or [Geeetech GPRS 2.0 shield](http://www.dx.com/p/geeetech-updated-gprs-gsm-sim900-shield-v2-0-compatible-with-arduino-green-367958)
- [DHT11](http://www.dx.com/p/temperature-humidity-sensor-dht11-module-for-arduino-deep-blue-works-with-official-arduino-board-288612) for humidity
- [BMP085](http://www.dx.com/p/bmp085-barometric-pressure-height-sensor-module-for-arduino-blue-148612) for barometric pressure
- [DS18B20](http://www.dx.com/p/ds18b20-waterproof-digital-temperature-probe-black-silver-204290) for temperature
- TFA 30.3512 tipping bucket rain gauge for rain
- A [solar-powered USB battery pack](http://www.dx.com/p/3000mah-solar-powered-dual-usb-external-battery-power-bank-w-led-indicator-flashlight-black-279135) for power
- [ThingSpeak](https://thingspeak.com) for storing/visualizing data

## Schematic

![Arduino schematic](schematic_bb.png?raw=true)

## TODO:

- [x] Get DHT11 working
- [x] Get DS18B20 working
- [ ] Get BMP085 working
- [ ] Get rain gauge working
- [x] Get WiFi working
- [ ] Get GPRS working
- [ ] Make sure the battery provides enough power
- [ ] Build the box

## Notes along the way

- The firmware of the WiFi shield needs to be updated if you want it to connect to any server when using version >1.0.2 of the Arduino IDE. Otherwise the shield will connect to the WiFi network, but all calls to `client.connect()` fail.
