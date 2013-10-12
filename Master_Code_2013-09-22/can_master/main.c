/* INGENIARTS
 * Initial release: 2013-09-21
 * Created by Timur Ridjanovic and Rami Jarjour
 * Last revision: 2013-09-21 */


#include "DSP281x_Device.h"     // DSP281x Headerfile Include File
#include "DSP281x_Examples.h"   // DSP281x Examples Include File

#define BusVoltageBit 9
#define BatteryTempBit 7
#define SOCBit 7
#define BigRollML (BusVoltageBit + BatteryTempBit + SOCBit)
#define MediumRollML (BatteryTempBit + SOCBit)
#define SmallRollML SOCBit

#define ADC_usDELAY  8000L  // -- "L" means "long double"
#define ADC_usDELAY2 20L	// -- "L" means "long double"
#define PRECISIONADC 10		// is used in the for loop of the ADC

// Prototype statements for functions found within this file.
interrupt void eCAN0INT_ISR(void);
void initEcanInterrupt(void);
void sendMessage(Uint32 messageLow, Uint32 messageHigh);
void createMessage(void);
void initMBoxes(void);
void mailbox_read(int16 MBXnbr);
void delay_loop(void);
void Init_ADC(void);
void TOGGLE_PIN(void);
int get_ADC(void);
void decodeMessage(void);


// Global variables for this example
Uint32  ErrorCount = 0;
Uint16 MessageReceived = 0;
Uint32  TestMbox1 = 0;
Uint32  TestMbox2 = 0;
Uint32  TestMbox3 = 0;

Uint32 BatteryVoltage = 0;
Uint32 BusVoltage = 0;
Uint32 BatteryTemp = 0;
Uint32 SOC = 0;
Uint32 MDLTempR = 0;
Uint32 MDHTempR = 0;
Uint32 SlaveFaultState = 0;

Uint32 SlaveBatteryPowerONAuth = 0; // 1 bit
Uint32 MotorOrGenerator = 0; // 1 bit
Uint32 BatteryCurrentCharge = 0; // 6 bits
Uint32 BatteryCurrentDischarge = 0; // 7 bits
Uint32 MachinePowerTransfer = 0; // 7 bits
Uint32 MotorFrequency = 0; // 10 bits
Uint32 MDLTempS = 0;
Uint32 MDHTempS = 0;
Uint32 RotorPosition = 0; // 9 bits
Uint32 PhaseAngle = 0; // 5 bits

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
	   Init_ADC();

	   InitECan();

	   initEcanInterrupt();

	   initMBoxes(); //for Master

	   while(1) {
		   createMessage();
		   sendMessage(MDLTempS, MDHTempS);
		   if (MessageReceived == 1) {
			   decodeMessage();

		   }

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

void createMessage(void) {
	SlaveBatteryPowerONAuth = 1;
	MotorOrGenerator = 1;
	BatteryCurrentCharge = 32;
	BatteryCurrentDischarge = 60;
	MachinePowerTransfer = 20;
	MotorFrequency = 600;

	//create MDLTempS
	MDLTempS = (MotorFrequency << 22) + (MachinePowerTransfer << 15) + (BatteryCurrentDischarge << 8) + (MotorOrGenerator << 1) + SlaveBatteryPowerONAuth;

	RotorPosition = 255;
	PhaseAngle = 25;

	MDHTempS = (RotorPosition << 5) + PhaseAngle;

}


void sendMessage(Uint32 messageLow, Uint32 messageHigh) {

	struct ECAN_REGS ECanaShadow;

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

	if (ECanaShadow.CANTA.bit.TA0 == 1) {
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

	else if (ECanaShadow.CANRMP.bit.RMP1 == 1) {
		ECanaShadow.CANRMP.all = 0;
		ECanaShadow.CANRMP.bit.RMP1 = 1;
		ECanaRegs.CANRMP.all = ECanaShadow.CANRMP.all; //clear RMP

		mailbox_read(1);
		MDLTempR = TestMbox1;
		MDHTempR = TestMbox2;
		MessageReceived = 1;
	}

	else {
		ErrorCount++;
	}

	PieCtrlRegs.PIEACK.bit.ACK9 = 1; // clear aknowledge bit

}

void decodeMessage(void) {
	MessageReceived = 0;

	//decode MDLTempR
	BatteryVoltage = MDLTempR >> BigRollML;
	BusVoltage = (MDLTempR - (BatteryVoltage << BigRollML)) >> MediumRollML;
	BatteryTemp = (MDLTempR - (BatteryVoltage << BigRollML) - (BusVoltage << MediumRollML)) >> SmallRollML;
	SOC = MDLTempR - (BatteryVoltage << BigRollML) - (BusVoltage << MediumRollML) - (BatteryTemp << SmallRollML);

	//decode MDHTempR
	SlaveFaultState = MDHTempR;

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


void Init_ADC(void){
	/*
	1.6 Power-up Sequence
	The ADC resets to the ADC off state. When powering up the ADC, use the
	following sequence:
	1) If external reference is desired, enable this mode using bit 8 in the ADCCTRL3 Register. This mode must be enabled before band gap is
	powered. This avoids internal reference signals (ADCREFP and ADCREFM) driving external reference sources if present o the board.
	2) Power up the reference and bandgap circuits for at least 7 ms before powering up the rest of the ADC analog circuitry.
	3) After the ADC has been fully powered up, an additional delay of at least 20 ms is required before performing the first ADC conversion.
	When powering down the ADC, all three bits can be cleared simultaneously. The ADC power level must be controlled via software and they are
	independent of the state of the device power modes.
	Note: Follow Power-up Sequence
	For reliability and accuracy, the power-up sequence must be followed precisely.
	See the most recent data sheet for timing data.
	 */

// Configures ADCTRL1 to user-desired value
	AdcRegs.ADCTRL1.bit.RESET = 0x1; // Resets the ADC (RESET = 1)
    asm(" RPT #10 ||NOP");
	AdcRegs.ADCTRL1.bit.RESET = 0x0;	// RESET set to 0

	AdcRegs.ADCTRL1.bit.SUSMOD = 0x2; //MODE 2
	AdcRegs.ADCTRL1.bit.ACQ_PS = 1;
	AdcRegs.ADCTRL1.bit.CPS = 0;	//CPS = 0 ==> ADCCLK = Fclk /1
	AdcRegs.ADCTRL1.bit.CONT_RUN = 1;	// Start-stop mode. Sequencer stops after reaching EOS. On the next SOC, the
										// sequencer starts from the state where it ended unless a sequencer reset is
										// performed

	AdcRegs.ADCTRL1.bit.SEQ_OVRD = 1;	// 0 Disabled, Allows the sequencer to wrap around at the end of conversions
										// set by MAX CONVn.
	AdcRegs.ADCTRL1.bit.SEQ_CASC = 1;	// 1: Cascaded mode. SEQ1 and SEQ2 operate as a single 16-state sequencer
										// (SEQ).

// Configures ADCTRL2 to user-desired value
	AdcRegs.ADCTRL2.bit.EVB_SOC_SEQ = 0; // No action
	AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;// 1 - Immediately reset sequencer to state CONV00
	AdcRegs.ADCTRL2.bit.SOC_SEQ1 = 0; /* 0 Clears a pending SOC trigger.
									  * Note: If the sequencer has already started, this bit is automatically cleared,
									  * and hence, writing a zero has no effect; i.e., an already started sequencer
									  * cannot be stopped by clearing this bit.
									  */

	AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = 1; // 1 - Interrupt request by INT SEQ1 is enabled.
	AdcRegs.ADCTRL2.bit.INT_MOD_SEQ1 = 0; //0 - INT SEQ1 is set at the end of every SEQ1 sequence
	AdcRegs.ADCTRL2.bit.EVA_SOC_SEQ1 = 1;// Event manager A SOC mask for SEQ1
										 // 1 Allows SEQ1/SEQ to be started by Event Manager A trigger. The Event
										 // Manager can be programmed to start a conversion on various events.

	AdcRegs.ADCTRL2.bit.EXT_SOC_SEQ1 = 0;// External start of conversion for SEQ1 - 0 = No action
	/*
	 ONLY IN DUAL MODE, IGNORED IN CASCADE MODE
	 * AdcRegs.ADCTRL2.bit.RST_SEQ2 = ;       // Reset SEQ2
	 * AdcRegs.ADCTRL2.bit.SOC_SEQ2 = ;       // Start of conversion for SEQ2
	 * AdcRegs.ADCTRL2.bit.INT_ENA_SEQ2 = ; //     SEQ2 Interrupt enable
	 * AdcRegs.ADCTRL2.bit.INT_MOD_SEQ2 = ;   // 2     SEQ2 Interrupt mode
	 * AdcRegs.ADCTRL2.bit.EVB_SOC_SEQ2 = ;   // Event manager B SOC mask for SEQ2
	 */


// 	ADCCHSELSEQ1;  // Channel select sequencing control 1
	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0x0;	// Selecting ADCINA0
	   AdcRegs.ADCCHSELSEQ1.all = 0x0;         // Initialize all ADC channel selects to A0
	   AdcRegs.ADCCHSELSEQ2.all = 0x0;
	   AdcRegs.ADCCHSELSEQ3.all = 0x0;
	   AdcRegs.ADCCHSELSEQ4.all = 0x0;
	   AdcRegs.ADCMAXCONV.bit.MAX_CONV1 = 0x7;  // convert and store in 8 results registers


// Start SEQ1
      AdcRegs.ADCTRL2.all = 0x2000;
// Configures ADCTRL3 to user-desired value

	AdcRegs.ADCTRL3.bit.ADCEXTREF = 0;     // ADC external reference - 0 = internal reference
	AdcRegs.ADCTRL3.bit.ADCBGRFDN = 0x3; // 7:6 ADC bandgap/ref power down - Power up bandgap/reference circuits
	DELAY_US(ADC_usDELAY); // Delay before powering up ADC for 8ms -- L means "long double"
	AdcRegs.ADCTRL3.bit.ADCCLKPS = 3;      // 4:1    ADC core clock divider, // HSPCLK = SYSCLKOUT/6 = 25MHz , ADCCLKPS = 5 ==> 25MHZ / 10 = 2.5MHZ
	AdcRegs.ADCTRL3.bit.SMODE_SEL = 0;     // 0      Sampling mode select - 0 Sequential sampling mode is selected.
	AdcRegs.ADCTRL3.bit.ADCPWDN = 1;       // 5      1 : The analog circuitry inside the core is powered up. - ADC powerdown, 1 = Power up rest of ADC
	DELAY_US(ADC_usDELAY2); // Delay after powering up ADC - an additional delay of at least 20 us is required before performing the first ADC conversion

}

void TOGGLE_PIN(void)
{
	GpioDataRegs.GPATOGGLE.bit.GPIOA0 = 1;
	GpioDataRegs.GPATOGGLE.bit.GPIOA1 = 1;
}

int get_ADC(void)
{
	//Declare local variable
	long int comparatifA0 = 0;
	long int comparatifMoyenne = 0;
	int i = 0 ;

	for (i=0 ; i<PRECISIONADC ; i++)
	{
		while (AdcRegs.ADCST.bit.INT_SEQ1== 0){}
		asm(" setc XF ");                 // Set XF for monitoring  -optional
		AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;
		comparatifA0 += AdcRegs.ADCRESULT0 >> 4;
	}

	comparatifMoyenne = comparatifA0 / PRECISIONADC;

	return comparatifMoyenne;
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
