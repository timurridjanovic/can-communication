/* INGENIARTS
 * Initial release: 2013-09-21
 * Created by Timur Ridjanovic and Rami Jarjour
 * Last revision: 2013-09-21 */


#include "DSP281x_Device.h"     // DSP281x Headerfile Include File
#include "DSP281x_Examples.h"   // DSP281x Examples Include File

// Prototype statements for functions found within this file.
interrupt void eCAN0INT_ISR(void);
void initEcanInterrupt(void);
void sendMessage(Uint32 messageLow, Uint32 messageHigh);
void initMBoxes(void);
void mailbox_read(int16 MBXnbr);
void delay_loop(void);


// Global variables for this example
Uint32  ErrorCount;
Uint32  Success;

Uint32  TestMbox1 = 0;
Uint32  TestMbox2 = 0;
Uint32  TestMbox3 = 0;
Uint32 MessageHigh = 0;
Uint32 MessageLow = 0;

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

	   InitGpio(); // changed the gpio.c file to close LED light

	   InitECan();

	   initEcanInterrupt();

	   initMBoxes(); //for Master

	   int y = 3;
	   int x =14;

	   while(1) {
		   y++;
		   x++;
		   sendMessage(y, x);
		   delay_loop();

	   }


}

void initEcanInterrupt(void) {
	struct ECAN_REGS ECanaShadow;

	EALLOW;

	// set CANGIM to 1 to connect to the I0EN line; check eCAN spru074F interrupt scheme p.76
	ECanaShadow.CANGIM.all = ECanaRegs.CANGIM.all;
	ECanaShadow.CANGIM.bit.I0EN = 1;
	ECanaRegs.CANGIM.all = ECanaShadow.CANGIM.all;


	// set CANMIL to 0 to connect to the I0EN line
	ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
	ECanaShadow.CANMIL.bit.MIL0 = 0;
	ECanaShadow.CANMIL.bit.MIL1 = 0;
	ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;

	// set CANMIM to 1 to enable the interrupt
	ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
	ECanaShadow.CANMIM.bit.MIM0 = 1;
	ECanaShadow.CANMIM.bit.MIM1 = 1;
	ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;

	// Interrupts that are used in this example are re-mapped to
	// ISR functions found within this file.
	PieVectTable.ECAN0INTA = &eCAN0INT_ISR;

	//Configure Pie interrupts (check system control and interrupts scheme p.107 and 119)
	PieCtrlRegs.PIECRTL.bit.ENPIE = 1; //Enable Pie interrupts
	PieCtrlRegs.PIEACK.bit.ACK9 = 1; //Set to 1 to clear the bit (inversed logic)

	PieCtrlRegs.PIEIER9.bit.INTx5 = 1; //Enable INTx5 of INT9 (eCAN0INT) (eCAN1INT is INTx6)

	IER |= 0x0100; // Enable INT9 (on bit number 8) p.138 system control and interrupts
	EINT; // Enable Global interrupt INTM

	EDIS; //peripheral interrupt phase is done... Now we need to set up the PIE interrupt section
}



void initMBoxes(void) {
	// eCAN control registers require read/write access using 32-bits.  Thus we
	// will create a set of shadow registers for this example.  These shadow
	// registers will be used to make sure the access is 32-bits and not 16.
	struct ECAN_REGS ECanaShadow;

	// Mailboxs can be written to 16-bits or 32-bits at a time
	// Write to the MSGID field of TRANSMIT mailboxes MBOX0
	ECanaMboxes.MBOX0.MSGID.all = 0xC0000013; // C = mask ; 9 = filter
	ECanaMboxes.MBOX1.MSGID.all = 0x91000001;

	// Configure Mailbox 0 as Tx (0) and Mailbox 1 as Rx (1)
	ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
	ECanaShadow.CANMD.bit.MD0 = 0;
	ECanaShadow.CANMD.bit.MD1 = 1;
	ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;



	// Enable Mailboxes 0 and 1
	// a shadow register is required.
	ECanaShadow.CANME.all = ECanaRegs.CANME.all;
	ECanaShadow.CANME.bit.ME0 = 1;
	ECanaShadow.CANME.bit.ME1 = 1;
	ECanaRegs.CANME.all = ECanaShadow.CANME.all;

	// Specify that 8 bits will be sent
	ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;


}



void sendMessage(Uint32 messageLow, Uint32 messageHigh) {

	struct ECAN_REGS ECanaShadow;
    MessageLow = messageLow;
    MessageHigh = messageHigh;

    // Write to the mailbox RAM field of MBOX0 - 15
    ECanaMboxes.MBOX0.MDL.all = messageLow;
    ECanaMboxes.MBOX0.MDH.all = messageHigh;


    // Begin transmitting
    ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
    ECanaShadow.CANTRS.bit.TRS0 = 1;
    ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;



}


interrupt void eCAN0INT_ISR(void) {
	struct ECAN_REGS ECanaShadow;
	ECanaShadow.CANTA.all = ECanaRegs.CANTA.all;
	ECanaShadow.CANRMP.all = ECanaRegs.CANRMP.all;

	if (ECanaShadow.CANTA.bit.TA0 == 1) {  // inversed logic
		ECanaShadow.CANTA.all = 0;
		ECanaShadow.CANTA.bit.TA0 = 1;
		ECanaRegs.CANTA.all = ECanaShadow.CANTA.all; //clear TAn


		if (GpioDataRegs.GPFDAT.bit.GPIOF14 == 0) {
			GpioDataRegs.GPFSET.bit.GPIOF14 = 1;
			//delay_loop();
		}

		else {
			GpioDataRegs.GPFCLEAR.bit.GPIOF14 = 1;
			//delay_loop();
		}


	}

	else if (ECanaShadow.CANRMP.bit.RMP1 == 1) {  // inversed logic
		ECanaShadow.CANRMP.all = 0;
		ECanaShadow.CANRMP.bit.RMP1 = 1;
		ECanaRegs.CANRMP.all = ECanaShadow.CANRMP.all; //clear RMP

		mailbox_read(1);
	}

	else {
		ErrorCount++;
	}

	PieCtrlRegs.PIEACK.bit.ACK9 = 1; // clear aknowledge bit

}


// This function reads out the contents of the indicated
// by the Mailbox number (MBXnbr).
void mailbox_read(int16 MBXnbr)
{
   volatile struct MBOX *Mailbox;
   Mailbox = &ECanaMboxes.MBOX0 + MBXnbr;
   TestMbox1 = Mailbox->MDL.all; // = 0x9555AAAn (n is the MBX number)
   TestMbox2 = Mailbox->MDH.all; // = 0x89ABCDEF (a constant)
   TestMbox3 = Mailbox->MSGID.all;// = 0x9555AAAn (n is the MBX number)

} // MSGID of a rcv MBX is transmitted as the MDL data.


void delay_loop(void) {
	Uint32 i;
	for (i=0; i<2000000; i++) {}
}






/*
     // Mailboxs can be written to 16-bits or 32-bits at a time
    // Write to the MSGID field of TRANSMIT mailboxes MBOX0 - 15
    ECanaMboxes.MBOX0.MSGID.all = 0xC0000001;
    ECanaMboxes.MBOX1.MSGID.all = 0xC0000002;
    ECanaMboxes.MBOX2.MSGID.all = 0xC0000003;
    ECanaMboxes.MBOX3.MSGID.all = 0xC0000004;
    ECanaMboxes.MBOX4.MSGID.all = 0xC0000005;
    ECanaMboxes.MBOX5.MSGID.all = 0xC0000006;
    ECanaMboxes.MBOX6.MSGID.all = 0xC0000007;
    ECanaMboxes.MBOX7.MSGID.all = 0xC0000008;
    ECanaMboxes.MBOX8.MSGID.all = 0xC0000009;
    ECanaMboxes.MBOX9.MSGID.all = 0xC000000A;
    ECanaMboxes.MBOX10.MSGID.all = 0xC000000B;
    ECanaMboxes.MBOX11.MSGID.all = 0xC000000C;
    ECanaMboxes.MBOX12.MSGID.all = 0xC000000D;
    ECanaMboxes.MBOX13.MSGID.all = 0xC000000E;
    ECanaMboxes.MBOX14.MSGID.all = 0xC000000F;
    ECanaMboxes.MBOX15.MSGID.all = 0xC0000010;

    // Write to the MSGID field of RECEIVE mailboxes MBOX16 - 31
    ECanaMboxes.MBOX16.MSGID.all = 0xC1000001;
    ECanaMboxes.MBOX17.MSGID.all = 0xC1000002;
    ECanaMboxes.MBOX18.MSGID.all = 0xC1000003;
    ECanaMboxes.MBOX19.MSGID.all = 0xC1000004;
    ECanaMboxes.MBOX20.MSGID.all = 0xC1000005;
    ECanaMboxes.MBOX21.MSGID.all = 0xC1000006;
    ECanaMboxes.MBOX22.MSGID.all = 0xC1000007;
    ECanaMboxes.MBOX23.MSGID.all = 0xC1000008;
    ECanaMboxes.MBOX24.MSGID.all = 0xC1000009;
    ECanaMboxes.MBOX25.MSGID.all = 0xC100000A;
    ECanaMboxes.MBOX26.MSGID.all = 0xC100000B;
    ECanaMboxes.MBOX27.MSGID.all = 0xC100000C;
    ECanaMboxes.MBOX28.MSGID.all = 0xC100000D;
    ECanaMboxes.MBOX29.MSGID.all = 0xC100000E;
    ECanaMboxes.MBOX30.MSGID.all = 0xC100000F;
    ECanaMboxes.MBOX31.MSGID.all = 0xC1000010;

        // Configure Mailboxes 0-15 as Tx, 16-31 as Rx
    // Since this write is to the entire register (instead of a bit
    // field) a shadow register is not required.
    ECanaRegs.CANMD.all = 0xFFFF0000;

    // Enable all Mailboxes
    // Since this write is to the entire register (instead of a bit
    // field) a shadow register is not required.
    ECanaRegs.CANME.all = 0xFFFFFFFF;

    // Specify that 8 bits will be sent
    ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX2.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX3.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX4.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX5.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX6.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX7.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX8.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX9.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX10.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX11.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX12.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX13.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX14.MSGCTRL.bit.DLC = 8;
    ECanaMboxes.MBOX15.MSGCTRL.bit.DLC = 8;

    // Write to the mailbox RAM field of MBOX0 - 15
    ECanaMboxes.MBOX0.MDL.all = 0x00000001;
    ECanaMboxes.MBOX0.MDH.all = 0x00000001;

    ECanaMboxes.MBOX1.MDL.all = 0x00000002;
    ECanaMboxes.MBOX1.MDH.all = 0x00000002;

    ECanaMboxes.MBOX2.MDL.all = 0x00000003;
    ECanaMboxes.MBOX2.MDH.all = 0x00000003;

    ECanaMboxes.MBOX3.MDL.all = 0x00000000;
    ECanaMboxes.MBOX3.MDH.all = 0x00000000;

    ECanaMboxes.MBOX4.MDL.all = 0x00000000;
    ECanaMboxes.MBOX4.MDH.all = 0x00000000;

    ECanaMboxes.MBOX5.MDL.all = 0x00000000;
    ECanaMboxes.MBOX5.MDH.all = 0x00000000;

    ECanaMboxes.MBOX6.MDL.all = 0x00000000;
    ECanaMboxes.MBOX6.MDH.all = 0x00000000;

    ECanaMboxes.MBOX7.MDL.all = 0x00000000;
    ECanaMboxes.MBOX7.MDH.all = 0x00000000;

    ECanaMboxes.MBOX8.MDL.all = 0x00000000;
    ECanaMboxes.MBOX8.MDH.all = 0x00000000;

    ECanaMboxes.MBOX9.MDL.all = 0x00000000;
    ECanaMboxes.MBOX9.MDH.all = 0x00000000;

    ECanaMboxes.MBOX10.MDL.all = 0x00000000;
    ECanaMboxes.MBOX10.MDH.all = 0x00000000;

    ECanaMboxes.MBOX11.MDL.all = 0x00000000;
    ECanaMboxes.MBOX11.MDH.all = 0x00000000;

    ECanaMboxes.MBOX12.MDL.all = 0x00000000;
    ECanaMboxes.MBOX12.MDH.all = 0x00000000;

    ECanaMboxes.MBOX13.MDL.all = 0x00000000;
    ECanaMboxes.MBOX13.MDH.all = 0x00000000;

    ECanaMboxes.MBOX14.MDL.all = 0x00000000;
    ECanaMboxes.MBOX14.MDH.all = 0x00000000;

    ECanaMboxes.MBOX15.MDL.all = 0x00000000;
    ECanaMboxes.MBOX15.MDH.all = 0x00000000;

 */
