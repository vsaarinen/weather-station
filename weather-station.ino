#include <SPI.h>
#include <WiFi.h>
#include <dht11.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <MemoryFree.h>

#define DEBUG false
const long loopDelay = 60 * 1000L; // in milliseconds

// BMP085 atmospheric pressure gauge
// Connect VCC of the BMP085 sensor to 3.3V (NOT 5.0V!)
// Connect GND to Ground
// Connect SCL to i2c clock - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 5
// Connect SDA to i2c data - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 4
// EOC is not used, it signifies an end of conversion
// XCLR is a reset pin, also not used here
Adafruit_BMP085 bmp;
boolean bmpAvailable = true;

// DS18B20 temperature sensor
// Data wire is plugged into pin 8 on the Arduino
#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temperatureSensor(&oneWire);

// DHT11 humidity (+ temperature) sensor plugged into pin 2
#define DHT11PIN 2
dht11 DHT11;

// ThingSpeak
char thingSpeakServer[] = "api.thingspeak.com";
const long updateThingSpeakInterval = 5 * 60 * 1000L;   // Time interval in milliseconds to update ThingSpeak
long lastConnectionTime = -updateThingSpeakInterval; // update straight away
int failedCounter = 0;

// WiFi
WiFiClient client;

// Sensitive data
String thingSpeakWriteApiKey = "YOUR_API_KEY";
#define SSID_NETWORK "WIFINAME"
#define SSID_PASS "WIFIPASS"

// Weather data
int humidity, dhtStatus;
float temperature, bmpTemperature;
long pressure;
#define RAIN_FACTOR 0.2794 // each click is 0,2794 mm of rain
volatile int rainClicks = 0; 
volatile long lastRainClick;

void setup()
{
  Serial.begin(9600);

  wifiConnect();
  printWifiStatus();

  Serial.print("DHT11 LIB VER: ");
  Serial.println(DHT11LIB_VERSION);
  Serial.print("DALLAS TEMP LIB VER: ");
  Serial.println(DALLASTEMPLIBVERSION);

  attachInterrupt(1, increaseRainCount, RISING); // Interrupt #1 is pin D3

  temperatureSensor.begin();
  temperatureSensor.setResolution(TEMP_11_BIT); // 12 bits takes ~750ms, 11 bits ~375ms

  if (!bmp.begin()) {
    Serial.println("Can't find BMP085 sensor!");
    bmpAvailable = false;
  }

  if (DEBUG) {
    Serial.println("Setup() complete");
  }
}

void loop()
{
  // Read sensor data
  dhtStatus = DHT11.read(DHT11PIN);
  humidity = DHT11.humidity;

  temperatureSensor.requestTemperatures();
  temperature = temperatureSensor.getTempCByIndex(0);

  if (bmpAvailable) {
    bmpTemperature = bmp.readTemperature();
    pressure = bmp.readPressure();
  }

  if (DEBUG) {
    printSensorData();
  }

  if (!client.connected()) {
    if (millis() - lastConnectionTime > updateThingSpeakInterval) { // time to update
      updateThingSpeak("1="+String(temperature)+"&2="+String(humidity)+"&3="+String(pressure)+"&4="+String(rainClicks*RAIN_FACTOR)+"&5="+String(freeMemory()));
      rainClicks = 0;
    }
  }

  delay(loopDelay);
}

void wifiConnect(){
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    
    while(true) // don't continue
      ;
  }
  
  Serial.print("... Connecting to WiFi...");
  int wifiStatus = WiFi.begin(SSID_NETWORK, SSID_PASS);

  while (wifiStatus != WL_CONNECTED) {
    delay(10000);
    Serial.println("Not successful. Retrying...");
    wifiStatus = WiFi.begin(SSID_NETWORK, SSID_PASS);
  }

  Serial.println("Joined.");
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("Strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void increaseRainCount() {
  long thisTime = micros() - lastRainClick;
  lastRainClick = micros();
  if (thisTime > 1000) { // debounce of 1ms
    rainClicks++;
    if (DEBUG) {
      Serial.print("Rain! ");
      Serial.println(rainClicks);
    }
  }
}

void updateThingSpeak(String tsData)
{
  if (client.connect(thingSpeakServer, 80))
  {
    if (DEBUG) {
      Serial.println("... Connected, POSTing: " + tsData);
    }

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: ");
    client.print(thingSpeakWriteApiKey);
    client.print("\nContent-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");

    client.print(tsData);

    lastConnectionTime = millis();

    if (client.connected())
    {
      failedCounter = 0;
    }
    else
    {
      failedCounter++;

      Serial.println("... Unable to POST ("+String(failedCounter, DEC)+")");
      Serial.println();
    }
  }
  else
  {
    failedCounter++;

    Serial.println("... Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");
    Serial.println();
  }

  while (client.connected()) {
    client.flush();
  }

  client.stop();

  // Check if Arduino WiFi needs to be restarted
  if (failedCounter > 4) {
    if ((WiFi.status() != WL_CONNECTED) || (failedCounter > 14)) {
      failedCounter = 0;
      Serial.println("... Disconnecting wifi.");
      WiFi.disconnect();
      wifiConnect();
    }
  }
}

void printSensorData() {
  Serial.print("DHT11 sensor: ");
  switch (dhtStatus)
  {
    case DHTLIB_OK:
    Serial.println("OK");
    break;
    case DHTLIB_ERROR_CHECKSUM:
    Serial.println("Checksum error");
    break;
    case DHTLIB_ERROR_TIMEOUT:
    Serial.println("Time out error");
    break;
    default:
    Serial.println("Unknown error");
    break;
  }
  Serial.print("DHT11 Humidity (%): ");
  Serial.println(humidity);
  Serial.print("DHT11 Temperature (C): ");
  Serial.println(DHT11.temperature);

  Serial.print("DS18B20 Temperature (C): ");
  Serial.println(temperature);

  if (bmpAvailable) {
    Serial.print("BMP085 Temperature (C): ");
    Serial.print(bmpTemperature);
    Serial.println(" *C");

    Serial.print("BMP085 Pressure (Pa): ");
    Serial.print(pressure);
    Serial.println(" Pa");
  }
}

