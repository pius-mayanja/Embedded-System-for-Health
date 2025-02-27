#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <avr/interrupt.h>


// Pins
#define BUZZER_PIN 9
#define LED_PIN 8
#define RX_PIN 10
#define TX_PIN 11

const int switchPin = 7;    

// Debouncing variables
unsigned long lastDebounceTime = 0; 
const unsigned long debounceDelay = 50;
bool switchState = false;
bool lastSwitchReading = LOW; 

// Timer flag for periodic checking
volatile bool checkData = false;

// Timer interrupt service routine
ISR(TIMER1_COMPA_vect) {
    checkData = true; 
}

// Variables for temperature and heart rate
float receivedTemperature = 0.0;
int receivedHeartRate = 0;

// Healthy ranges
float normalTempMin;
float normalTempMax;
int normalHRMin;
int normalHRMax;

// Serial communication with Board Three
SoftwareSerial boardThreeSerial(RX_PIN, TX_PIN);

// Timers and flags
// volatile bool checkData = false;
String receivedData = ""; 


// Set up Timer1 for 1-second intervals
void setupTimer() {
    noInterrupts(); // Disable interrupts

    TCCR1A = 0;     
    TCCR1B = 0;    
    TCNT1 = 0;      
    // Set compare match register for 1Hz increments
    OCR1A = 15624; 
    TCCR1B |= (1 << WGM12); 
    TCCR1B |= (1 << CS12) | (1 << CS10); 
    TIMSK1 |= (1 << OCIE1A); 

    interrupts(); // Enable interrupts
}


void setup() {
    // Initialize Serial Monitor
    Serial.begin(9600);

    // Initialize SoftwareSerial
    boardThreeSerial.begin(9600);

    // Initialize pins
    pinMode(switchPin, INPUT_PULLUP); // Use internal pull-up resistor for the switch
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

    // Fetch healthy ranges from EEPROM
    fetchHealthyRanges();

    // Print healthy ranges
    Serial.println("Healthy Body Ranges:");
    Serial.print("Temperature: ");
    Serial.print(normalTempMin);
    Serial.print(" - ");
    Serial.println(normalTempMax);
    Serial.print("Heart Rate: ");
    Serial.print(normalHRMin);
    Serial.print(" - ");
    Serial.println(normalHRMax);

    // Set up Timer1 for 1-second interrupts
    setupTimer();
}

void loop() {
   // Read and debounce the switch
    bool currentSwitchReading = digitalRead(switchPin);
    if (currentSwitchReading != lastSwitchReading) {
        lastDebounceTime = millis(); 
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        // If the debounce delay has passed, update the switch state
        if (currentSwitchReading == LOW) {
            switchState = !switchState; 
        }
    }
    lastSwitchReading = currentSwitchReading; // Update last reading

        if (boardThreeSerial.available()) {
        char incomingByte = boardThreeSerial.read(); // Read incoming byte
        if (incomingByte == '\n') {
            // Parse the received data
            Serial.print("this is the recieved data ==> ");
            Serial.print(receivedData);
            Serial.print("\n");
            // parseData(receivedData);
            Serial.print("Incoming byte: ");
            Serial.println(incomingByte);

            // Check data every second (triggered by Timer1)
            if (checkData) {
                checkData = false; 
                Serial.print("Ready to parse: ");
                Serial.println(receivedData);

                parseData(receivedData);
            }
            receivedData = ""; 
        } else {
            receivedData += incomingByte; 
        }
    }

}


// Function to activate the alert system
void activateAlert() {
    digitalWrite(LED_PIN, HIGH); 
    digitalWrite(BUZZER_PIN, HIGH); 
    Serial.println("ALERT: Values out of range!");
}

// Function to deactivate the alert system
void deactivateAlert() {
    digitalWrite(LED_PIN, LOW);  
    digitalWrite(BUZZER_PIN, LOW);  
}

// EEPROM helper functions
float readFloatFromEEPROM(int address) {
    float value;
    byte* data = (byte*)&value;
    for (int i = 0; i < sizeof(float); i++) {
        data[i] = EEPROM.read(address + i);
    }
    return value;
}


int readIntFromEEPROM(int address) {
    byte lowByte = EEPROM.read(address);
    byte highByte = EEPROM.read(address + 1);
    return (highByte << 8) | lowByte; // Combine bytes
}

// Fetch healthy ranges from EEPROM
void fetchHealthyRanges() {
    normalTempMin = readFloatFromEEPROM(0);
    normalTempMax = readFloatFromEEPROM(4);
    normalHRMin = readIntFromEEPROM(8);
    normalHRMax = readIntFromEEPROM(12);
}

// Parse received data
void parseData(String data) {
    int separatorIndex = data.indexOf(','); 
    if (separatorIndex > 0) {
        // Extract temperature and heart rate values
        receivedTemperature = data.substring(0, separatorIndex).toInt();
        receivedHeartRate = data.substring(separatorIndex + 1).toInt();

        Serial.print("Received Temperature: ");
        Serial.print(receivedTemperature);
        Serial.print("Received Heart Rate: ");
        Serial.print(receivedHeartRate);
    }

    // Check if values are out of range and switch is ON
    if (switchState) {
        if (receivedTemperature < normalTempMin || receivedTemperature > normalTempMax ||
            receivedHeartRate < normalHRMin || receivedHeartRate > normalHRMax) {
            activateAlert();
        } else {
            deactivateAlert();
        }
    } else {
        deactivateAlert(); 
    }

}










