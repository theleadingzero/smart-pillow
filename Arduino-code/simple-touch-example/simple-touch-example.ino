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
  }

  // Initialise the HID keyboard
  // Sends a clean report to the host. This is important on any Arduino type.
  Keyboard.begin();

}

void loop() {
  // Request the latest touch data from Trill
  getSensorData();


  // Print the latest touch data from Trill
  printSensorData();

  for (int i = 0; i < 30; i++ ) {
    if (isTouched(i)) {
      Serial.print("touched ---- ");
      Serial.println(i);
    }
    if (isTapped(i)) {
      Serial.print("tapped ----------- ");
      Serial.println(i);
    }
  }

  if ( isHeldTimer(24, 1500))
    Serial.println("24 is held to time");
}


bool isTouched(int pin) {
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
  return false
}

bool isHeldTimer(int pin, int minTime) {
  // check if the pin has been actively touched for at least the minimum time in ms
  unsigned long elapsedTime = millis() - touchTimers[pin];
  if ( sensorTouches[pin] && elapsedTime > minTime )
    return true;
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
