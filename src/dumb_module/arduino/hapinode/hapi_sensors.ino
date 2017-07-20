#include <Arduino.h>

#include "misc.h"

/*
#*********************************************************************
#Copyright 2016 Maya Culpa, LLC
#
#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.
#*********************************************************************
*/

void setupSensors(void) {
// Initialize Digital Pins for Input or Output - From the arrays pinControl and pinDefaults
  for (int i = 0; i < ARRAY_LENGTH(pinControl); i++) {
    switch (pinControl[i]) {
    case DIGITAL_INPUT_PIN:
      pinMode(i, INPUT);
      break;
    case DIGITAL_INPUT_PULLUP_PIN:
      pinMode(i, INPUT_PULLUP);
      break;
    case DIGITAL_OUTPUT_PIN:
      pinMode(i, OUTPUT);
      digitalWrite(i, pinDefaults[i]);
      break;
    case ANALOG_OUTPUT_PIN:
      pinMode(i, OUTPUT);
      break;
    default:
      break;
    }
  }

// Start the DHT-22
  dht1.begin();
  /*for (int i = 0; i < NUM_DHTS; i++) {
    dhts[i].begin();
  }*/

// Start the DS18B20
  wp_sensors.begin();

// Start the flow sensor
  pinMode(sWtrFlow_PIN, INPUT);
  flowrate.attach(sWtrFlow_PIN);
  flowrate.interval(5);

// Start the I2C
  Wire.begin(SDA_PIN,SCL_PIN);      // Default
  Wire.setClock(400000);  // choose 400 kHz I2C rate
  Alarm.delay(100);
}

String getPinArray() {
  // Returns all pin configuration information
  String response = "";
  for (int i = 0; i < ARRAY_LENGTH(pinControl); i++) {
    if (i < NUM_DIGITAL) {
      response += String(i) + String(pinControl[i]);
    }
    else {
      response += "A" + String(i - NUM_DIGITAL) + String(pinControl[i]);
    }
  }
  return response;
}

float readHumidity(int iDevice) {
  // readHumidity
  // Uses the DHT Library to read the current humidity
  float humidity;

  //humidity = dhts[iDevice].readHumidity();
  humidity = dht1.readHumidity();
  if (isnan(humidity)) {
    humidity = -1;
  }
  // Serial.print(F("DHT Humidity: "));
  // Serial.println(humidity);
  return humidity;
}

float readTemperatured(int iDevice) {
  // readTemperature
  // Uses the DHT Library to read the current temperature
  float temperature;

  //temperature = dhts[iDevice].readTemperature();
  temperature = dht1.readTemperature();
  if (isnan(temperature)) {
    temperature = -1;
  }
  else if (!metric) {
    temperature = FAHRENHEIT(temperature);
  }
  // Serial.print(F("DHT Temperature: "));
  // Serial.println(temperature);
  return temperature;
}

float read1WireTemperature(int iDevice) {
  // readWaterTemperature
  // Uses the Dallas Temperature library to read the waterproof temp sensor
  float temperature;

  wp_sensors.requestTemperatures();
  temperature = wp_sensors.getTempCByIndex(0);
  if (isnan(temperature)) {
    temperature = -1;
  }
  else if (!metric) {
    temperature = FAHRENHEIT(temperature);
  }
  // Serial.print(F("18B20 Temperature: "));
  // Serial.println(temperature);
  return temperature;
}

void swap(unsigned char *x, unsigned char *y, size_t n)
{
  unsigned char t;
  size_t i;

  for (i = 0; i < n; i++) {
    t = x[i];
    x[i] = y[i];
    y[i] = t;
  }
}

void sort(
  void *values,
  size_t n,
  size_t size,
  int (*compare)(const void *, const void *))
{
  for (int i = 0; i < n - 1; i++) {
    for (int j = i + 1; j < n; j++) {
      if ((*compare)(values+i*size, values+j*size) > 0) {
        swap(values+i*size, values+j*size, size);
      }
    }
  }
}

int compare_int(int *x, int *y)
{
    if (*x == *y)
        return 0;
    else if (*x > *y)
        return 1;
    else /* *x < *y */
        return -1;
}

#define N_SAMPLES (10)
#define N_OUTLIERS (2)

#define VCC (5.0) // Unit is 1 Volt. ^^^ Should be in some header. ^^^Should this be different for some boards?
#define MAX_ADC_VALUE ((1<<10)-1) // ^^^ Should be in some header.

float read_voltage(int device) {
  int buf[N_SAMPLES];
  int pin;
  unsigned long int sum;
  float averaged_voltage;

  // Get samples.
  pin = HapicData[device].hcs_sensepin;
  for (int i = 0; i < ARRAY_LENGTH(buf); i++) {
    buf[i] = analogRead(pin);
    delay(10/*^^^evil magic number*/);
  }

  sort(
    buf,
    ARRAY_LENGTH(buf),
    sizeof(*buf),
    (int (*)(const void *, const void *))compare_int);

  // Average the samples, ignoring N_OUTLIERS high and N_OUTLIERS low samples.
  // Also convert to Volts.
  sum = 0;
  for (int i = N_OUTLIERS; i < ARRAY_LENGTH(buf) - N_OUTLIERS; i++)
    sum += buf[i];
  averaged_voltage = sum;
  averaged_voltage *= VCC /
    (MAX_ADC_VALUE * (ARRAY_LENGTH(buf) - 2*N_OUTLIERS));

  return averaged_voltage;
}

float readpH(int device) {
  float voltage;
  float ph_value;

  voltage = read_voltage(device);
  ph_value = 3.5 * voltage;
  //  Serial.print(F("pH: "));
  //  Serial.println(ph_value);
  return ph_value;
}

float readTDS(int device) {
  float voltage;
  float tds;

  voltage = read_voltage(device);

  //TODO Need temperature compensation for TDS
  tds = 1.0 * voltage;
  //  Serial.print(F("TDS: "));
  //  Serial.println(tds);
  return tds;
}

//  Type                Ambient light (lux)  Photocell resistance (Ω)
//
//  Dim hallway         0.1 lux               600KΩ
//  Moonlit night       1 lux                 70 KΩ
//  Dark room           10 lux                10 KΩ
//  Dark overcast day   Bright room 100 lux   1.5 KΩ
//  Overcast day        1000 lux              300 Ω

float readLightSensor(int device) {
  // Simple code to read a Light value from a CDS sensor, with 10k to ground
  float Lux;
  ControlData d;
  d = HapicData[device];

  int RawADC = analogRead(d.hcs_sensepin);
//TODO
  Lux = (float)RawADC; // Need to do some processing to get lux from CDS reading
  return Lux;
}

float readFlow(int device) {
  // readWaterFlowRate  - Uses an input pulse that creates an average flow rate
  //                      The averaging is done in software and stores a 30second rolling count
  ControlData d;
  d = HapicData[device];
//TODO
  return (float)WaterFlowRate;
}

float readSensorPin(int device) {
  float pinData;
//TODO
  return pinData;
}

void hapiSensors(void) {
  for (int device = 0; device < ARRAY_LENGTH(HapisFunctions); device++) {
    currentTime = now();                  // Set the time
    sendMQTTAsset(SENSORID_FN, device);   // Read the sensor value
  }
}

