// Libraries
#include <SoftwareSerial.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Constants
const int waterSensorPin = A0; // Changed to A0 for the water level sensor
const int buzzerPin = 9;        // Changed to Digital Pin 9 for the buzzer
const int waterLevelThreshold = 10; // Water level threshold
bool waterDetected = false;
unsigned long buzzStartTime;  // To track when the buzzer started buzzing
const unsigned long buzzDuration = 15000; // 15 seconds in milliseconds

// GSM serial setup
SoftwareSerial mySerial(10, 11); // RX, TX

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16x2 display

#define DHTPIN A3       // Pin connected to the DHT11 data pin
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

int h;  // Stores humidity value
int t;  // Stores temperature value

void setup() {
    Serial.begin(9600);
    mySerial.begin(9600);
    dht.begin();
    pinMode(waterSensorPin, INPUT);
    pinMode(buzzerPin, OUTPUT);
    
    lcd.init(); // Initialize the LCD
    lcd.backlight(); // Turn on the backlight
    
    Serial.println("System Initializing..."); 
    delay(1000);
}

void loop() {
    // Read humidity and temperature
    h = dht.readHumidity();    // Read humidity
    t = dht.readTemperature();  // Read temperature in Celsius

    // Check for DHT reading failure
    if (isnan(h) || isnan(t)) {
        return; // Skip sending if read failed
    }

    // Check humidity for SMS alert
    if (h > 84) { // Assuming 80% humidity indicates a condition to alert
        SendMessage("WARNINGG!!.. POSSIBLE RAINFALL HAS BEEN DETECTED!..");
    } 

    // Water level detection
    int sensorValue = analogRead(waterSensorPin);
    if (sensorValue > waterLevelThreshold) {
        if (!waterDetected) {
            waterDetected = true; // Update state
            digitalWrite(buzzerPin, HIGH); // Turn on buzzer
            buzzStartTime = millis(); // Record the time when water is detected
            SendMessage("WARNING!!... FLUSHFLOOD DETECTED!!.. IN LUMANGOG RIVER");
        }
    } else {
        if (waterDetected) {
            waterDetected = false; // Update state
            digitalWrite(buzzerPin, LOW); // Turn off buzzer
        }
    }

    // Ensure buzzer stays on for 15 seconds if water is detected
    if (waterDetected) {
        if (millis() - buzzStartTime < buzzDuration) {
            digitalWrite(buzzerPin, HIGH); // Keep buzzer on
        } else {
            digitalWrite(buzzerPin, LOW); // Turn off buzzer after 15 seconds
            waterDetected = false; // Reset the water detected flag
        }
    }

    // Handle GSM commands from Serial Monitor
    if (Serial.available() > 0) {
        char command = Serial.read();
        switch (command) {
            case 's':
                SendMessage("WARNING!!.. FLUSHFLOOD DETECTED!!..IN LUMANGOG RIVER");
                break;
            case 'r':
                RecieveMessage();
                break;
            case 'c':
                CallNumber();
                break;
        }
    }

    // Read GSM responses
    if (mySerial.available()) {
        delay(1000); // Give it some time to read the response
        Serial.println(mySerial.readString());
    }

    // Display temperature and humidity on LCD
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(t);
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(h);
    lcd.print("%");

    delay(2000); // Wait before the next loop
}

void SendMessage(String message) {
    mySerial.println("AT+CMGF=1"); // Set GSM Module to Text Mode
    Serial.println(readSerial());

    mySerial.println("AT+CMGS=\"+639859074220\""); // Mobile phone number to send message
    Serial.println(readSerial());

    mySerial.println(message);
    mySerial.write(26); // ASCII code for CTRL+Z to send the message
    Serial.println("Message Sent!");
    Serial.println(readSerial());
}

String readSerial() {
    delay(100);
    String response = "";
    while (mySerial.available()) {
        response += mySerial.readString();
    }
    return response;
}

void CallNumber() {
    mySerial.println("ATD+639603580096;"); // Mobile phone number to call
    Serial.println("Dialing...");
    Serial.println(readSerial());
    
    delay(20000); // Wait for 20 seconds (or adjust as needed)
    
    mySerial.println("ATH"); // Hang up the call
    delay(1000);
    Serial.println("Call Ended");
    Serial.println(readSerial());
}

void RecieveMessage() {
    Serial.println("Reading SMS...");
    mySerial.println("AT+CMGF=1"); // Set SMS mode to text
    Serial.println(readSerial());

    mySerial.println("AT+CNMI=1,2,0,0,0"); // Configure to receive SMS automatically
    Serial.println(readSerial());
}
