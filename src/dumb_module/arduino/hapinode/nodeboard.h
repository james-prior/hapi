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

#ifndef HAPIBOARD_H
#define HAPIBOARD_H

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof(*(x)))

enum pin_mode_enum {
  UNUSED_PIN, // or reserved
  DIGITAL_INPUT_PIN,
  DIGITAL_INPUT_PULLUP_PIN,
  DIGITAL_OUTPUT_PIN,
  ANALOG_OUTPUT_PIN,
  ANALOG_INPUT_PIN
};

struct pin_config_struct {
  pin_mode_enum mode;
  int default_value;
};

#ifdef HN_ENET             // Mega256
#define NUM_DIGITAL 54    // Number of digital I/O pins
#define PIN_MAP_SIZE NUM_DIGITAL*2   // Array size for default digital state data
                                     // 2 bytes per digital I/O pin, 1st byte = State, 2nd byte = Value
// Default pin allocation
#define DHT_PIN 12        // Reserved pin for DHT-22 sensor
#define WIRE_PIN  13      // Reserved pin for 1-Wire bus
#define LED_PIN 2         // Use the built-in led for visual feedback
#define TDS_DRIVE1 33     // Reserved pin for TDS probe
#define TDS_INPUT1 25     // Reserved pin for TDS probe
#define TDS_INPUT2 26     // Reserved pin for TDS probe
#define TDS_DRIVE2 27     // Reserved pin for TDS probe

#define cWatr_PIN 4       // Water pump control pin
#define sFlow_PIN 36      // Water flow sensor pin
#define cFill_PIN 2       // Fill pump control pin
#define sFloat_PIN 39      // Fill float sensor pin
#define cNutr_PIN 15      // Nutrient pump control pin
#define sTDS_PIN 25       // TDS sensor pin
#define cpHUp_PIN 17      // pHUp pump control pin
#define spH_PIN   35      // pH sensor pin
#define cpHDn_PIN 16      // pHDown pump control pin
#define cLamp_PIN 14      // Lamp control pin
#define sLux_PIN  4       // Light sensor pin

// Analog input pins are assumed to be used as analog input pins
const struct pin_config_struct pin_configurations[] = {
  // digital
  {UNUSED_PIN,               LOW }, //  0
  {UNUSED_PIN,               LOW }, //  1
  {DIGITAL_OUTPUT_PIN,       HIGH}, //  2
  {DIGITAL_OUTPUT_PIN,       HIGH}, //  3
  {UNUSED_PIN,               LOW }, //  4
  {DIGITAL_OUTPUT_PIN,       HIGH}, //  5
  {DIGITAL_OUTPUT_PIN,       HIGH}, //  6
  {DIGITAL_OUTPUT_PIN,       HIGH}, //  7
  {DIGITAL_OUTPUT_PIN,       HIGH}, //  8
  {DIGITAL_OUTPUT_PIN,       HIGH}, //  9
  {UNUSED_PIN,               LOW }, // 10
  {DIGITAL_INPUT_PULLUP_PIN, LOW }, // 11
  {DIGITAL_INPUT_PIN,        LOW }, // 12
  {DIGITAL_OUTPUT_PIN,       LOW }, // 13
  {UNUSED_PIN,               LOW }, // 14
  {UNUSED_PIN,               LOW }, // 15
  {UNUSED_PIN,               LOW }, // 16
  {UNUSED_PIN,               LOW }, // 17
  {UNUSED_PIN,               LOW }, // 18
  {UNUSED_PIN,               LOW }, // 19
  {UNUSED_PIN,               LOW }, // 20
  {UNUSED_PIN,               LOW }, // 21
  {DIGITAL_OUTPUT_PIN,       LOW }, // 22
  {DIGITAL_OUTPUT_PIN,       LOW }, // 23
  {DIGITAL_OUTPUT_PIN,       LOW }, // 24
  {DIGITAL_OUTPUT_PIN,       LOW }, // 25
  {DIGITAL_OUTPUT_PIN,       LOW }, // 26
  {DIGITAL_OUTPUT_PIN,       LOW }, // 27
  {DIGITAL_INPUT_PIN,        LOW }, // 28
  {DIGITAL_INPUT_PIN,        LOW }, // 29
  {DIGITAL_INPUT_PIN,        LOW }, // 30
  {DIGITAL_INPUT_PIN,        LOW }, // 31
  {DIGITAL_INPUT_PIN,        LOW }, // 32
  {DIGITAL_INPUT_PIN,        LOW }, // 33
  {DIGITAL_INPUT_PIN,        LOW }, // 34
  {DIGITAL_INPUT_PIN,        LOW }, // 35
  {DIGITAL_INPUT_PIN,        LOW }, // 36
  {DIGITAL_INPUT_PIN,        LOW }, // 37
  {DIGITAL_INPUT_PIN,        LOW }, // 38
  {DIGITAL_INPUT_PIN,        LOW }, // 39
  {DIGITAL_INPUT_PIN,        LOW }, // 40
  {DIGITAL_INPUT_PIN,        LOW }, // 41
  {DIGITAL_INPUT_PIN,        LOW }, // 42
  {DIGITAL_INPUT_PIN,        LOW }, // 43
  {DIGITAL_INPUT_PIN,        LOW }, // 44
  {DIGITAL_INPUT_PIN,        LOW }, // 45
  {DIGITAL_INPUT_PIN,        LOW }, // 46
  {DIGITAL_INPUT_PIN,        LOW }, // 47
  {DIGITAL_INPUT_PULLUP_PIN, LOW }, // 48
  {DIGITAL_INPUT_PULLUP_PIN, LOW }, // 49
  {UNUSED_PIN,               LOW }, // 50
  {UNUSED_PIN,               LOW }, // 51
  {UNUSED_PIN,               LOW }, // 52
  {UNUSED_PIN,               LOW }, // 53
  // analog
  {ANALOG_INPUT_PIN,         LOW }, // 54
  {ANALOG_INPUT_PIN,         LOW }, // 55
  {ANALOG_INPUT_PIN,         LOW }, // 56
  {ANALOG_INPUT_PIN,         LOW }, // 57
  {ANALOG_INPUT_PIN,         LOW }, // 58
  {ANALOG_INPUT_PIN,         LOW }, // 59
  {ANALOG_INPUT_PIN,         LOW }, // 60
  {ANALOG_INPUT_PIN,         LOW }, // 61
  {ANALOG_INPUT_PIN,         LOW }, // 62
  {ANALOG_INPUT_PIN,         LOW }, // 63
  {ANALOG_INPUT_PIN,         LOW }, // 64
  {ANALOG_INPUT_PIN,         LOW }, // 65
  {UNUSED_PIN,               LOW }, // 66
  {UNUSED_PIN,               LOW }, // 67
  {UNUSED_PIN,               LOW }, // 68
  {UNUSED_PIN,               LOW }  // 69
};
#endif

#ifdef HN_ESP8266
#define NUM_DIGITAL 17    // Number of digital I/O pins
#define PIN_MAP_SIZE NUM_DIGITAL*2   // Array size for default state data
                                     // 2 bytes per digital I/O pin, 1st byte = State, 2nd byte = Value

// Default pin allocation
#define LIGHT_SENSORPIN 2 // Reserved pin for a Light sensor
#define FLOW_SENSORPIN 4  // Reserved pin for a flow sensor
#define DHT_PIN 12        // Reserved pin for DHT-22 sensor
#define WIRE_PIN  13      // Reserved pin for 1-Wire bus
#define PH_SENSORPIN 14   // Reserved pin for pH probe
#define TDS_SENSORPIN 15  // Reserved pin for TDS probe
#define LED_PIN 5         // Use the built-in led for visual feedback

#define cWatr_PIN 12      // Water pump control pin
#define sWatr_PIN A0      // Water pump sensor pin
#define cFill_PIN 12      // Fill pump control pin
#define sFill_PIN A0      // Fill pump sensor pin
#define cNutr_PIN 12      // Nutrient pump control pin
#define sNutr_PIN A0      // Nutrient pump sensor pin
#define cpHUp_PIN 12      // pHUp pump control pin
#define spHUp_PIN A0      // pHUp pump sensor pin
#define cpHDn_PIN 12      // pHDown pump control pin
#define spHDn_PIN A0      // pHDown pump sensor pin
#define cLamp_PIN 12      // Lamp control pin
#define sLamp_PIN A0      // Analog Lamp sensor pin

// Analog input pins are assumed to be used as analog input pins
const struct pin_config_struct pin_configurations[] = {
  // digital
  {DIGITAL_OUTPUT_PIN, HIGH}, //  0
  {DIGITAL_OUTPUT_PIN, HIGH}, //  1
  {DIGITAL_OUTPUT_PIN, HIGH}, //  2
  {DIGITAL_INPUT_PIN,  HIGH}, //  3
  {DIGITAL_OUTPUT_PIN, HIGH}, //  4
  {DIGITAL_OUTPUT_PIN, HIGH}, //  5
  {UNUSED_PIN,         LOW }, //  6
  {UNUSED_PIN,         LOW }, //  7
  {UNUSED_PIN,         LOW }, //  8
  {UNUSED_PIN,         LOW }, //  9
  {UNUSED_PIN,         LOW }, // 10
  {UNUSED_PIN,         LOW }, // 11
  {DIGITAL_OUTPUT_PIN, HIGH}, // 12
  {DIGITAL_OUTPUT_PIN, HIGH}, // 13
  {DIGITAL_OUTPUT_PIN, HIGH}, // 14
  {DIGITAL_OUTPUT_PIN, HIGH}, // 15
  {DIGITAL_OUTPUT_PIN, HIGH}, // 16
  // analog
  {ANALOG_INPUT_PIN,   5   }  // A0 input
};
#endif

#ifdef HN_ESP32
#define NUM_DIGITAL 54    // Number of digital I/O pins
#define PIN_MAP_SIZE NUM_DIGITAL*2   // Array size for default digital state data
                                     // 2 bytes per digital I/O pin, 1st byte = State, 2nd byte = Value
// Default pin allocation
#define LED_PIN 2         // Use the built-in led for visual feedback

#define DHT_PIN 13        // Reserved pin for DHT-22 sensor
#define WIRE_PIN 14       // Reserved pin for 1-Wire bus
#define SCL_PIN 22        // Default pin for I2C
#define SDA_PIN 21        // Default pin for I2C

#define cWtrPump_PIN 4    // Water pump control pin
#define sWtrFlow_PIN 36   // Water flow sensor pin
#define cWtrFill_PIN 5    // Fill pump control pin
#define sWtrFloat_PIN 39  // Fill float sensor pin

#define cNutr_PIN 15      // Nutrient pump control pin
#define sTDS_PIN 34       // Analog TDS sensor pin
#define cTDSO1_PIN 33     // Drive1 pin for TDS probe
#define sTDSI1_PIN 25     // Input1 pin for TDS probe
#define sTDSI2_PIN 26     // Input2 pin for TDS probe
#define cTDSO2_PIN 27     // Drive2 pin for TDS probe

#define cpHUp_PIN 17      // pHUp pump control pin
#define cpHDn_PIN 16      // pHDown pump control pin
#define spH_PIN   35      // pH sensor pin

#define cLamp_PIN 12      // Lamp control pin
#define sLux_PIN  32      // Light sensor pin

// Analog input pins are assumed to be used as analog input pins
const struct pin_config_struct pin_configurations[] = {
  // digital
  {UNUSED_PIN,         LOW }, //  0
  {UNUSED_PIN,         LOW }, //  1
  {DIGITAL_OUTPUT_PIN, HIGH}, //  2
  {UNUSED_PIN,         HIGH}, //  3
  {DIGITAL_OUTPUT_PIN, LOW }, //  4
  {DIGITAL_OUTPUT_PIN, HIGH}, //  5
  {UNUSED_PIN,         HIGH}, //  6
  {UNUSED_PIN,         HIGH}, //  7
  {UNUSED_PIN,         HIGH}, //  8
  {UNUSED_PIN,         HIGH}, //  9
  {UNUSED_PIN,         LOW }, // 10
  {UNUSED_PIN,         LOW }, // 11
  {DIGITAL_OUTPUT_PIN, LOW }, // 12
  {DIGITAL_OUTPUT_PIN, LOW }, // 13
  {DIGITAL_OUTPUT_PIN, LOW }, // 14
  {DIGITAL_OUTPUT_PIN, LOW }, // 15
  {UNUSED_PIN,         LOW }, // 16
  {UNUSED_PIN,         LOW }, // 17
  {DIGITAL_OUTPUT_PIN, LOW }, // 18
  {DIGITAL_OUTPUT_PIN, LOW }, // 19
  {UNUSED_PIN,         LOW }, // 20
  {DIGITAL_OUTPUT_PIN, LOW }, // 21
  {DIGITAL_OUTPUT_PIN, LOW }, // 22
  {DIGITAL_OUTPUT_PIN, LOW }, // 23
  {UNUSED_PIN,         LOW }, // 24
  {DIGITAL_OUTPUT_PIN, LOW }, // 25
  {DIGITAL_OUTPUT_PIN, LOW }, // 26
  {DIGITAL_OUTPUT_PIN, LOW }, // 27
  {UNUSED_PIN,         LOW }, // 28
  {UNUSED_PIN,         LOW }, // 29
  {UNUSED_PIN,         LOW }, // 30
  {UNUSED_PIN,         LOW }, // 31
  {DIGITAL_OUTPUT_PIN, LOW }, // 32
  {DIGITAL_OUTPUT_PIN, LOW }, // 33
  {DIGITAL_OUTPUT_PIN, LOW }, // 34
  {DIGITAL_OUTPUT_PIN, LOW }, // 35
  {UNUSED_PIN,         LOW }, // 36
  {UNUSED_PIN,         LOW }, // 37
  {UNUSED_PIN,         LOW }, // 38
  {UNUSED_PIN,         LOW }, // 39
  {UNUSED_PIN,         LOW }, // 40
  {UNUSED_PIN,         LOW }, // 41
  {UNUSED_PIN,         LOW }, // 42
  {UNUSED_PIN,         LOW }, // 43
  {UNUSED_PIN,         LOW }, // 44
  {UNUSED_PIN,         LOW }, // 45
  {UNUSED_PIN,         LOW }, // 46
  {UNUSED_PIN,         LOW }, // 47
  {UNUSED_PIN,         LOW }, // 48
  {UNUSED_PIN,         LOW }, // 49
  {UNUSED_PIN,         LOW }, // 50
  {UNUSED_PIN,         LOW }, // 51
  {UNUSED_PIN,         LOW }, // 52
  {UNUSED_PIN,         LOW }, // 53
  // analog
  {ANALOG_INPUT_PIN,   LOW }, // 54
  {ANALOG_INPUT_PIN,   LOW }, // 55
  {ANALOG_INPUT_PIN,   LOW }, // 56
  {ANALOG_INPUT_PIN,   LOW }, // 57
  {ANALOG_INPUT_PIN,   LOW }, // 58
  {ANALOG_INPUT_PIN,   LOW }, // 59
  {ANALOG_INPUT_PIN,   LOW }, // 60
  {ANALOG_INPUT_PIN,   LOW }, // 61
  {ANALOG_INPUT_PIN,   LOW }, // 62
  {ANALOG_INPUT_PIN,   LOW }, // 63
  {ANALOG_INPUT_PIN,   LOW }, // 64
  {ANALOG_INPUT_PIN,   LOW }, // 65
  {UNUSED_PIN,         LOW }, // 66
  {UNUSED_PIN,         LOW }, // 67
  {UNUSED_PIN,         LOW }, // 68
  {UNUSED_PIN,         LOW }  // 69
};
#endif

#define NUM_ANALOG (ARRAY_LENGTH(pin_configurations) - NUM_DIGITAL) // Number of analog I/O pins

#endif //HAPIBOARD_H
