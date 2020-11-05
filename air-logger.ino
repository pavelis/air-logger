/* 
  Autonomous air pollution logger for a drone
*/

#include <Wire.h> // Built-in library
#include <Adafruit_BME280.h> // https://github.com/adafruit/Adafruit_BME280_Library
#include <SoftwareSerial.h> // Built-in library
#include <PMS.h> // https://github.com/fu-hsi/pms
#include <SPI.h> // Built-in library
#include <SD.h> // Built-in library

#define SEALEVELPRESSURE_HPA (1013.25) // standard pressure on sea level
#define DELAY_BETWEEN_MEASUREMENTS 2000 // delay between measurements in microseconds

Adafruit_BME280 bme;
int initialAltitude = 0;
SoftwareSerial PMSSerial(2, 3); // Pins for RX and TX
PMS pms(PMSSerial);
PMS::DATA data;
String filename;
File datafile;

void setup() {
  int i = 0;
  Serial.begin(9600);
  if (!bme.begin(0x76)) {
    Serial.println(F("Could not find a valid BME280 sensor!"));
  }
  
  PMSSerial.begin(9600);

  // Set SS/SC pin of an Arduino. For Pro Mini, it's better to use 10.
  if (!SD.begin(10)) {
    Serial.println(F("SD initialization failed!"));
  }
  Serial.println(F("SD initialized."));

  // Incremental file name search
  while (1) {
    filename = "data" + String(i) + ".txt";
    if (!SD.exists(filename)) {
      Serial.println("Using " + filename);
      break;
    }
    else {
      i++;
    }
  }

  datafile = SD.open(filename, FILE_WRITE);
  if (datafile) {
    datafile.println(F("Seconds;Altitude_m;Temperature_degC;Humidity_%;Pressure_hPa;PM1_ug/m3;PM2.5_ug/m3;PM10_ug/m3"));
    datafile.close();
  }
  else
    Serial.println("Error opening " + filename);

  Serial.println(F("Seconds;Altitude_m;Temperature_degC;Humidity_%;Pressure_hPa;PM1_ug/m3;PM2.5_ug/m3;PM10_ug/m3"));
  delay(5000);
}

void loop() {
  int humidity, altitude, delta;
  float temperature, pressure;

  if (pms.read(data)) {
    temperature = bme.readTemperature();
    pressure = bme.readPressure() / 100;
    humidity = int(bme.readHumidity());
    altitude = int(bme.readAltitude(SEALEVELPRESSURE_HPA)); // not real altitude, just suggesting pressure is standard

    if (initialAltitude == 0)
      initialAltitude = altitude;

    // Delta is an elevation from start point
    delta = altitude - initialAltitude;

    Serial.println(String(millis()/1000) + ";" + String(delta) + ";" + String(temperature) + ";" + String(humidity) + ";" + String(pressure) + ";" + String(data.PM_AE_UG_1_0) + ";" + String(data.PM_AE_UG_2_5) + ";" + String(data.PM_AE_UG_10_0));

    datafile = SD.open(filename, FILE_WRITE);
    if (datafile) {
      datafile.println(String(millis()/1000) + ";" + String(delta) + ";" + String(temperature) + ";" + String(humidity) + ";" + String(pressure) + ";" + String(data.PM_AE_UG_1_0) + ";" + String(data.PM_AE_UG_2_5) + ";" + String(data.PM_AE_UG_10_0));
      datafile.close();
    }
    else
      Serial.println("Error opening " + filename);

    delay(DELAY_BETWEEN_MEASUREMENTS);
  }

}
