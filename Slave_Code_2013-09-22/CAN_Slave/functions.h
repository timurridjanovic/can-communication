/*
 * functions.h
 *
 *  Created on: 2013-09-21
 *      Author: Rami
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

// global variables shared with others files
extern Uint32 ErrorCount;
extern Uint32 TestMbox1;
extern Uint32 TestMbox2;
extern Uint32 MessageHigh;
extern Uint32 MessageLow;
extern Uint32 MessageToBeSent;
extern Uint32 TestMbox3;

//fuctions shared with main()
extern interrupt void eCAN0INT_ISR(void);


void initEcanInterrupt(void);
void sendMessage(Uint32 messageLow, Uint32 messageHigh);
void initMBoxes(void);
void mailbox_read(int16 MBXnbr);
void delay_loop(void);


#endif /* FUNCTIONS_H_ */
