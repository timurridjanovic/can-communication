/*
 * INGENIARTS
 * Initial release : 2013-09-21
 * created by Rami Jarjour and Timur Ridjanovic
 * Last revision : 2013-09-21
 */


#include "DSP281x_Device.h"     // DSP281x Headerfile Include File
#include "DSP281x_Examples.h"   // DSP281x Examples Include File
#include "functions.h"   // DSP281x Examples Include File

// Global variables for this example

void initMBoxes(void) {
	// eCAN control registers require read/write access using 32-bits.  Thus we
	// will create a set of shadow registers for this example.  These shadow
	// registers will be used to make sure the access is 32-bits and not 16.

	// MB0 == TRANSMITTER
	// MB1 == RECEIVER

	struct ECAN_REGS ECanaShadow;

	// Mailboxs can be written to 16-bits or 32-bits at a time
	// Write to the MSGID field of TRANSMIT mailboxes MBOX0


	// Write to the MSGID field of TRANSMIT mailboxes --> MBOX0
	ECanaMboxes.MBOX0.MSGID.all = 0x91000001; // This is exactly the same adress as the Master receiving mailbox address

	// Write to the MSGID field of RECEIVE mailboxes MBOX1
	ECanaMboxes.MBOX1.MSGID.all = 0xC0000003;// NB - MASTER ADRESS IS : ECanaMboxes.MBOX0.MSGID.all = 0xC0000013;
		//	ECanaMboxes.MBOX1.MSGID.bit.IDE = 1; // Extended Identifier
		//	ECanaMboxes.MBOX1.MSGID.bit.AME = 1; // acceptance mask set to 1
		//	ECanaMboxes.MBOX1.MSGID.bit.AAM = 0;
	ECanaLAMRegs.LAM1.all = 0xF0; // bits set to 1 means we don't care, bits set to 0 means they must match
	ECanaLAMRegs.LAM1.bit.LAMI = 1;

// Configure MB0 as transmitter and MB1 as receiver
	ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
	ECanaShadow.CANMD.bit.MD0 = 0; // 0 The corresponding mailbox is configured as a transmit mailbox.
	ECanaShadow.CANMD.bit.MD1 = 1; // 1 The corresponding mailbox is configured as a receive mailbox.
	ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;

	// Enable Mailboxes 0 and 1 a shadow register is required.
	ECanaShadow.CANME.all = ECanaRegs.CANME.all;
	ECanaShadow.CANME.bit.ME0 = 1;
	ECanaShadow.CANME.bit.ME1 = 1;
	ECanaRegs.CANME.all = ECanaShadow.CANME.all;

	// Specify that 8 bits will be sent
	ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = 8;
	ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = 8;

	// Configure the eCAN for self test mode
	// Enable the enhanced features of the eCAN.
//	EALLOW;
//	ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
//	ECanaShadow.CANMC.bit.STM = 1;    // Configure CAN for self-test mode
//	ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;
//	EDIS;


}



void sendMessage(Uint32 messageLow, Uint32 messageHigh) {

	struct ECAN_REGS ECanaShadow;
    ErrorCount = 0;
    MessageLow = messageLow;
    MessageHigh = messageHigh;

    // Write to the mailbox RAM field of MBOX0 - 15
    ECanaMboxes.MBOX0.MDL.all = messageLow;
    ECanaMboxes.MBOX0.MDH.all = messageHigh;


    // Begin transmitting
    ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
    ECanaShadow.CANTRS.bit.TRS0 = 1;
    ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;
    MessageToBeSent = 0;

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
	for (i=0; i<200000000; i++) {}
}


void initEcanInterrupt() {
	struct ECAN_REGS ECanaShadow;

	EALLOW;

	// set CANGIM to 1 to connect to the I0EN line; check eCAN spru074F interrupt scheme p.76
	ECanaShadow.CANGIM.all = ECanaRegs.CANGIM.all;
	ECanaShadow.CANGIM.bit.I0EN = 1;
	ECanaRegs.CANGIM.all = ECanaShadow.CANGIM.all;


	// set CANMIL to 0 to connect to the I0EN line to MIL0 & MIL1
	ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
	ECanaShadow.CANMIL.bit.MIL0 = 0;
	ECanaShadow.CANMIL.bit.MIL1 = 0;
	ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;

	// set CANMIM to 1 to enable the interrupt on MIM0 & MIM1
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


interrupt void eCAN0INT_ISR(void) {

	struct ECAN_REGS ECanaShadow;
	ECanaShadow.CANRMP.all = ECanaRegs.CANRMP.all; //to be able to check RPM1 status

	// Receive procedure
	if (ECanaShadow.CANRMP.bit.RMP1 == 1)
	{
		ECanaShadow.CANRMP.all = 0;
		ECanaShadow.CANRMP.bit.RMP1 = 1;
		ECanaRegs.CANRMP.all = ECanaShadow.CANRMP.all; //clear RMP1

		mailbox_read(1);

		if (GpioDataRegs.GPFDAT.bit.GPIOF14 == 1) {
			GpioDataRegs.GPFCLEAR.bit.GPIOF14 = 1;
		}
		else {
			GpioDataRegs.GPFSET.bit.GPIOF14 = 1;
		}

		MessageToBeSent = 1; //Message as to be sent

	}

	else if(ECanaShadow.CANTA.bit.TA0 == 1)
	{
		ECanaShadow.CANTA.all = 0;
		ECanaShadow.CANTA.bit.TA0 = 1;
		ECanaRegs.CANTA.all = ECanaShadow.CANTA.all; //clear RMP1
		MessageToBeSent = 0; //Message has been sent
	}
	else //error
	{
		ErrorCount++;
	}

	PieCtrlRegs.PIEACK.bit.ACK9 = 1; //Set to 1 to clear the bit (inversed logic)

}
