/* -----------------------------------------------------------------------------
   @file    CurveShieldR1.ino

   This firmware is for an IV curve tracer Arduino shield that was designed for
   a graduate project in ECEN 5730, Practical PCB Design at CU, Boulder. It
   relies on the following libraries:
      1) "Adafriut_MCP4725" - the library for setting DAC output
         Further info: https://learn.adafruit.com/mcp4725-12-bit-dac-tutorial/using-with-arduino
      2) "ADS1115_WE" - the library for reading ADC inputs
         Further info: https://wolles-elektronikkiste.de/ads1115
   Big thank you to the above individuals/teams for writing that code!

   The IV Curve shield

   @author  Jake Michael
   @date    2020-11-09
   @rev     1.0
   @notes   The
   -----------------------------------------------------------------------------
*/

#include <Wire.h>
// MCP4725 DAC Library
#include <Adafruit_MCP4725.h>
// ADS1115 ADC Library
#include<ADS1115_WE.h>

// I2C 7-bit Addresses (no R/W bit)
#define MCP4725_I2C_ADDRESS (0x60)
#define ADS1115_I2C_ADDRESS (0x48)

// I2C bus freq
#define I2C_CLK_FREQ     (400000) // fast mode

// define the library objects
Adafruit_MCP4725 dac;
ADS1115_WE adc(ADS1115_I2C_ADDRESS);

/*
   @brief   Setup function run before main loop
*/
void setup() {
  // set baud for comms
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("Communciation Success.");

  // init i2c interface
  Wire.begin();
  // set to 400 kHz (fast mode)
  Wire.setClock(I2C_CLK_FREQ);

  // initialize dac, adc
  if (!dac.begin(MCP4725_I2C_ADDRESS)) {
    Serial.println("MCP4725 not found.");
  }

  if (!adc.init()) {
    Serial.println("ADS1115 not found.");
  }

  // set adc parameters
  adc.setVoltageRange_mV(ADS1115_RANGE_6144);
  adc.setConvRate(ADS1115_128_SPS);
}

uint32_t counter;
int16_t diffCh1;
int16_t diffCh2;
bool isProcessInitiated = false;
bool isdacSetOff = false;
String buf;

/*
   @brief   Main loop, will never return
*/
void loop() {
  if (!isProcessInitiated) {
    // read serial string
    buf = Serial.readStringUntil('\n');

    // compare the strings, if not equal skip iteration of while loop
    if ( buf.startsWith("START") ) {
      isProcessInitiated = true;
    }
    if (!isdacSetOff) {
      dac.setVoltage(0, false, I2C_CLK_FREQ);
      isdacSetOff = true;
    }
  }
  else {
    // Run through the 12-bit scale on dac, and readout raw adc values
    for (counter = 0; counter < 4095; counter += 8)
    {
      // set dac voltage
      dac.setVoltage(counter, false, I2C_CLK_FREQ);

      // read RSense
      diffCh1 = ADS1115_Read(ADS1115_COMP_0_1);
      // read DUT
      diffCh2 = ADS1115_Read(ADS1115_COMP_2_3);

      // send data over UART
      Serial.print(counter); Serial.print(",");
      Serial.print(diffCh1); Serial.print(",");
      Serial.print(diffCh2); Serial.print("\r\n");

      // set dac voltage to 0
      dac.setVoltage(0, false, I2C_CLK_FREQ);

      // delay for some time
      delay(20);
    }

    // go back to wait state to initiate another process
    isProcessInitiated = false;
    isdacSetOff = false;
  }
  
} // end loop()

/*
   @brief   Reads a channel from ads1115
*/
int16_t ADS1115_Read(ADS1115_MUX channel) {
  int16_t rawResult = 0;
  adc.setCompareChannels(channel);
  adc.startSingleMeasurement();
  while (adc.isBusy()) {
    ;
  }
  rawResult = adc.getRawResult();
  return rawResult;
}

/*
  The Adafriut_MCP4725 library compels us to keep this notice:

  Software License Agreement (BSD License)

  Copyright (c) 2012, Adafruit Industries
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  3. Neither the name of the copyright holders nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
