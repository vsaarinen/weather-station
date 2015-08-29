# An Arduino weather station

Built with:

- Arduino Uno
- WiFi shield or Geeetech GPRS 2.0 shield
- DHT11 for humidity
- BMP085 for barometric pressure
- DS18B20 for temperature
- TFA 30.3512 tipping bucket rain gauge for rain
- A solar-powered USB batter pack for power
- [ThinkSpeak](https://thingspeak.com) for storing/visualizing data

![Arduino schematic](schematic_bb.png?raw=true)

### TODO:

- [x] Get DHT11 working
- [x] Get DS18B20 working
- [ ] Get BMP085 working
- [ ] Get rain gauge working
- [x] Get WiFi working
- [ ] Get GPRS working
- [ ] Make sure the battery provides enough power
- [ ] Build the box

### Notes along the way

- The firmware of the WiFi shield needs to be updated if you want it to connect to any server when using version >1.0.2 of the Arduino IDE. Otherwise the shield will connect to the WiFi network, but all calls to `client.connect()` fail.
