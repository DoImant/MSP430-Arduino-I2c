//////////////////////////////////////////////////////////////////////////////
/// @file main.cpp
/// @author Kai R.
/// @brief Sample programm for I2C master - slave communication
/// 
/// @date 2022-01-03
/// @version 1.0
/// 
/// @copyright Copyright (c) 2022
/// 
//////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <Wire.h>

#include "bufferToInt.h"

#define UNSIGNED                    // Display values are unsigned. Comment out for display signed values 
#define NUMBER_OF_BYTES 2           // Can be 2 or 4.

//////////////////////////////////////////////////////////////////////////////
/// global variable(s)
//////////////////////////////////////////////////////////////////////////////
#if (NUMBER_OF_BYTES == 2)
  uint8_t buffer[NUMBER_OF_BYTES] = {0,0};
#elif (NUMBER_OF_BYTES == 4)
  uint8_t buffer[NUMBER_OF_BYTES] = {0,0,0,0};
#else
  #error Incorrect value specified for NUMBER_OF_BYTES. Only the value 2 or 4 may be specified.
#endif

#ifndef IAM_SLAVE
//////////////////////////////////////////////////////////////////////////////
/// @brief I2C Master:  This is an I2C to Serial Converter. 
///                     Numbers are received from an I2C slave and displayed 
///                     on the serial console.
/// 
//////////////////////////////////////////////////////////////////////////////
void setup()
{
  Wire.begin(); 
  Serial.begin(115200);
}

void loop()
{
  static char outText[41];
  static uint16_t adcValue = 0;
  static double voltage = 0;
  int idx = 0;
  
  Wire.requestFrom(I2C_SLAVE_ADDRESS, NUMBER_OF_BYTES);   // request NUMBER_OF_BYTES byte
                                                          // from slave device I2C_SLAVE_ADDRESS
  //Serial.print("Received: ");
  while(Wire.available())    // slave may send less than requested
  {
    if (idx < NUMBER_OF_BYTES) {
      buffer[idx] = Wire.read(); // receive a byte as character
      idx++;
    } else {
      break;
    }
  }

#if NUMBER_OF_BYTES == 2
  #ifdef UNSIGNED
    adcValue = bufferToInt16<uint16_t>(buffer);
    voltage = (0.6 / 65535) * adcValue;
    sprintf(outText,"ADC-Value: %d -> Voltage: %1.3f V",adcValue, voltage);
    Serial.println(outText);
  #else
    Serial.println(bufferToInt16<int16_t>(buffer));
  #endif
#else
  #ifdef UNSIGNED
  Serial.println(bufferToInt32<uint32_t>(buffer));
  #else
  Serial.println(bufferToInt32<int32_t>(buffer));
  #endif
#endif
  delay(1000);
}
#else
void setup() {
  pinMode(PIN_LED,OUTPUT);
  Wire.begin(I2C_SLAVE_ADDRESS); 
  Wire.onRequest(requestEvent);
}

void loop() {
  //nix 
}

void requestEvent() {
  static byte digits[2] = {0x41,0x42};
  Wire.write(digits,2);
  digits[0] += 2;
  digits[1] += 2;
  if(digits[0] > 0x5A) {
    digits[0] = 0x41;
    digits[1] = 0x42;
  }  
}
#endif

