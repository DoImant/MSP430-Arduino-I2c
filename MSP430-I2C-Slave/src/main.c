
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

//uint8_t slvData[NUMBER_OF_BYTES] = {'A','1','C','2'};   // Data buffer (extern in msp430-i2c.c) !!!

//////////////////////////////////////////////////////////////////////////////
/// Forward declaration(s)
//////////////////////////////////////////////////////////////////////////////

void sd16Setup(void);

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog
  if (CALBC1_1MHZ==0xFF)					        // If calibration constants erased
  {											
    while(1);                             // do not load, trap CPU!!	
  }

  DCOCTL = 0;                             // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ;                  // Set DCO
  DCOCTL = CALDCO_1MHZ;
    
  i2cSlaveSetup();
  sd16Setup();

  while(1) {
    SD16CCTL0 &= ~SD16IFG;                // clear ADC Interrupt Flag
    SD16CCTL0 |= SD16SC;                  // Start ADC
    while (!(SD16CCTL0 & SD16IFG)) { 
    }                                     // ADC ready?
    __disable_interrupt();
    setTxData16(SD16MEM0);                // transmit ADC value
    __enable_interrupt();
    __delay_cycles(25000);
  } 
 // LPM0;                                   // CPU off, await USI interrupt
 // __no_operation();
}

void sd16Setup() {
  SD16CTL = SD16REFON;      // Referenzvoltage µC, 1200 mV
  SD16CTL |= SD16SSEL_1;    // Clk is SMCLK
  SD16INCTL0 = SD16INCH_1;  // ADC-input via A1+; P1.2
  SD16AE = SD16AE2;         // Input viar A1+; A1- internal GND
  SD16CCTL0 = SD16UNI;      // Unipolar Input 0 up to 0xFFFF
  SD16CCTL0 |= SD16SNGL;    // Single-Mode; 
  SD16CCTL0 |= SD16DIV_0;
}
