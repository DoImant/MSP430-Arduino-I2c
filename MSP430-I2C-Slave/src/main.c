
//////////////////////////////////////////////////////////////////////////////
/// @file Sample program for i2c communication.
///       The program represents an I2C slave that can only send data.
///
/// @author Kai R.
/// @brief 
/// 
/// @date 2022-01-05
/// @version 1.0
///                
///               MSP430F20x2/3        
///             -----------------      
///          ^ |                 |    
///          | |                 |     
///          --|RST              |    
///            |                 |     
///      LED <-|P1.0             |     
///            |                 |     
///            |         SDA/P1.7|------->
///            |         SCL/P1.6|<-------
/// 
//////////////////////////////////////////////////////////////////////////////

#include <msp430.h>
#include "msp430-i2c.h"

//////////////////////////////////////////////////////////////////////////////
/// Global definitions
//////////////////////////////////////////////////////////////////////////////

#define NUM_OF_ADCVALUES 11               // Value has to be odd

//////////////////////////////////////////////////////////////////////////////
/// Module global variables
//////////////////////////////////////////////////////////////////////////////

//uint8_t slvData[NUMBER_OF_BYTES] = {'A','1','C','2'};   // Data buffer (extern in msp430-i2c.c) !!!

//////////////////////////////////////////////////////////////////////////////
/// Forward declaration(s)
//////////////////////////////////////////////////////////////////////////////

void sd16Setup(void);
uint16_t getMedian(uint16_t* data, uint8_t items); 

//////////////////////////////////////////////////////////////////////////////
/// @brief Mainprogram:  Reading of voltage data on the ADC and 
///                      transmission of the measured values to an I2C master
/// 
/// @return int 
//////////////////////////////////////////////////////////////////////////////
int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog
  if (CALBC1_1MHZ==0xFF)	{				        // If calibration constants erased											
    while(1);                             // do not load, trap CPU!!	
  }

  DCOCTL = 0;                             // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ;                  // Set DCO
  DCOCTL = CALDCO_1MHZ;

  uint16_t median;              
  uint16_t adcValues[NUM_OF_ADCVALUES];   // Array with the data for which 
                                          // the meriadian is to be found
  uint8_t idx = 0;

  i2cSlaveSetup();
  sd16Setup();
  while(1) {
    SD16CCTL0 &= ~SD16IFG;                // clear ADC Interrupt Flag
    SD16CCTL0 |= SD16SC;                  // start ADC
    while (!(SD16CCTL0 & SD16IFG)) { 
    }                                     // AD conversion ended? 
    adcValues[idx] = SD16MEM0;            
    idx++;
    if (idx == NUM_OF_ADCVALUES) {
      median = getMedian(adcValues, NUM_OF_ADCVALUES); 
      __disable_interrupt();
      setTxData16(median);                // Update ADC value to I2C Databuffer
      __enable_interrupt();
      __delay_cycles(25000);
      idx = 0;
    }
  }  
 // LPM0;                                   // CPU off, await USI interrupt
 // __no_operation();
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Set up ADC in single mode without interrupt
/// 
//////////////////////////////////////////////////////////////////////////////
void sd16Setup() {
  SD16CTL = SD16REFON;      // Referenzvoltage µC, 1200 mV
  SD16CTL |= SD16SSEL_1;    // Clk is SMCLK
  SD16INCTL0 = SD16INCH_1;  // ADC-input via A1+; P1.2
  SD16AE = SD16AE2;         // Input viar A1+; A1- internal GND
  SD16CCTL0 = SD16UNI;      // Unipolar Input 0 up to 0xFFFF
  SD16CCTL0 |= SD16SNGL;    // Single-Mode; 
  SD16CCTL0 |= SD16DIV_0;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Determining the median from a list of measured values
/// 
/// @param data           Measured values
/// @param items          Number of measured values
/// @return uint16_t      Return median value 
//////////////////////////////////////////////////////////////////////////////
uint16_t getMedian(uint16_t* data, uint8_t items)
{ 
  uint16_t tmp;
  uint8_t i, j;

  for (i = 0; i < items; ++i) {
    for (j = 0; j < items - i - 1; ++j) {
      if (data[j] > data[j + 1]) {
        tmp = data[j];
        data[j] = data[j + 1];
        data[j + 1] = tmp;
      }
    }
  }
  tmp = (items >> 1);                     // items/2
  return data[tmp];                       // The result set is odd. Simply return 
                                          // the mean value of the set of values.
}
