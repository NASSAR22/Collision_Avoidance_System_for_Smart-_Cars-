// Smart Car Collision Avoidance System
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NewPing.h>
#include <Servo.h>

// Ultrasonic sensor pins
#define TRIG_PIN 3 
#define ECHO_PIN 2
#define MAX_DISTANCE 200 

// Create objects
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE); 
Servo myservo;   
LiquidCrystal_I2C lcd(0x27, 16, 2); // Try 0x3F if 0x27 doesn't work

// Buzzer and LED pins
const int buzzerPin = 4;
const int redLedPin = 7;

// Motor control pins
const int motorPin1 = 11;  
const int motorPin2 = 10;  
const int motorPin3 = 6; 
const int motorPin4 = 5;  

// Variables
boolean goesForward = false;
int distance = 100;
int dangerThreshold = 10; // Distance in cm to trigger alarm
unsigned long lastBuzzerToggle = 0;
boolean buzzerState = false;

// Direction status
String currentDirection = "FORWARD";

void setup() {
  // Initialize pins
  pinMode(buzzerPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  
  // Initialize I2C LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Initialize servo
  myservo.attach(9);  
  myservo.write(90); // Center position
  delay(1000);
  
  // Initialize distance readings
  for (int i = 0; i < 4; i++) {
    distance = readPing();
    delay(100);
  }
  
  // Start with moving forward
  moveForward();
  updateDisplay();
}

void loop() {
  // Update distance reading
  distance = readPing();
  
  // Update display with current distance and direction
  updateDisplay();
  
  // Check if obstacle is too close
  if (distance <= dangerThreshold) {
    // Activate alarm - will continue until obstacle is removed
    activateAlarm(true);
    
    // Stop and navigate around obstacle
    moveStop();
    currentDirection = "STOPPED";
    updateDisplay();
    delay(100);
    
    moveBackward();
    currentDirection = "BACKWARD";
    updateDisplay();
    delay(300);
    
    moveStop();
    currentDirection = "SCANNING";
    updateDisplay();
    delay(200);
    
    // Look around for best route using new scanning pattern
    int distanceR = scanRight();
    int distanceL = scanLeft();
    
    // Choose the best direction
    if (distanceR >= distanceL) {
      turnRight();
      currentDirection = "RIGHT";
    } else {
      turnLeft();
      currentDirection = "LEFT";
    }
    updateDisplay();
    
    // Wait for the turn to complete
    delay(500);
    
    // Move forward in the new direction
    moveForward();
    currentDirection = "FORWARD";
    updateDisplay();
  } else {
    // Deactivate alarm if obstacle is far enough
    activateAlarm(false);
    
    // Continue moving forward
    moveForward();
    currentDirection = "FORWARD";
    updateDisplay();
  }
  
  delay(40);
}

// Function to update LCD display with distance and direction
void updateDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dist: ");
  lcd.print(distance);
  lcd.print(" cm");
  lcd.setCursor(0, 1);
  lcd.print("Dir: ");
  lcd.print(currentDirection);
}

// Function to activate/deactivate alarm - modified to avoid using tone()
void activateAlarm(boolean active) {
  digitalWrite(redLedPin, active ? HIGH : LOW);
  
  // Manual buzzer control instead of tone()
  if (active) {
    // Create a square wave for buzzer (approx 2000Hz)
    unsigned long currentMillis = millis();
    if (currentMillis - lastBuzzerToggle >= 1) { // Toggle every 0.5ms for ~1000Hz
      lastBuzzerToggle = currentMillis;
      buzzerState = !buzzerState;
      digitalWrite(buzzerPin, buzzerState);
    }
  } else {
    digitalWrite(buzzerPin, LOW);
  }
}

// New scanning functions with requested pattern
int scanRight() {
  int maxDistance = 0;
  
  // Gradually move from center to right (90° to 0°)
  for (int angle = 90; angle >= 0; angle -= 10) {
    myservo.write(angle);
    delay(50);
    int currentDistance = readPing();
    if (currentDistance > maxDistance) {
      maxDistance = currentDistance;
    }
    
    // Update display during scanning
    currentDirection = "SCAN R " + String(angle);
    updateDisplay();
  }
  
  // Return to center
  myservo.write(90);
  delay(100);
  return maxDistance;
}

int scanLeft() {
  int maxDistance = 0;
  
  // Gradually move from center to left (90° to 180°)
  for (int angle = 90; angle <= 180; angle += 10) {
    myservo.write(angle);
    delay(50);
    int currentDistance = readPing();
    if (currentDistance > maxDistance) {
      maxDistance = currentDistance;
    }
    
    // Update display during scanning
    currentDirection = "SCAN L " + String(angle);
    updateDisplay();
  }
  
  // Return to center
  myservo.write(90);
  delay(100);
  return maxDistance;
}

int readPing() { 
  delay(50);
  int cm = sonar.ping_cm();
  if (cm == 0) {
    cm = 250; // Set to max if no echo received
  }
  return cm;
}

void moveStop() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, LOW);
} 
  
void moveForward() {
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin4, LOW);  
}

void moveBackward() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, HIGH);   
}  

void turnRight() {
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, HIGH);    
  delay(300);
}
 
void turnLeft() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);   
  digitalWrite(motorPin3, HIGH);
  digitalWrite(motorPin4, LOW);     
  delay(300);
}
