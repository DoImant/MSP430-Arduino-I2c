#ifndef _MSP430_I2C_H_
#define _MSP430_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <stdint.h>

//////////////////////////////////////////////////////////////////////////////
/// Global definitions
//////////////////////////////////////////////////////////////////////////////

#define SLAVE_ADDR  0x24                    // Slave-Address 
#define NUMBER_OF_BYTES  4U                 // Bytes for transfer data buffer (2 or 4)
#define WITH_LED

typedef enum I2C_ModeEnum{                  // States for Statemachine
    I2C_IDLE = 0,
    I2C_ADDRESS = 2,
    I2C_PROCESS_ADDRESS = 4,
    I2C_RX_DATA = 6,
    I2C_RX_CHECK = 8,
    I2C_TX_DATA = 10,
    I2C_ACK_NACK = 12,
    I2C_TX_CHECK = 14
} I2C_Mode;

//////////////////////////////////////////////////////////////////////////////
/// Global variables
//////////////////////////////////////////////////////////////////////////////

static uint8_t slvAddr = (SLAVE_ADDR << 1);        // Init Address of this slave 
                                                   // 7 Bit Address is << 1 for R/W       

//////////////////////////////////////////////////////////////////////////////
/// Forward declaration(s)
//////////////////////////////////////////////////////////////////////////////

void i2cSlaveSetup(void);
uint8_t txData(uint8_t* data_idx);
uint8_t prepStart(void);
void setTxData16(uint16_t val);
void setTxData32(uint32_t val);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
