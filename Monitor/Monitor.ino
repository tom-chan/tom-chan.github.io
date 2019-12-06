/*
  Ping))) Sensor

  This sketch reads a PING))) ultrasonic rangefinder and returns the distance
  to the closest object in range. To do this, it sends a pulse to the sensor to
  initiate a reading, then listens for a pulse to return. The length of the
  returning pulse is proportional to the distance of the object from the sensor.

  The circuit:
  - +V connection of the PING))) attached to +5V
  - GND connection of the PING))) attached to ground
  - SIG connection of the PING))) attached to digital pin 7

  created 3 Nov 2008
  by David A. Mellis
  modified 30 Aug 2011
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Ping
*/

// include the library code:
#include <LiquidCrystal.h>
#include <Servo.h> 


// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 0 /*11*/, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int pingPin = 11;
const int pongPin = 10;

const int servoPin = 9;
Servo servo;  

int engaged = 1050;
int disengaged = 1150;

void setup() {
  // initialize serial communication: Serial.begin(9600);
  pinMode(pingPin, OUTPUT);
  pinMode(pongPin, INPUT);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

}

void loop() {
  // establish variables for duration of the ping, and the distance result
  // in inches and centimeters:
  long duration, inches, cm;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH pulse
  // whose duration is the time (in microseconds) from the sending of the ping
  // to the reception of its echo off of an object.
  duration = pulseIn(pongPin, HIGH);

  // convert the time into a distance
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);

  // clear screen for the next loop:
  lcd.clear();

  lcd.print(inches);
  lcd.print("in, ");
  lcd.print(cm);
  lcd.print("cm");

  delay(500);
}

long microsecondsToInches(long microseconds) {
  // According to Parallax's datasheet for the PING))), there are 73.746
  // microseconds per inch (i.e. sound travels at 1130 feet per second).
  // This gives the distance travelled by the ping, outbound and return,
  // so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return microseconds / 29 / 2;
}

void onOffButton() {
  servo.attach(servoPin); 
  servo.write(engaged);
  delay(100);
  servo.detach();
  delay(400);
  servo.attach(servoPin); 
  servo.write(disengaged);
  delay(100);
  servo.detach();
}
