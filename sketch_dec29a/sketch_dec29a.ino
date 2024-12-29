#include <DHT.h>

// Pin definitions
#define DHTPIN 2
#define DHTTYPE DHT11
#define TRIG_PIN 7
#define ECHO_PIN 8
#define BUZZER_PIN 9  
#define RESET_BTN 4    // Optional button for manual reset

// Constants
#define DISTANCE_THRESHOLD 4
#define SERIAL_BAUD 9600
#define MEASUREMENT_INTERVAL 100

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Global variables
unsigned long lastMeasurement = 0;
unsigned long lastBuzzerToggle = 0;
bool alarmActive = false;
bool buzzerState = false;
bool lastButtonState = HIGH;  // For button debouncing

void setup() {
  Serial.begin(SERIAL_BAUD);
  
  // Initialize pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RESET_BTN, INPUT_PULLUP);  // Enable internal pullup
  
  // Test buzzer at startup
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
  
  dht.begin();
  delay(2000);
}

float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  
  if (duration == 0) {
    return 400; // No echo detected
  }
  
  float distance = duration * 0.034 / 2; // Convert duration to cm
  return constrain(distance, 0, 400);    // Constrain to 0-400 cm
}

void handleBuzzer() {
  if (alarmActive) {
    unsigned long currentMillis = millis();
    // Toggle buzzer every second
    if (currentMillis - lastBuzzerToggle >= 1000) {
      buzzerState = !buzzerState;
      digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
      lastBuzzerToggle = currentMillis;
    }
  } else {
    digitalWrite(BUZZER_PIN, LOW);  // Ensure buzzer is off when alarm is inactive
    buzzerState = false;
  }
}

void checkResetButton() {
  // Read the current button state
  bool buttonState = digitalRead(RESET_BTN);
  
  // Check for button press with debouncing
  if (buttonState == LOW && lastButtonState == HIGH) {
    delay(50);  // Simple debounce delay
    if (digitalRead(RESET_BTN) == LOW) {  // Check again after delay
      resetAlarm(); // Call reset alarm function
    }
  }
  lastButtonState = buttonState;
}

void resetAlarm() {
  alarmActive = false;
  digitalWrite(BUZZER_PIN, LOW);
  buzzerState = false;
  Serial.println("Alarm Reset!"); // Notify the Python GUI
}

void checkSerialCommand() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // Read command from serial
    command.trim(); // Remove any extra whitespace or newline characters
    if (command == "RESET") {
      resetAlarm(); // Reset the alarm if "RESET" command received
    }
  }
}

void loop() {
  // Check reset button on every loop iteration
  checkResetButton();
  
  // Check serial input for reset command
  checkSerialCommand();
  
  // Take measurements every interval
  if (millis() - lastMeasurement >= MEASUREMENT_INTERVAL) {
    float distance = measureDistance();
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    
    // Check for intruder
    if (distance < DISTANCE_THRESHOLD && distance > 0) {
      alarmActive = true;
    }
    
    // Send data to Python GUI
    Serial.print(distance);
    Serial.print(",");
    Serial.print(temperature);
    Serial.print(",");
    Serial.print(humidity);
    Serial.print(",");
    Serial.println(alarmActive ? 1 : 0);
    
    lastMeasurement = millis();
  }
  
  // Handle buzzer state
  handleBuzzer();
}
