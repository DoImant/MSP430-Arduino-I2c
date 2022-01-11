//////////////////////////////////////////////////////////////////////////////
/// @brief I2C Slave Transmitter, multiple bytes using the USI
///        Data can only be sent but not received.
///         
/// 
//////////////////////////////////////////////////////////////////////////////
///
///  CONTROL REGISTERS
///
///  USICTL0 -> Control Register 0
///  --------------------------------------------------------
///    7   |  6   |  5   |  4   |   3  | 2   |  1  |    0   | 
///  --------------------------------------------------------
///  USIPE7|USIPE6|USIPE5|USILSB|USIMST|USIGE|USIOE|USISWRST|
///  USIPE7    (USI SDI/SDA port enable) = 1  ->  USI function enabled
///  USIPE6    (USI SDO/SCL port enable) = 1  ->  USI function enabled
///  USIPE5    (USI SCLK port enable)    = 0  ->  SCLK disable
///  USILSB    (LSB first)               = 0  ->  MSB first
///  USIMST    (Master)                  = 0  ->  Slave mode
///  USIGE     (Output latch control)    = 0  ->  Output latch enable depends on shift clock
///  USIOE     (Serial data output enable)     = 0b  ->  Output enabled
///  USISWRST  (USI software reset)      = 1  ->  Software reset
///
///  USICTL1 -> Control Register 1
///  --------------------------------------------------------------
///  |   7    |  6   |   5    |  4  | 3   |  2   |    1    |  0   |
///  |------------------------------------------------------------|
///  |USICKPH |USII2C|USISTTIE|USIIE|USIAL|USISTP|USISTTIFG|USIIFG|
///  |------------------------------------------------------------|
///  USICKPH   (Clock phase select)               = 0  ->  Data is changed on the first SCLK edge and captured on the following edge.
///  USII2C    (I2C mode enable)                  = 1  ->  I2C mode enabled
///  USISTTIE  (START condition interrupt-enable) = 1  ->  Interrupt on START condition enabled
///  USIIE     (USI counter interrupt enable)     = 1  ->  Interrupt enabled
///  USIAL     (Arbitration lost)                 = 0  ->  Not used
///  USISTP    (STOP condition received)          = 0  ->  Not used
///  USISTTIFG (START condition interrupt flag)   = 0  ->  Not used
///  USIIFG    (USI counter interrupt flag)       = 0  ->  No interrupt pending
///                
///  USICKCTL, USI Clock Control Register
///  -------------------------------------
///  |   7-5  |   4-2   |   1   |   0    |
///  ------------------------------------
///  |USIDIVx |USISSELx |USICKPL|USISWCLK|
///  -------------------------------------
///  USIDIVx (Clock divider)   = 000 -> Divide by 1
///  USISSELx (Clock source)   = 000 -> SCLK 
///  USICKPL (Clock polarity)  = 1   -> Inactive state is high
///  USISWCLK (Software clock) = 0 (not used)
///               
///  USICNT, USI Bit Counter Register
///  --------------------------------------
///  |     7   |   6   |     5    |  4-0  |
///  ------------------------------------
///  |USISCLREL| USI16B |USIIFGCC |USICNTx|
///  --------------------------------------
///  USISCLREL (SCL release)                     = 0 -> SCL line is held low if USIIFG is set
///  USI16B (16-bit shift register enable)       = 0 -> 8-bit shift register mode
///  USIIFGCC (USI interrupt flag clear control) = 1 -> USIIFG is not cleared automatically
///  USICNTx (USI bit count)                     = 00000 (not relevant at this moment)
///
//////////////////////////////////////////////////////////////////////////////

#include <msp430.h>
#include "msp430-i2c.h"

//////////////////////////////////////////////////////////////////////////////
/// Module global variables
//////////////////////////////////////////////////////////////////////////////
//extern uint8_t slvData[NUMBER_OF_BYTES];
uint8_t slvData[NUMBER_OF_BYTES]; 


//////////////////////////////////////////////////////////////////////////////
/// @brief Interrupt function for processing the I2C protocol. 
///        Only data is sent, not received.
/// 
//////////////////////////////////////////////////////////////////////////////
#pragma vector = USI_VECTOR
__interrupt void USI_TXRX (void)
{
  static uint8_t i2cState;     
  static uint8_t dataIdx = 0;
  if (USICTL1 & USISTTIFG)  {             // Start entry?
#ifdef WITH_LED
    P1OUT |= 0x01;                        // LED on: sequence start
#endif
    i2cState = I2C_ADDRESS;               // Enter 1st state on start
  }

  switch(i2cState)  {
    case I2C_IDLE:             // Idle, wait for next start
      break;

    case I2C_ADDRESS:          // RX Address
      USICNT = (USICNT & 0xE0) + 0x08;    // Bit counter = 8, RX address
      USICTL1 &= ~USISTTIFG;              // Clear start flag
      i2cState = I2C_PROCESS_ADDRESS;     // Check address
      break;

    case I2C_PROCESS_ADDRESS:  // Process Address and send (N)Ack
      if (USISRL & 0x01) {                // If master read...
        slvAddr++;                        // Save R/W bit
      }
      USICTL0 |= USIOE;                   // SDA = output
      if (USISRL == slvAddr) {            // Oh. Somebody wants something from me.
        USISRL = 0x00;                    // Send Ack
#ifdef WITH_LED
        P1OUT &= ~0x01;                   // LED off
#endif
        i2cState = I2C_TX_DATA;           // send data
      } else {                              
        USISRL = 0xFF;                    // No. That's none of my business. Send NAck
#ifdef WITH_LED
        P1OUT |= 0x01;                    // LED on
#endif
        i2cState = I2C_RX_CHECK;          // prep for next Start
      } 
      USICNT |= 0x01;                     // Bit counter = 1, send (N)Ack bit
      break; 

    case I2C_RX_CHECK:                    // This slave does not receive anything. -> Prep next start
      i2cState = prepStart();
      break;

    case I2C_TX_DATA:          // Send first data byte
      i2cState = txData(&dataIdx);        // Attention! dataIdx will be incremented!
      break;

    // This status checks the (N)ACK status between each data byte.
    case I2C_ACK_NACK:         // Receive Data (N)Ack
      USICTL0 &= ~USIOE;                  // SDA = input
      USICNT |= 0x01;                     // Bit counter = 1, receive (N)Ack
      i2cState = I2C_TX_CHECK;            // Check (N)Ack
      break;

    case I2C_TX_CHECK:         // Process Data Ack/NAck
      if (USISRL & 0x01) {                // If Nack received...
        i2cState = prepStart();
        dataIdx=0;                        // Reset data index
      } else {                            // ... else Ack received
        if (dataIdx < NUMBER_OF_BYTES) {
          i2cState = txData(&dataIdx);    // TX next byte. Attention! dataIdx will be incremented!
        } else {
          i2cState = prepStart();
          dataIdx=0;                      // Reset data index
        }
#ifdef WITH_LED
        P1OUT &= ~0x01;                   // LED off
#endif
      }
      break;
  }
  USICTL1 &= ~USIIFG;                     // Clear pending flags
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Initialize this microcontroller as an I2C slave.
/// 
//////////////////////////////////////////////////////////////////////////////
void i2cSlaveSetup(void){
  P1OUT = BIT6 | BIT7;                    // P1.6 & P1.7 OUT
  //P1REN |= BIT6 + BIT7                  // P1.6 & P1.7 Pullups

  USICTL0 |= USISWRST;                    // Do config -> Disable USI
  USICTL0 = USIPE6+USIPE7+USISWRST;       // Port & USI mode setup
  USICTL1 = USII2C+USIIE+USISTTIE;        // Enable I2C mode & USI interrupts
  USICKCTL = USICKPL;                     // Setup clock polarity
  USICNT |= USIIFGCC;                     // Disable automatic clear control
  USICTL0 &= ~USISWRST;                   // Config ready -> Enable USI
  USICTL1 &= ~USIIFG;                     // Clear pending flag
  __enable_interrupt();
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Transfer of a data byte.
/// 
/// @param pIdx       Pointer to the index of the data field.
///                   Attention: the index value will be inkremented!
/// @return uint8_t   next I2C state  
//////////////////////////////////////////////////////////////////////////////
uint8_t txData(unsigned char* pIdx) {
  USICTL0 |= USIOE;                       // SDA = output
  USISRL = slvData[*pIdx];
  USICNT |=  0x08;                        // Bit counter = 8, TX data
  (*pIdx)++;
  return I2C_ACK_NACK;                    // next state: receive (N)Ack
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Reset I2C to Idle and wait for the next start 
/// 
/// @return uint8_t next I2C state 
//////////////////////////////////////////////////////////////////////////////
uint8_t prepStart(void) {
  USICTL0 &= ~USIOE;                      // SDA = input
  slvAddr = (SLAVE_ADDR << 1);            // Reset slave address
  return I2C_IDLE;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Break down the 16 Bit integer number into individual bytes 
///        for transmission
/// 
/// @param val The 16 Bit integer value
//////////////////////////////////////////////////////////////////////////////
void setTxData16(uint16_t val) {
  slvData[0] = (val>>8) & 0xFF;
  slvData[1] = val & 0xFF;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Break down the 32 Bit integer number into individual bytes 
///        for transmission
/// 
/// @param val The 32 Bit integer value
//////////////////////////////////////////////////////////////////////////////
void setTxData32(uint32_t val) {
  slvData[0] = (val>>24) & 0xFF;
  slvData[1] = (val>>16) & 0xFF;
  slvData[2] = (val>>8) & 0xFF;
  slvData[3] = val & 0xFF;
}
