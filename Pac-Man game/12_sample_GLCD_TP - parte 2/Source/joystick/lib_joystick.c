/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           joystick.h
** Last modified Date:  2018-12-30
** Last Version:        V1.00
** Descriptions:        Atomic joystick init functions
** Correlated files:    lib_joystick.c, funct_joystick.c
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "LPC17xx.h"
#include "joystick.h"

/*----------------------------------------------------------------------------
  Function that initializes joysticks and switch them off
	
	P1.25 select
	P1.26	down
	P1.27	left
	P1.28	right
	P1.29	up
 *----------------------------------------------------------------------------*/

void joystick_init(void) {
	
	/* joystick DOWN functionality */
	LPC_PINCON->PINSEL3 &= ~(3<<20);  //PIN mode GPIO (00b value per P1.26)
	LPC_GPIO1->FIODIR   &= ~(1<<26);	//P1.26 Input (joysticks on PORT1 defined as Input)
	
	/* joystick LEFT functionality */
	LPC_PINCON->PINSEL3 &= ~(3<<22);  //PIN mode GPIO (00b value per P1.27)
	LPC_GPIO1->FIODIR   &= ~(1<<27);	//P1.26 Input (joysticks on PORT1 defined as Input)
	
	/* joystick RIGHT functionality */
	LPC_PINCON->PINSEL3 &= ~(3<<24);  //PIN mode GPIO (00b value per P1.28)
	LPC_GPIO1->FIODIR   &= ~(1<<28);	//P1.26 Input (joysticks on PORT1 defined as Input)
	
	/* joistick UP functionality*/
  LPC_PINCON->PINSEL3 &= ~(3<<26);	//azzero il valore PIN mode GPIO (00b value per P1.29)   
	LPC_GPIO1->FIODIR   &= ~(1<<29);	//P1.29 Input (joysticks on PORT1 defined as Input) gestisco il registro della direzione per la porta 1
	//stabilisco che il piedino 29 deve essere = 0, cioè per dire che premo il tasto up
}
