/*
   simple-touch-example
*/

#include <Wire.h>
#include <HID-Project.h>
#include <Trill.h>


Trill trillSensor;

// arrays holding information about each touch sensing channel
int sensorValues[30]; // raw values from the touch sensors
int touchThresholds[30]; // threshold for what counts as a touch
bool sensorTouches[30]; // true if currently above touch threshold
bool prevSensorTouches[30]; // previous frame of touches
unsigned long touchTimers[30]; // when a pin is first touched
bool heldToTime[30]; // if pin has been held to timer

void setup() {
  // Initialise serial and touch sensor
  Serial.begin(115200);
  Serial.println("Initialising Trill");
  delay(2000);

  // Initialse Trill and save returned value
  int ret = trillSensor.setup(Trill::TRILL_CRAFT);
  delay(1000);

  // If returned value isn't 0, something went wrong
  if (ret != 0) {
    Serial.println("failed to initialise trillSensor");
    Serial.print("Error code: ");
    Serial.println(ret);
  }

  // Make sure Trill Craft is set to differential mode
  trillSensor.setMode(Trill::DIFF);

  // Fill the arrays with starting data
  for (int i = 0; i < 30; i++) {
    sensorValues[i] = 0;
    touchThresholds[i] = 400;
    sensorTouches[i] = false;
    prevSensorTouches[i] = false;
    touchTimers[i] = 0;
    heldToTime[i] = false;
  }

  // Initialise the HID keyboard
  // Sends a clean report to the host. This is important on any Arduino type.
  Keyboard.begin();

}

void loop() {
  // Request the latest touch data from Trill
  getSensorData();

  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------
  // Uncomment to print all the latest touch data from Trill
  // printSensorData();

  // Example of pressing the spacebar when pin 0 is tapped
  if ( isTapped(0) ) {
    Serial.println("0 was tapped");
    Keyboard.press(KEY_SPACE);
    Keyboard.releaseAll();
  }

  // Example of pressing when pin 24 is tapped while pin 26 is held
  if ( isBeingTouched(24) && isTapped( 26) ) {
    Serial.println("24 is being held and 26 was tapped");
    Keyboard.press('m');
    Keyboard.releaseAll();

  }

  // Example of pressing once pin 29 has been held for 1.5 seconds
  if ( isHeldTimer(29, 1500)) {
    Serial.println("29 was held to time");
    Keyboard.press('f');
    Keyboard.releaseAll();
  }
  //-----------------------------------------------------------------------------
  //-----------------------------------------------------------------------------
}


bool isBeingTouched(int pin) {
  return sensorTouches[pin];
}

bool isTapped(int pin) {
  // the sensor was touched but has just been released
  // (no longer being touched)
  // and it was touched for less than a half second (500 ms)
  unsigned long elapsedTime = millis() - touchTimers[pin];
  if ( !sensorTouches[pin] && prevSensorTouches[pin] && elapsedTime < 500)
    return true;
  // otherwise return false
  return false;
}

bool isDoubleTapped(int pin) {
  // the sensor was tapped twice in quick succession
  // if one tap has happened and less than 50ms has passed
  // and a second tap occurs
  //otherwise return false
  return false;
}

bool isHeldTimer(int pin, int minTime) {
  // check if previously was held to time and hasn't been released
  if ( heldToTime[pin] && sensorTouches[pin] ) {
    return false;
  }
  // check if the pin has been actively touched for at least the minimum time in ms
  unsigned long elapsedTime = millis() - touchTimers[pin];
  if ( sensorTouches[pin] && elapsedTime > minTime ) {
    heldToTime[pin] = true;
    return true;
  }
  // if sensor has been released, reset the held to time flag
  if ( !sensorTouches[pin] )
    heldToTime[pin] = false;
  // otherwise
  return false;
}


void getSensorData() {
  // Request the latest sensor values from the chip
  trillSensor.requestRawData();

  // start with the first channel, number 0
  int sensorChannel = 0;

  // if there is data available
  if (trillSensor.rawDataAvailable() > 0) {
    // read in the data
    while (trillSensor.rawDataAvailable() > 0) {
      sensorValues[sensorChannel] = trillSensor.rawDataRead();
      // increase by one to read next channel
      sensorChannel++;
    }
  }

  // save the touches from that last data read as previous touches
  for (int pin = 0; pin < 30; pin++) {
    prevSensorTouches[pin] = sensorTouches[pin];
  }

  // update whether a pin has gone above the touch threshold
  for (int pin = 0; pin < 30; pin++) {
    if ( sensorValues[pin] > touchThresholds[pin] ) {
      sensorTouches[pin] = true;
    } else {
      sensorTouches[pin] = false;
    }
  }


  // save the current time for the timers
  unsigned long currTime = millis();

  // if the touch is new, save the current time
  for (int pin = 0; pin < 30; pin++) {
    if ( sensorTouches[pin] && !prevSensorTouches[pin]) {
      touchTimers[pin] = currTime;
    }
  }
}


void printSensorData() {
  // Print on a single line the current sensor values
  for (int i = 0; i < 30; i++) {
    Serial.print(sensorValues[i]);
    Serial.print(' ');
  }
  Serial.println("");

}
