# I2C communication with an MSP430X20XX as slave device  

This PlatformIO project contains two programs that enable I2C communication between an MSP430 and another I2C capable µcontroller as a master.

The I2C communication is realized with the USI (Universal Serial Interface) module, which is used on the MSP430X20XX models. The MSP430 software was created with the MSP430 framework from PlatformIO. The Arduino framework was used for the software of the I2C master. The software can e.g. serve as an I2C to serial gateway to display measured (register) values from the MSP430 controller on a serial console.

![Serial Output](https://github.com/DoImant/Stuff/blob/main/MSP430-I2C/MSP430-I2C.png)

## Arduino-I2C-Master-Slave  

This directory contains an example program for an I2C master software that can run on all I2C capable µcontrollers that can be programmed with the help of the Arduino framework. Depending on the value of the constant NUMBER_OF_BYTES, the program requests 2 (16 integer) or 4 bytes (32 bit integer) data from the slave.  The slave address can be specified in the file lib/bufferToint/bufferToint.h by adjusting the constant I2C_SLAVE_ADDRESS accordingly.

## MSP430-I2C-Slave

This sample program continuously reads data from the SD16_A (ADC) and sends the value from the register SD16MEM0 to the I2C master. The I2C implementation is located in the lib/ directory. There is a constant SLAVE_ADDR in the msp430-i2c.h file. The slave address is specified here. The address value must be shifted one bit to the left. For example, the address 0x24 corresponds to the value 0x48 in the slave configuration.

In order to save some memory space, the definition WITH_LED can be commented out. This deactivates the status LED functionality on PIN 1.0.

## Example circuit

![circuit](https://github.com/DoImant/Stuff/blob/main/MSP430-I2C/MP430-I2C-To-Pico.jpg)
