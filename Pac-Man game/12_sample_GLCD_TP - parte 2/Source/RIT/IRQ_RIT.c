/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "LPC17xx.h"
#include "RIT.h"
#include "../GLCD/GLCD.h"
//#include "../led/led.h"
#include "../timer/timer.h"

/******************************************************************************
come gestisco il joistick: 
se premo su un tasto va avanti senza fermarmi
mettere controlli muro e cambio direzione
dentro al RIT modifico la matrice, poi ho un timer2 con una frequenza molto bassa e lì dentro stampo sul display
******************************************************************************/

volatile int down=0;
extern int PacmanX;
extern int PacmanY;
extern int mat[31][28];
extern int score;
int J_up=0; //configurazione di utilizzo joistick, statica perchè voglio incrementarla ogni interruzione
int J_down = 0;
int J_left = 0;
int J_right = 0;
volatile int ButtonPressed = 0 ;



void RIT_IRQHandler (void)
{	
	static int bottonPressed = 0;
	

	/*BOTTON SELECT
	static int select=0;
	if((LPC_GPIO1->FIOPIN & (1<<25) == 0){	
	select ++;
	switch(select){
	case 1:  //qui non ho problema del bouncing (dove eseguivo la seconda volta) e posso eseguire subito
					//la prima volta che premo, vado a eseguire l'azione. Tutto questo per garantire che qnd riladcio il 
		qui scrivo l'azione
		break;
	default:
	break;} }
	else{ select = 0}  se il piedino non è attivo, select = 0 e riparto da capo a contare */
	
	
	/*BOTTON UP
	Joytick UP pressed. Loco di programma dove controllo se il piedino=0. 
		Leggo la porta 1, and logica con bit 29 che se è premuto è =0, se and loica =0 tasto premuto
	Allora incremento variabile del piedino	*/
	
	if((LPC_GPIO1->FIOPIN & (1<<29)) == 0){
			bottonPressed ++; 
		switch (bottonPressed) {
			case 1: 
				J_up = 1; 
				J_down = 0;
				J_left = 0;
				J_right = 0;
			case 2: 
			default: 
				bottonPressed = 0;
				break; }}
	
	/*BOTTON DOWN*/
	if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){	
			bottonPressed ++; 
		switch (bottonPressed) {
			case 1: 
				J_up = 0; 
				J_down = 1;
				J_left = 0;
				J_right = 0;
			default: 
				bottonPressed = 0;
				break; }}
	
	/*  BOTTON LEFT   */
	if((LPC_GPIO1->FIOPIN & (1<<27)) == 0){	
			bottonPressed ++; 
		switch (bottonPressed) {
			case 1: 
				J_up = 0; 
				J_down = 0;
				J_left = 1;
				J_right = 0;
			default: 
				bottonPressed = 0;
				break; }}
	
	/*  BOTTON RIGHT   */
	if((LPC_GPIO1->FIOPIN & (1<<28)) == 0){	
			bottonPressed ++; 
		switch (bottonPressed) {
			case 1: 
				J_up = 0; 
				J_down = 0;
				J_left = 0;
				J_right = 1;
			default: 
				bottonPressed = 0;
				break; }}
	
	
	
	
	/* button management */
	if(down != 0){ 
		if((LPC_GPIO2->FIOPIN & (1<<10)) == 0){	/* INT0 pressed */
			down++;
			switch(down){	
				case 2:
					switch(ButtonPressed){
						case 0: //controllo se è la prima volta che premo int0, metto in pausa
							ButtonPressed++;
							disable_timer(0); //fermo il pacman
							disable_timer(1); //così dovrebbe fermarsi il countdown salvando in countdown la variabile per decrementare
							int riga = 12; 
							int colonna = 10;
							uint8_t Pausa[] = "Pausa";
							GUI_Text(90, 154, Pausa, Darkgray, White);
							break;
						case 1:  //ripremo pausa
							enable_timer(0); //riattivo i timer
							enable_timer(1);
							ButtonPressed = 0;
							LCD_DrawFilledRectangle(90, 154, 60, 20, White); //pulisco
						default:
							break; } }
					
		} else {
					down = 0; /* button released */	
					NVIC_EnableIRQ(EINT0_IRQn);							 /* enable Button interrupts			*/
					LPC_PINCON->PINSEL4    |= (1 << 20);     /* External interrupt 0 pin selection */
				}
			}
				
	
/*	else{
			if(down==1)
				down++;
	} */
	
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
