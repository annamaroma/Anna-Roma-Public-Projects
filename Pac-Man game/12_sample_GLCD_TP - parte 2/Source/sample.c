/**************************************************************************

**--------------File Info---------------------------------------------------------------------------------
** File name:  sample.c
** Description: In questo file è contenuto il main che contiene le funzioni per far partire il gioco
      
**
**----	TO DO LIST	----------------------------------------------------------------------------------------------------
fatti da controllare:
- INT0 Pause the game: Place a “PAUSE” message in the middle of the screen and stop PacMan movement. Pressing again INT0 resume the game (delete the Pause message and resume
movement). The game starts in “PAUSE” mode: PROBLEMA è che non riparte quando premo
- timer per cibo grande, è sbagliato, sto implementando la funzione che posiziona un numero a caso nella matrice ma non è corretta

da fare:

-  Configure the speaker to play sound effects and background music using the speaker.

- Use the CAN peripheral to print the current Score of the game, the Remaining Lives and the Countdown timer through the CAN bus
You must adequately configure the CAN controllers of the LandTiger Boards. It must be configured to be
in what is called external “loopback” mode. This means that the board will communicate the message
back to itself! To achieve this goal, you will use CAN1 and CAN2, one set to receive the message (CAN2)
and one set to send the message (CAN1). Since these are two different channels, they don’t actually know
that they are communicating with the same board! So by all means, you are communicating through CAN
with another board.
Remaining time 		Remaining lives 		Score
8 bits 						8 bits 							16 bits
Every message must be saved in a 32-bit unsigned int variable

- Create an AI-controlled ghost that pursue Pac-Man, causing him to lose a life in case of contact.

*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include "TouchPanel/TouchPanel.h"
#include "timer/timer.h"



#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif


int startX = 20;     // Posizione iniziale del labirinto in X
int startY = 70;     // Posizione iniziale del labirinto in Y
unsigned short wallColor =  0x0210; // DARKBLUE per i muri
unsigned short foodColor = 0xCC68;  //BROWN per il cibo piccolo
unsigned short powerFoodColor = 0x5268; //DARKGRAY per il cibo grande
unsigned short pacmanColor = 0x9008; // PURPLE per il pacman
unsigned short backgroundColor = White;  // Colore dello sfondo
extern int score;
int Vite = 3;


	

int main(void)
{
  SystemInit();  												/* System Initialization (i.e., PLL)  */
	joystick_init();											/* Joystick Initialization            */
	init_RIT(0x004C4B40);									/* RIT Initialization 50 msec       	*/
	enable_RIT();													/* RIT enabled												*/
	BUTTON_init();												/* BUTTON Initialization              */
  LCD_Initialization();
	
  TP_Init();
	//TouchPanel_Calibrate();
	//LCD_Clear(White);
	
	
	//disegno il labirinto
	DrawMaze(startX, startY, wallColor, foodColor, powerFoodColor, pacmanColor, backgroundColor);
	//imposto le scritte
	TopInitialize();
	DownInitialize(Vite);

	//GUI_Text(0, 280, (uint8_t *) " touch here : 1 sec to clear  ", Red, White);
	//init_timer(0, 0x1312D0 ); 						/* 50ms * 25MHz = 1.25*10^6 = 0x1312D0 */
	//init_timer(1, 0x6108 ); 						  /* 1ms * 25MHz = 25*10^3 = 0x6108 */
	//init_timer(0, 0x4E2 ); 						    /* 500us * 25MHz = 1.25*10^3 = 0x4E2 */
	LPC_SC -> PCONP |= (1<<22) ; //ACCENDE TIMER 2
	LPC_SC -> PCONP |= (1<<23) ; //accende timer 3
	//count = 1[s]*25*10^6 [1/s] = 0x17D7840 
	//SRI=011=3   TIMER 1
	init_timer(1,25000000);
	//init_timer(1,0,0,3,0x017D7840);	
	enable_timer(1);   // per far partire il timer gameover
	init_timer(0, 0x5F5E10); //timer per la velocità del pacman  12500000
	enable_timer(0);
	
	PrintNumber(140, 40, score, Purple, White);
	
	LPC_SC->PCON |= 0x1;									/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);						
	
  while (1)	
  {
		__ASM("wfi");
  }
}
/*********************************************************************************************************
      END FILE
void TIMER1_IRQHandler(void) {
    static int countdown = 60; // Variabile statica per mantenere il valore tra chiamate

    // Controlla se il countdown è ancora in corso
    if (countdown > 0) {
				PrintNumber(10, 10, countdown, Purple, White);
        countdown--; // Decrementa il countdown
    } else {
	      GUI_Text(10, 10, (uint8_t *) "GAME OVER", Green, White);
        disable_timer(1); // Ferma il timer
    }
		
		// Resetta il flag di interruzione per TIMER1
    LPC_TIM1->IR = 1; 
}
*********************************************************************************************************/
