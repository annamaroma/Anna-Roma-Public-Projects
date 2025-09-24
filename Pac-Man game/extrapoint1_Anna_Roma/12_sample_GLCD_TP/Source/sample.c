/*******************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions: qui inizializzo il problema, faccio partire tutti i timer e chiamo le funzioni per disegnare il labirinto e 
per visualizzare le informazioni sul display

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
	LCD_Clear(White);
	srand(1234);
  //imposto le scritte
	TopInitialize();
	DownInitialize(Vite);
	//disegno il labirinto
	DrawMaze(startX, startY, wallColor, foodColor, powerFoodColor, pacmanColor, backgroundColor);
	

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
*********************************************************************************************************/
