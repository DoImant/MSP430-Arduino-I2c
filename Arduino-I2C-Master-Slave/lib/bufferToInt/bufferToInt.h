#ifndef _BUFFER_TO_INT_H_
#define _BUFFER_TO_INT_H_

#include <stdint.h>

#define IAM_SLAVE_OFF
#define I2C_SLAVE_ADDRESS 0x24        // Can be changed

#ifdef IAM_SLAVE
void requestEvent(void);
#endif

//////////////////////////////////////////////////////////////////////////////
/// @brief Convert the bytes from a charBuf back to a 16 Bit 
///        signed/unsigned number
/// 
/// @param charBuf      Buffer with byte data
/// @return int16_t     Integer number put together from the charBuf data
//////////////////////////////////////////////////////////////////////////////
template <typename R, typename ARG1>
R bufferToInt16(ARG1 charBuf) {
  uint16_t val = 0;
  val = *charBuf;
  val = (val << 8) | *(charBuf+1);
  return val;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Convert the bytes from a charBuf back to a 32 Bit
///        signed/unsigned number
/// 
/// @param charBuf      Buffer with byte data
/// @return int32_t     Integer number put together from the charBuf data
//////////////////////////////////////////////////////////////////////////////
template <typename R, typename ARG1>
R bufferToInt32(ARG1 charBuf) { 
  uint32_t val = 0;
  val = charBuf[0];
  val = (val << 8) | charBuf[1];
  val = (val << 8) | charBuf[2];
  val = (val << 8) | charBuf[3];
  return val;
}

#endif