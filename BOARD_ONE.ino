#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4; 
byte rates[RATE_SIZE];    
byte rateSpot = 0;
long lastBeat = 0;        

float beatsPerMinute;
int beatAvg;

void setup() {
  Serial.begin(9600); 
  Wire.begin();

  // Initialize MAX30102
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found. Check connections.");
    while (1);
  }
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0); 

  Serial.println("Board 1 Initialized");
  Serial.println("Place your index finger on the sensor with steady pressure.");
}

void loop() {
  // Read pulse and temperature
  int temperature = getTemperature();
  int heartRate = readHeartRate();

  // Format data
  String data = String(heartRate) + "," + String(temperature);
  Serial.println(data); 
  // delay(1000); 
}

int readHeartRate() {
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  // Return the average heart rate value
  return beatAvg;
}
int getTemperature() {
  int analogValue = analogRead(A0); 
  float voltage = analogValue * (5.0 / 1023.0); // Convert to voltage
  float tempC = ((voltage - 0.5) * 100.0) + 16; 
  int TMP = (int)tempC;
  // return 35;
  return TMP;
}
// 