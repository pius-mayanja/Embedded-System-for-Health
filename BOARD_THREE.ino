#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

//#define DISABLE_LED_FEEDBACK_FOR_RECEIVE // saves 108 bytes program space
#if FLASHEND <= 0x1FFF
#define EXCLUDE_EXOTIC_PROTOCOLS
#define EXCLUDE_UNIVERSAL_PROTOCOLS
#endif

#define MARK_EXCESS_MICROS    20 


#include "PinDefinitionsAndMore.h"

#include <IRremote.h>

#if defined(APPLICATION_PIN)
#define DEBUG_BUTTON_PIN    APPLICATION_PIN 
#else
#define DEBUG_BUTTON_PIN   6
#endif

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

// Initialize the LCD at address 0x27 with 16 columns and 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initialize SoftwareSerial for communication with Board Four
SoftwareSerial boardFourSerial(10, 11); // RX, TX

// Variables to store the temperature and heart rate values
uint32_t temperature = 0;
uint32_t heartRate = 0;

void setup() {

boardFourSerial.begin(9600);
#if defined(IR_MEASURE_TIMING) && defined(IR_TIMING_TEST_PIN)
    pinMode(IR_TIMING_TEST_PIN, OUTPUT);
#endif
#if FLASHEND > 0x1FFF 
    pinMode(DEBUG_BUTTON_PIN, INPUT_PULLUP);
#endif

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) || defined(ARDUINO_attiny3217)
    delay(4000); 
#endif

    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));


    Serial.println(F("Enabling IRin..."));


    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    Serial.print(F("Ready to receive IR signals at pin "));
#if defined(ARDUINO_ARCH_STM32) || defined(ESP8266)
    Serial.println(IR_RECEIVE_PIN_STRING);
#else
    Serial.println(IR_RECEIVE_PIN);
#endif

#if FLASHEND > 0x1FFF 
    Serial.print(F("Debug button pin is "));
    Serial.println(DEBUG_BUTTON_PIN);
#endif

    // infos for receive
    Serial.print(RECORD_GAP_MICROS);
    Serial.println(F(" us is the (minimum) gap, after which the start of a new IR packet is assumed"));
    Serial.print(MARK_EXCESS_MICROS);
    Serial.println(F(" us are subtracted from all marks and added to all spaces for decoding"));
    
    lcd.begin(16, 2, LCD_5x8DOTS); 
    lcd.backlight();               
    lcd.clear();
    lcd.setCursor(0, 0);  
}



void loop() {
    if (IrReceiver.decode()) {
        // Extract the address and command from the decoded data
        uint32_t address = IrReceiver.decodedIRData.address;
        uint32_t command = IrReceiver.decodedIRData.command;

        // Check the address and store the corresponding value
        if (address == 0x2F1C) {
            temperature = command;
        } else if (address == 0x2F1B) {
            heartRate = command;
        } else {
            Serial.println(F("Unknown address received."));
        }

        // Print the combined temperature and heart rate values to the serial monitor
        Serial.print(F("Temperature => "));
        Serial.print(temperature);

        // Update the LCD display for temperature
        lcd.setCursor(0, 0); 
        lcd.print("Temp => ");
        lcd.print(temperature); 

        Serial.print(F(" | Heart Rate => "));
        Serial.println(heartRate);

        // Update the LCD display for heart rate
        lcd.setCursor(0, 1); 
        lcd.print("HeartRate => ");
        lcd.print(heartRate); 

        // Clear any extra characters from previous values on the LCD
        lcd.print("     ");

            // Send data to Board Four
          boardFourSerial.print(temperature);
          boardFourSerial.print(",");
          boardFourSerial.println(heartRate);

        // Resume IR receiver for the next signal
        IrReceiver.resume();
    }
}


