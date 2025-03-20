#include <Servo.h>

const int buttonPin = 12; // button led here
const int greenLedPin = 3;  // PWM-capable pin for green led
const int whiteStartPin = 4; // white led here
const int redLedPin = 11; // red led here
const int servoPin = 13;  // servo connected here

const int blinkDelay = 1000; // time it takes to advance to the next light

const float frequency = 0.0025;  // adjust to change speed of sine wave
const int amplitude = 127;  // half of 255 (max PWM value)
const int offset = 128;  // center it within the 0-255 range

bool firstPress = false;  // tracks if button has been pressed once
bool isRunning = false;  // tracks if animation is running
bool buttonState = false;
bool lastButtonState = false;

Servo myServo;

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(greenLedPin, OUTPUT);
  myServo.attach(servoPin);  // attach servo to pin 13
  myServo.write(85);  // starting position

  for (int i = whiteStartPin; i <= redLedPin; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, HIGH);
  }
}

void loop() {
  buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH && lastButtonState == LOW) {  // detect button press

      isRunning = true; 

      for (int i = whiteStartPin; i <= redLedPin; i++) {
        digitalWrite(greenLedPin, LOW);
        digitalWrite(i, LOW);
        delay(blinkDelay / 2);
        digitalWrite(greenLedPin, HIGH);
        delay(blinkDelay / 2);
      }
      
      // when red LED turns off, move servo
      myServo.write(40); 

      digitalWrite(greenLedPin, HIGH);
      for (int i = whiteStartPin; i <= redLedPin; i++) {
        digitalWrite(i, HIGH);
      }

      delay(2000);
      isRunning = false;  // reset flag after animation

  }
  lastButtonState = buttonState;  // update button state

  // sine wave on green LED while in standby
  if (!isRunning) {
    unsigned long currentTime = millis();
    float angle = currentTime * frequency;  // convert time to radians
    int brightness = offset + amplitude * sin(angle);  // scale sin() output to 0-255
    analogWrite(greenLedPin, brightness);
    delay(20);
  }
}