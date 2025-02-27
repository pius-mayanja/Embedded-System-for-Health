#include <SoftwareSerial.h>
#include <string.h> 

// Pins for SoftwareSerial 
#define RX_PIN 10
#define TX_PIN 11

SoftwareSerial Board1Serial(RX_PIN, TX_PIN); 

void setup() {
  Serial.begin(9600);       
  Board1Serial.begin(9600); 
  Serial.println("Board Two Initialized. Waiting for data from Board One...");
}

void loop() {
  // Check if data is available from Board One
  if (Board1Serial.available() > 0) {
    String data = Board1Serial.readStringUntil('\n'); 
    Serial.print("Received from Board One: ");
    Serial.println(data);

    // Process and send the data via IR
    if (isValidDataFormat(data)) {
      processAndSendData(data);
    } else {
      Serial.println("Invalid data received. Skipping...");
    }
    delay(1000); 
  }
}

// Function to validate data format
bool isValidDataFormat(String data) {
  // Check if data contains a comma
  int commaIndex = data.indexOf(',');
  if (commaIndex == -1) {
    return false; 
  }

  // Separate temperature and heart rate strings
  String tempString = data.substring(0, commaIndex);
  String heartRateString = data.substring(commaIndex + 1);

  // Convert strings to integers
  int temperature = tempString.toInt();
  int heartRate = heartRateString.toInt();

  // Ensure values are within the valid range (0-255)
  if (temperature < 0 || temperature > 255 || heartRate <= 0 || heartRate > 255) {
    return false; 
  }

  return true; 

// Function to process incoming data, convert to hexadecimal, and send via IR
void processAndSendData(String data) {
  // Separate temperature and heart rate
  int commaIndex = data.indexOf(',');
  String heartRateString = data.substring(0, commaIndex);
  String tempString = data.substring(commaIndex + 1);

  // Convert strings to integers
  int temperature = tempString.toInt();
  int heartRate = heartRateString.toInt();

  // Convert to hexadecimal for debugging
  String tempHex = decToHexa(temperature);
  String heartRateHex = decToHexa(heartRate);

  // Display debug information
  Serial.print("Temperature in Hex: ");
  Serial.println(tempHex);
  Serial.print("Heart Rate in Hex: ");
  Serial.println(heartRateHex);

  // Convert to actual hexadecimal (uint8_t) values for transmission
  uint8_t tempValue = (uint8_t)temperature;
  uint8_t heartRateValue = (uint8_t)heartRate;

  // Send temperature
  sendIR(0x1C, tempValue);
  
  delay(1000);
  // Send heart rate
  sendIR(0x1B, heartRateValue);
}

// Function to send data via YS-IRTM IR Transmitter
void sendIR(uint8_t userCode1, uint8_t command) {
  Serial.print("Transmitting via IR: UserCode1=0x");
  Serial.print(userCode1, HEX);
  Serial.print(", Command=0x");
  Serial.println(command, HEX);

  uint8_t userCode2 = 0x2F; 
  uint8_t inverseCommand = ~command; 

  uint8_t packet[] = {0xA1, 0xF1, userCode1, userCode2, command, inverseCommand};

  // Send the data packet via Serial to the YS-IRTM module
  Serial.write(packet, sizeof(packet)); 
  Serial.println("Data sent via IR.");
}

// Function to check if a string contains only numeric characters
bool isNumeric(String str) {
  for (int i = 0; i < str.length(); i++) {
    if (!isDigit(str[i])) {
      return false; 
    }
  }
  return true; 
}

// Function to convert a decimal number to hexadecimal
String decToHexa(int decimalNumber) {
  int i = 0, temp;
  char hexaNumber[10];

  // If the decimal number is zero, return "0"
  if (decimalNumber == 0) {
    return "0";
  }

  // Loop to convert decimal to hexadecimal
  while (decimalNumber != 0) {
    temp = decimalNumber % 16;

    // Converting the remainder into hexadecimal characters
    if (temp < 10) {
      temp = temp + 48; 
    } else {
      temp = temp + 55; 
    }

    hexaNumber[i++] = temp; 
    decimalNumber = decimalNumber / 16;
  }

  hexaNumber[i] = '\0'; 

  // Reverse the string since hexadecimal digits are calculated in reverse order
  String hexString = "";
  for (int j = i - 1; j >= 0; j--) {
    hexString += hexaNumber[j];
  }

  return hexString;
}
