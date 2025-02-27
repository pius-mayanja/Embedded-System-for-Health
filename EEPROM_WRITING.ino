#include <EEPROM.h>

// EEPROM addresses to store temperature and heart rate ranges
#define TEMP_MIN_ADDR 0
#define TEMP_MAX_ADDR 4
#define HR_MIN_ADDR   8
#define HR_MAX_ADDR   12

// Normal ranges for body temperature and heart rate
float normalTempMin = 32.0;
float normalTempMax = 40.0;
int normalHRMin = 50;
int normalHRMax = 100;

void setup() {
    Serial.begin(9600);

    // Write data to EEPROM (only when necessary)
    writeToEEPROM();

    // Read data back from EEPROM
    float tempMin = readFloatFromEEPROM(TEMP_MIN_ADDR);
    float tempMax = readFloatFromEEPROM(TEMP_MAX_ADDR);
    int hrMin = readIntFromEEPROM(HR_MIN_ADDR);
    int hrMax = readIntFromEEPROM(HR_MAX_ADDR);

    // Print the retrieved ranges
    Serial.println("Healthy Body Ranges:");
    Serial.print("Temperature Range: ");
    Serial.print(tempMin);
    Serial.print(" - ");
    Serial.println(tempMax);
    Serial.print("Heart Rate Range: ");
    Serial.print(hrMin);
    Serial.print(" - ");
    Serial.println(hrMax);
}

void loop() {
    // Nothing here
}

// Function to write data into EEPROM
void writeToEEPROM() {
    Serial.println("Writing healthy ranges to EEPROM...");
    writeFloatToEEPROM(TEMP_MIN_ADDR, normalTempMin);
    writeFloatToEEPROM(TEMP_MAX_ADDR, normalTempMax);
    writeIntToEEPROM(HR_MIN_ADDR, normalHRMin);
    writeIntToEEPROM(HR_MAX_ADDR, normalHRMax);
    Serial.println("Data written successfully.");
}

// Function to write a float value into EEPROM
void writeFloatToEEPROM(int address, float value) {
    byte* data = (byte*)&value;
    for (int i = 0; i < sizeof(float); i++) {
        EEPROM.write(address + i, data[i]);
    }
}

// Function to read a float value from EEPROM
float readFloatFromEEPROM(int address) {
    float value;
    byte* data = (byte*)&value;
    for (int i = 0; i < sizeof(float); i++) {
        data[i] = EEPROM.read(address + i);
    }
    return value;
}

// Function to write an integer value into EEPROM
void writeIntToEEPROM(int address, int value) {
    byte lowByte = value & 0xFF;       // Extract lower byte
    byte highByte = (value >> 8) & 0xFF; // Extract higher byte
    EEPROM.write(address, lowByte);
    EEPROM.write(address + 1, highByte);
}

// Function to read an integer value from EEPROM
int readIntFromEEPROM(int address) {
    byte lowByte = EEPROM.read(address);
    byte highByte = EEPROM.read(address + 1);
    return (highByte << 8) | lowByte; // Combine bytes to form integer
}
