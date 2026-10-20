#define BLYNK_TEMPLATE_ID "TMPL2_L8A3tqJ"
#define BLYNK_TEMPLATE_NAME "MowerControl"
#define BLYNK_AUTH_TOKEN "1IBStcByuSvGCUjoDZJgzA0SV71ncgCV"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// 2. Enter your Wi-Fi credentials
char ssid[] = "HenrysiPhone";
char pass[] = "Ramsey12";

// Define the GPIO pins connected to your motor controller's PWM inputs
const int motor1PwmPin = 26, 
  motor2PwmPin = 27,
  motor1DirPin = 14,
  motor2DirPin = 13,
  motor1Channel = 0,
  motor2Channel = 1; 

// PWM Configuration
const int pwmFrequency = 5000,
 pwmResolution = 8;    // 8-bit resolution (values from 0 to 255)

int joyY = 0,
  joyX = 0;

int lastPrintTime;

void setup() {
  Serial.begin(115200);

  // Configure the pins for PWM (Modern ESP32 API)
  // Syntax: ledcAttach(pin, frequency, resolution)
  ledcSetup(motor1Channel, pwmFrequency, pwmResolution);
  ledcSetup(motor2Channel, pwmFrequency, pwmResolution);

  ledcAttachPin(motor1PwmPin, motor1Channel);
  ledcAttachPin(motor2PwmPin, motor2Channel);

  pinMode(motor1DirPin, OUTPUT);
  pinMode(motor2DirPin, OUTPUT);

  digitalWrite(motor1DirPin, LOW); // Reverses direction for one motor since DIR is active low
  digitalWrite(motor2DirPin, HIGH);

  Serial.println("Trying to log on to Blynk...");
  WiFi.disconnect(true);
  Serial.printf("Free Heap before Wi-Fi: %d bytes\n", ESP.getFreeHeap());
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  Serial.printf("Free Heap after Wi-Fi: %d bytes\n", ESP.getFreeHeap());
  Serial.println("Successfully logged on Blynk!");
  Blynk.virtualWrite(V0, joyY);
  Blynk.virtualWrite(V1, joyX);
}


BLYNK_WRITE(V0) { // This is run whenever a Y input is given on joystick
  joyY = param.asInt(); // Get joystick value
}
BLYNK_WRITE(V1) { // This is run whenever a X input is given on joystick
  joyX = param.asInt(); // Get joystick value
}
BLYNK_WRITE(V2) {
  int buttonState = param.asInt();

  // If the button is pressed
  if (buttonState == 1) {
    // 1. Reset global variables to zero
    joyY = 0;
    joyX = 0;

    // 2. Force the Blynk phone UI sliders/joystick controls back to 0
    Blynk.virtualWrite(V0, 0);
    Blynk.virtualWrite(V1, 0);
  }
}

void loop() {
  Blynk.run(); // Run through inputs on the app
  int motor1 = joyY + joyX;
  int motor2 = joyY - joyX;
  if (motor1 < 0) {
    motor1 = -motor1; // Reverse motor input to make it positive
    if (digitalRead(motor1DirPin) == LOW) digitalWrite(motor1DirPin, HIGH); // Reverse motor direction
  }
  else digitalWrite(motor1DirPin, LOW);

  if (motor2 < 0) {
    motor2 = -motor2; // Reverse motor input to make it positive
    if (digitalRead(motor2DirPin) == HIGH) digitalWrite(motor2DirPin, LOW); // Reverse motor direction
  }
  else digitalWrite(motor2DirPin, HIGH);
  
  ledcWrite(motor1Channel, motor1);
  ledcWrite(motor2Channel, motor2);

  if (millis() - lastPrintTime > 500) {
    Serial.printf("Y: %d | X: %d | Motor1 Speed: %d | Motor2 Speed: %d\n", joyY, joyX, motor1, motor2);
    lastPrintTime = millis();
  }
}