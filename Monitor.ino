// include the library code:
#include <LiquidCrystal.h>
#include <Servo.h> 
#include "IRremote.h"

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 0 /*11*/, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
// 12 pins needed (left to right)
// VSS => GND
// VDD => 5V
// V0 => pot
// RS => pin 12
// RW => GND
// E => pin 0
// D4 => pin 5
// D5 => pin 4
// D6 => pin 3
// D7 => pin 2
// A => 5V backlight (need 220ohm?)
// K => GND 

const int pingPin = 11;
const int pongPin = 10;

const int servoPin = 9;
Servo servo;  

int engaged = 950;
int disengaged = 1150;

const int buttonPin = 8;
const int ledPin = 1;

// state of the monitor: 0=off, >0=countdown
int countdown = 0;

// backlight status: 0=off; 1-10=countdown; 100=always on
const int alwaysOn = 100;
const int backlightCountdown = 10;
int backlight = backlightCountdown;

const int distanceThreshold = 32; // inches
const int countdownReset = 300;
const int countdownIncrement = 300;

const int RECV_PIN = 7;
IRrecv irrecv(RECV_PIN);
decode_results results;

byte custChars[8][8] = {
  { // 0
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
  },
  { // 1
    B11111,
    B11111,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
  },
  { // 2
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11111,
    B11111,
  },
  { // 3
    B11111,
    B11111,
    B11000,
    B11000,
    B11000,
    B11000,
    B11111,
    B11111,
  },
  { // 4
    B11111,
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
  },
  { // 5
    B11111,
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
  },
  { // 6
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
  },
  { // 7
    B11000,
    B11000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11000,
    B11000,
  },
};

// TL, TR, BL, BR
const byte B = 32; // " "
const byte digits[10][4] = {
  {1, 0, 2, 0}, // 0
  {B, 0, B, 0}, // 1
  {4, 0, 3, 7}, // 2
  {5, 0, 6, 0}, // 3
  {2, 0, B, 0}, // 4
  {3, 7, 6, 0}, // 5
  {3, 7, 2, 0}, // 6
  {4, 0, B, 0}, // 7
  {3, 0, 2, 0}, // 8
  {3, 0, 6, 0}  // 9
};

void setup() {
  // initialize serial communication: Serial.begin(9600);
  pinMode(pingPin, OUTPUT);
  pinMode(pongPin, INPUT);
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  for (int i = 0; i < 8; i++) {
    lcd.createChar(i, custChars[i]);
  }
  irrecv.enableIRIn();
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
  // cm = microsecondsToCentimeters(duration);

  lcd.setCursor(12, 1);
  lcd.print("))");
  if (inches > 99) {
    lcd.print("99");
  } else if (inches <= 9) {
    lcd.print(" ");
    lcd.print(inches);
  } else {
    lcd.print(inches);
  }

  if (irrecv.decode(&results)){
    switch (results.value) {
      /* Increase countdown */
      case 0x9CB47: // Sony Up
      case 0x1EE17887: // Yamaha Up
      case 0xC12F41BE: // Projector Up
      case 0xC20370A1: // Panasonic Up
      case 0x91CA2EE3: // From ir blaster projector up
        backlightOn();
        if (countdown <= 0) {
          // monitor is off; turn on the monitor
          onOffButton();
        }
        countdown += countdownIncrement;
        break;
      /* Decrease countdown */
      case 0x5CB47: // Sony Down
      case 0x1EE1F807: // Yamaha Down
      case 0xC12FA15E: // Projector Down
      case 0x81930A09: // Panasonic Down
      case 0xF24340FD: // From ir blaster projector down
        backlightOn();
        if (countdown > 0) { // decrease only if >0 otherwise no-op
          countdown -= countdownIncrement;
          if (countdown <= 0) {
            // time is up; turn off the monitor
            countdown = 0;
            onOffButton();
          }
        }
        break;
      /* Toggle backlight */
      case 0xBCB47: // Sony Enter
      case 0x1EE15DA2: // Yamaha Dimmer
      case 0xC12F619E: // Projector Enter
      case 0xBB0ED9E1: // Panasonic OK
      case 0xC4400F1F: // From ir blaster projector ok
        if (backlight == alwaysOn) {
          digitalWrite(ledPin, LOW); // turn backlight off
          backlight = 0;
        } else {
          digitalWrite(ledPin, HIGH); // turn backlight on
          backlight = alwaysOn;
        }
        break;
      default: // take the lower 6 hex and print as YYMMDD
        byte valid = 1;
        for (int i = 5; i >= 0; i--) {
          byte hexDigit = (results.value >> (i*4)) & 0xF;
          if (hexDigit > 0x9) { // not a digit, pass
            valid = 0;
          }
        }
        if (valid) printDate(results.value);
        break;
    }

    irrecv.resume();
  }


  if (inches < distanceThreshold) {
    backlightOn();
    if (countdown <= countdownReset) { // prop it back up to countdownReset value
      showCountdown(countdownReset);
      if (countdown <= 0) {
        // monitor is off; turn on the monitor
        onOffButton();
      }
      countdown = countdownReset;
    } else { // continue to decrement
      countdown = countdown - 1;
      showCountdown(countdown);
    }
  } else {
    countdown = countdown - 1;
    if (countdown == 0) {
      // time is up; turn off the monitor
      showCountdown(0);
      countdown = 0;
      onOffButton();      
    } else if (countdown < 0) {
      countdown = 0;
      showCountdown(countdown);
    } else {
      showCountdown(countdown);
    }
  }
  delay(950);
  backlightDec();
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
  delay(300);
  servo.detach();
  delay(400);
  servo.attach(servoPin); 
  servo.write(disengaged);
  delay(100);
  servo.detach();
}

void showCountdown(int count) {
  lcd.setCursor(12, 0);
  if (count < 1000) lcd.print(" ");
  if (count < 100) lcd.print(" ");
  if (count < 10) lcd.print(" ");
  lcd.print(count);
}

void backlightOn() {
  if (backlight != alwaysOn) {
    if (backlight == 0) {
      digitalWrite(ledPin, HIGH); // turn backlight on
    }
    backlight = backlightCountdown; // reset timer
  }
}

void backlightDec() {
  if (backlight != alwaysOn) {
    if (backlight != 0) {
      backlight = backlight - 1;
      if (backlight == 0) {
        digitalWrite(ledPin, LOW); // turn backlight off
      }
    }
  }
}

void printDate(long date) {
  backlightOn();
  lcd.setCursor(0, 0); // use write not print below
  for (int i = 5; i >= 0; i--) {
    byte hexDigit = (date >> (i*4)) & 0xF;
    lcd.write(digits[hexDigit][0]);
    lcd.write(digits[hexDigit][1]);
  }
  lcd.setCursor(0, 1); // next line
  for (int i = 5; i >= 0; i--) {
    byte hexDigit = (date >> (i*4)) & 0xF;
    lcd.write(digits[hexDigit][2]);
    lcd.write(digits[hexDigit][3]);
  }
}
