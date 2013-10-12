// TI File $Revision: /main/2 $
// Checkin $Date: April 29, 2005   11:10:14 $
//###########################################################################
//
// FILE:	DSP281x_Gpio.c
//
// TITLE:	DSP281x General Purpose I/O Initialization & Support Functions.
//
//###########################################################################
// $TI Release: DSP281x C/C++ Header Files V1.20 $
// $Release Date: July 27, 2009 $
//###########################################################################

#include "DSP281x_Device.h"     // DSP281x Headerfile Include File
#include "DSP281x_Examples.h"   // DSP281x Examples Include File

//---------------------------------------------------------------------------
// InitGpio: 
//---------------------------------------------------------------------------
// This function initializes the Gpio to a known state.
//
void InitGpio(void)
{

// Set GPIO A port pins,AL(Bits 7:0)(input)-AH(Bits 15:8) (output) 8bits
/*
* Name Address Size (x16) Register Description
* GPAMUX 0x0000 70C0 1 GPIO A MUX Control Register
	* Each I/O port has one MUX register. The MUX registers are used to select
	* between the corresponding peripheral operation and the I/O operation of each
	* of the I/O pins. At reset all GP I/O pins are configured as I/O pins.
		* If GPxMUX.bit = 0, then the pin is configured as an I/O
		* If GPxMUX.bit = 1, then the pin is configured for the peripheral functionality

* Name Address Size (x16) Register Description
* GPADIR 0x0000 70C1 1 GPIO A Direction Control Register
	* Each I/O port has one direction control register. The direction register controls
	* whether the corresponding I/O pin is configured as an input or an output. At
	* reset all GP I/O pins are configured as inputs.
		* If GPxDIR.bit = 0, then the pin is configured as an input
		* If GPxDIR.bit = 1, then the pin is configured as an output
	* On reset, the default value for all GPxMUX and GPxDIR register bits is 0

* GPAQUAL 0x0000 70C2 1 GPIO A Input Qualification Control Register
*
* If configured for Digital I/O mode, additional registers are provided for setting
* individual I/O signals (via the GPxSET registers), for clearing individual I/O
* signals (via the GPxCLEAR registers), for toggling individual I/O signals (via
* the GPxTOGGLE registers), or for reading/writing to the individual I/O signals
* (via the GPxDAT registers).
*
* Therefore, when a pin is configured for GPIO operation, the corresponding peripheral
* functionality (and interrupt/generating capability) must be disabled. Otherwise,
* interrupts may be inadvertently triggered.

PWM1

*/

// Input Qualifier =0, none
// Set GPIO A port pins,AL(Bits 7:0)(input)-AH(Bits 15:8) (output) 8bits
// Input Qualifier =0, none
     EALLOW;
     GpioMuxRegs.GPAMUX.all=0x0000;
     GpioMuxRegs.GPADIR.all=0xFFFF;    	// upper byte as output/low byte as input
     GpioMuxRegs.GPAQUAL.all=0x0000;	// Input qualifier disabled

// Set GPIO B port pins, configured as EVB signals
// Input Qualifier =0, none
// Set bits to 1 to configure peripherals signals on the pins
     GpioMuxRegs.GPBMUX.all=0xFFFF;
     GpioMuxRegs.GPBQUAL.all=0x0000; // Input qualifier disabled

// GPFMUX
     GpioMuxRegs.GPFMUX.bit.XF_GPIOF14 = 0; // muxed to gpio
     GpioMuxRegs.GPFDIR.bit.GPIOF14 = 1; // output
     GpioDataRegs.GPFCLEAR.bit.GPIOF14 = 1; // clear the led light
     EDIS;
}	
	
//===========================================================================
// No more.
//===========================================================================
