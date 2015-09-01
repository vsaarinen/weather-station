#include <SPI.h>
#include <WiFi.h>
#include <dht11.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

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
const long updateThingSpeakInterval = 60 * 1000L;   // Time interval in milliseconds to update ThingSpeak
long lastConnectionTime = -updateThingSpeakInterval; // update straight away
boolean lastConnected = false;
int failedCounter = 0;

// WiFi
int status = WL_IDLE_STATUS;      // the Wifi radio's status
WiFiClient client;

// Sensitive data
char ssid[] = "YOUR_SSID";          // your network SSID (name)
char pass[] = "YOUR_NETWORK_PASSWORD";   // your network password
String thingSpeakWriteApiKey = "YOUR_API_KEY";

void wifiConnect(){
  Serial.print("... Connecting to WiFi...");
  status = WiFi.begin(ssid, pass);

  while (status != WL_CONNECTED) {
    delay(10000);
    Serial.println("Not successful. Retrying...");
    status = WiFi.begin(ssid, pass);
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

void setup()
{
  Serial.begin(9600);

  wifiConnect();
  printWifiStatus();

  Serial.print("DHT11 LIB VER: ");
  Serial.println(DHT11LIB_VERSION);
  Serial.print("DALLAS TEMP LIB VER: ");
  Serial.println(DALLASTEMPLIBVERSION);

  temperatureSensor.begin();
  temperatureSensor.setResolution(TEMP_11_BIT); // 12 bits takes ~750mx, 11 bits ~375ms

  if (!bmp.begin()) {
    Serial.println("Could not find BMP085 sensor!");
    bmpAvailable = false;
  }

  if (DEBUG) {
    Serial.println("Setup() complete");
  }
}

int humidity;
float temperature, bmpTemperature;
long pressure;

void loop()
{
  if (DEBUG) {
    printIncomingData();
  }

  // Read sensor data
  int chk = DHT11.read(DHT11PIN);
  humidity = DHT11.humidity;
  temperatureSensor.requestTemperatures();
  temperature = temperatureSensor.getTempCByIndex(0);
  if (bmpAvailable) {
    bmpTemperature = bmp.readTemperature();
    pressure = bmp.readPressure();
  }

  if (DEBUG) {
    printSensorData(chk);
  }

  // Disconnect from ThingSpeak
  if (!client.connected() && lastConnected) {
    client.stop();
  }

  // Update ThingSpeak
  if(!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval))
  {
    updateThingSpeak("1="+String(temperature)+"&2="+String(humidity));
  }

  // Check if Arduino WiFi needs to be restarted
  if (failedCounter > 3 ) {
    wifiConnect();
  }

  lastConnected = client.connected();

  delay(loopDelay);
}

void updateThingSpeak(String tsData)
{
  if (client.connect(thingSpeakServer, 80))
  {
    if (DEBUG) {
      Serial.println("... Connected, POSTing...");
    }

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+thingSpeakWriteApiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
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

      Serial.println("... Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");
      Serial.println();
    }
  }
  else
  {
    failedCounter++;

    Serial.println("... Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");
    Serial.println();
  }
}


void printIncomingData() {
  if (client.available()) {
    char c;
    String currentLine = "";

    while (client.available()) {
      c = client.read();

      if (c == '\n') {
        if (currentLine == "\r") {
          currentLine = "";

          // parse the HTTP body
          while (client.available()) {
            c = client.read();
            currentLine += c;
          }

          Serial.println("line: " + currentLine);

          currentLine = "";
          client.flush();
          break;
        } else {
          Serial.println(currentLine);
          currentLine = "";
        }
      } else {
        currentLine += c;
      }
    }
  }
}

void printSensorData(int dhtStatus) {
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
  Serial.print("DHT11 Temperature (°C): ");
  Serial.println(DHT11.temperature);
  
  Serial.print("DS18B20 Temperature (°C): ");
  Serial.println(temperature);

  if (bmpAvailable) {
    Serial.print("Temperature = ");
    Serial.print(bmpTemperature);
    Serial.println(" *C");
    
    Serial.print("Pressure = ");
    Serial.print(pressure);
    Serial.println(" Pa");
    
    // Calculate altitude assuming 'standard' barometric
    // pressure of 1013.25 millibar = 101325 Pascal
    Serial.print("Altitude = ");
    Serial.print(bmp.readAltitude());
    Serial.println(" m");

    Serial.print("Pressure at sealevel (calculated) = ");
    Serial.print(bmp.readSealevelPressure());
    Serial.println(" Pa");

    // you can get a more precise measurement of altitude
    // if you know the current sea level pressure which will
    // vary with weather and such. If it is 1015 millibars
    // that is equal to 101500 Pascals.
    Serial.print("Real altitude = ");
    Serial.print(bmp.readAltitude(101500));
    Serial.println(" m");
  }
}

