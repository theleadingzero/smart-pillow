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
unsigned long timeLastTap[30]; // time when the last tap occurred

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
    timeLastTap[i] = 0;
  }

  // Initialise the HID keyboard
  // Sends a clean report to the host. This is important on any Arduino type.
  Keyboard.begin();

}

void loop() {
  // Request the latest touch data from Trill
  getSensorData();

  //-----------------------------------------------------------------------------
  // vvvvvvv Only change code between here vvvvvvv
  //-----------------------------------------------------------------------------
  // Print all the latest touch data from Trill
  printSensorData();

  // Go through all of the sensor pins
  for ( int i = 0; i < 30; i++) {
    // send the key press of the number that was tapped
    if ( isBeingTouched(i) ) {
      // Print messages to the Serial monitor to help debug
      Serial.print(i);
      Serial.println(" is being touched");
      // Send a key press of the letter 'H'
      Keyboard.press('H');
      Keyboard.releaseAll();
    }
  }

  //-----------------------------------------------------------------------------
  // ^^^^^^^ Only change code above here ^^^^^^^
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
  if ( !sensorTouches[pin] && prevSensorTouches[pin] && elapsedTime < 500) {
    timeLastTap[pin] = millis();
    return true;
  }
  // otherwise return false
  return false;
}

bool isDoubleTapped(int pin) {
  // the sensor was tapped twice in quick succession
  // if one tap has happened and less than 50ms has passed since last tap
  unsigned long elapsedTime = millis() - timeLastTap[pin];
  if ( isTapped(pin) && elapsedTime < 400 ) {
    return true;
  }
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
