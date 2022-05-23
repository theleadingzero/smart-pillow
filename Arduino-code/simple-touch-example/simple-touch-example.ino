/*
   simple-touch-example
*/

#include <Wire.h>
#include <HID-Project.h>
#include <Trill.h>


Trill trillSensor;
int sensorValues[30];

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

  // Fill the array holding the sensor values with 0s
  for (int i = 0; i < 30; i++) {
    sensorValues[i] = 0;
  }

  // Initialise the HID keyboard
  // Sends a clean report to the host. This is important on any Arduino type.
  Keyboard.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  getSensorData();
  for(int i=0; i<30; i++){
    Serial.print(sensorValues[i]);
    Serial.print(' ');
  }
  Serial.println("");
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
}
