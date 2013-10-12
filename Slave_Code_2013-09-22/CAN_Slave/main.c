/*
 * INGENIARTS
 * Initial release : 2013-09-21
 * created by Rami Jarjour and Timur Ridjanovic
 * Last revision : 2013-09-21
 */


#include "DSP281x_Device.h"     // DSP281x Headerfile Include File
#include "DSP281x_Examples.h"   // DSP281x Examples Include File
#include "functions.h"   // DSP281x Examples Include File

// Prototype statements for functions found within this file.
interrupt void eCAN0INT_ISR(void);

Uint32 ErrorCount;
Uint32 TestMbox1;
Uint32 TestMbox2;
Uint32 TestMbox3;
Uint32 MessageHigh;
Uint32 MessageLow;
Uint32 MessageToBeSent;

void main(void)
{


// Step 1. Initialize System Control:
// PLL, WatchDog, enable Peripheral Clocks
// This example function is found in the DSP281x_SysCtrl.c file.
   InitSysCtrl();


// Disable CPU interrupts
   DINT;

// Initialize PIE control registers to their default state.
// The default state is all PIE interrupts disabled and flags
// are cleared.
// This function is found in the DSP281x_PieCtrl.c file.
   InitPieCtrl();

// Disable CPU interrupts and clear all CPU interrupt flags:
   IER = 0x0000;
   IFR = 0x0000;

   InitPieVectTable();

   InitGpio();

   InitECan();

   initEcanInterrupt();

   initMBoxes();

//   sendMessage(3, 4);

   while(1)	//waiting for messages from master
   {
	   if(MessageToBeSent == 1)
	   {
		   sendMessage(2*TestMbox1, 2*TestMbox2);
	   }

   }
}
