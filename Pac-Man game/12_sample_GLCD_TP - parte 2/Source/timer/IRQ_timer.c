/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include <string.h>
#include "LPC17xx.h"
#include "timer.h"
#include "../GLCD/GLCD.h" 
#include "../TouchPanel/TouchPanel.h"
#include <stdio.h> /*for sprintf*/

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
extern int PacmanX;
extern int PacmanY;
extern int mat[31][28];
extern int score;
extern int numeri[6];
int x=0;
int y=0;
extern int J_up;
extern int J_down;
extern int J_left;
extern int J_right;
extern int Vite;
static int contatore = 0;
int CiboDaMangiare = 0; 
extern int posizioni[6][2];
int conteggioCiboGrande = 0;

void TIMER0_IRQHandler (void)
{
	if (J_up == 1){
		//coordinate pacman  
		x = PacmanY*7+20;
		y = PacmanX*7+70;
	
		// Controllo se la posizione sopra è un muro
				if (mat[PacmanX-1][PacmanY] == 1) {
						// Se è un muro, Pacman non si muove
						LCD_DrawFilledCircle(x+3, y+3, 3, Purple);  
				} else {
					
						// Se non è un muro, gestisco il movimento di Pacman
						// Disegno lo sfondo dove era Pacman prima
						LCD_DrawFilledRectangle(x, y, 7, 7, White);  // Disegna il muro
						mat[PacmanX][PacmanY] = 2;  // Aggiorno la matrice (posizione precedente vuota)
						
						PacmanX = PacmanX - 1; // Aggiorna la posizione del Pacman
						y = PacmanX*7+70;
						
						// Controllo il contenuto della nuova cella
						if (mat[PacmanX][PacmanY] == 0) {
								// Se è cibo piccolo, incremento lo score di 10
								score += 10;
								CiboDaMangiare++ ;
							
						} else if (mat[PacmanX][PacmanY] == 3) {
								// Se è cibo grande, incremento lo score di 30
								score += 30;
								CiboDaMangiare++;
						}
						// Sposto Pacman nella nuova posizione
						PrintNumber(140, 40, score, Purple, White);
						LCD_DrawFilledCircle(x+3, y+3, 3, Purple);   // Disegna Pacman nella nuova posizione
						mat[PacmanX][PacmanY] = 4;  // Aggiorno la matrice con la posizione di Pacman
				}
	} else if (J_down == 1)
		{
			//coordinate pacman  
				x = PacmanY*7+20;
				y = PacmanX*7+70;
		// Controllo se la posizione sopra è un muro
				if (mat[PacmanX+1][PacmanY] == 1) {
						// Se è un muro, Pacman non si muove
						LCD_DrawFilledCircle(x+3, y+3, 3, Purple); 
					
				} else {
					
						// Se non è un muro, gestisco il movimento di Pacman
						// Disegno lo sfondo dove era Pacman prima
						LCD_DrawFilledRectangle(x, y, 7, 7, White);  
						mat[PacmanX][PacmanY] = 2;  // Aggiorno la matrice (posizione precedente vuota)
						PacmanX += 1; // Aggiorna la posizione del Pacman
						//coordinate pacman  
						y = PacmanX*7+70;

						// Controllo il contenuto della nuova cella
						if (mat[PacmanX][PacmanY] == 0) {
								// Se è cibo piccolo, incremento lo score di 10
								score += 10;
								CiboDaMangiare++ ;
						} else if (mat[PacmanX][PacmanY] == 3) {
								// Se è cibo grande, incremento lo score di 30
								score += 30;
								CiboDaMangiare++ ;
							
						}
						// Sposto Pacman nella nuova posizione
						PrintNumber(140, 40, score, Purple, White);
						LCD_DrawFilledCircle(x+3, y+3, 3, Purple);  // Disegna Pacman nella nuova posizione
						mat[PacmanX][PacmanY] = 4;  // Aggiorno la matrice con la posizione di Pacman 
			}
				
	}	else if (J_left == 1) 
		{//coordinate pacman  
				x = PacmanY*7+20;
				y = PacmanX*7+70;
			
			//controllo tunnel
				if (PacmanX == 14 && PacmanY == 0 ) {
					PacmanY = 27; } 
				
				if (mat[PacmanX][PacmanY-1] == 1) {
						LCD_DrawFilledCircle(x+3, y+3, 3, Purple);  
				} else {
						LCD_DrawFilledRectangle(x, y, 7, 7, White);  // Disegna il muro
						mat[PacmanX][PacmanY] = 2;  // Aggiorno la matrice (posizione precedente vuota)
						PacmanY = PacmanY - 1; // Aggiorna la posizione del Pacman
						
						x = PacmanY*7+20;

						// Controllo il contenuto della nuova cella
						if (mat[PacmanX][PacmanY] == 0) {
								// Se è cibo piccolo, incremento lo score di 10
								score += 10;
								CiboDaMangiare++ ;
						} else if (mat[PacmanX][PacmanY] == 3) {
								// Se è cibo grande, incremento lo score di 30
								score += 50;
								CiboDaMangiare++ ;
								}
								PrintNumber(140, 40, score, Purple, White);
								// Sposto Pacman nella nuova posizione
								LCD_DrawFilledCircle(x+3, y+3, 3, Purple);   // Disegna Pacman nella nuova posizione
								mat[PacmanX][PacmanY] = 4;  // Aggiorno la matrice con la posizione di Pacman
			} 
				
	} else if (J_right == 1) 
		{
			//coordinate pacman  
				x = PacmanY*7+20;
				y = PacmanX*7+70;
		
				if (PacmanX == 14 && PacmanY == 27 ) {
					PacmanY = 0; } 
				if (mat[PacmanX][PacmanY+1] == 1) {
						// Se è un muro, Pacman non si muove
						LCD_DrawFilledCircle(x+3, y+3, 3, Purple);  
				} else {
						
						LCD_DrawFilledRectangle(x, y, 7, 7, White);  // Disegna il muro
						mat[PacmanX][PacmanY] = 2; 
					
						PacmanY ++;  
						
						x = PacmanY*7+20;
						
						// Controllo il contenuto della nuova cella
						if (mat[PacmanX][PacmanY] == 0) {
								// Se è cibo piccolo, incremento lo score di 10
								score += 10;
								CiboDaMangiare++ ;
								
						} else if (mat[PacmanX][PacmanY] == 3) {
								// Se è cibo grande, incremento lo score di 30
								score += 30;
								CiboDaMangiare++ ;
								
						}
						PrintNumber(140, 40, score, Purple, White);
						mat[PacmanX][PacmanY] = 4; 
						LCD_DrawFilledCircle(x+3, y+3, 3, Purple);  
						
			}
	}
		
	
		if (score == 1000 || score == 1500 || score == 2000) {
				contatore ++;
				switch(contatore) {
					case 1: 
						LCD_DrawFilledRectangle(140, 40, 7, 7, White); //pulisco
						uint8_t NewLife[] = "Nuova vita !!!";
						GUI_Text(140, 40, NewLife, Darkgray, White);
						LCD_DrawFilledCircle(x+3, y+3, 3, White); //cancello il pacman attuale
						Vite = Vite - 1;
						if (Vite == 0) {
							uint8_t EndLifes[] = "Vite terminate !!!";
							GUI_Text(80, 60, EndLifes, Darkgray, White); 
							disable_timer(1);
							disable_timer(0);  //fermo il gioco
						} else {
							DownInitialize(Vite);
							PacmanX = 14;
							PacmanY = 3;
							x = PacmanY*7+20;
							y = PacmanX*7+70;
							LCD_DrawFilledCircle(x+3, y+3, 3, Purple); //riscrivo il nuovo pacman
						} 
						break;
						
					case 10: 
						LCD_DrawFilledRectangle(140, 40, 60, 20, White); //pulisco
						PrintNumber(140, 40, score, Purple, White);
						break;
				}
		}
		
		if (CiboDaMangiare == 240){
			//ho terminato i cibi 
			disable_timer(1);
			disable_timer(0);  //fermo il gioco
			LCD_DrawFilledRectangle(160, 65, 7, 7, White);
			uint8_t HaiVinto[] = "Hai vinto !!!";
			GUI_Text(160, 65, HaiVinto, Purple, White);
		}
			
			
	
  LPC_TIM0->IR = 1;			/* clear interrupt flag */
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler(void) {
    static int countdown = 100; // Variabile statica per mantenere il valore tra chiamate
		uint8_t gameover[] = "GAME OVER";
		int i;
	 
			
    // Controlla se il countdown è ancora in corso
    if (countdown > 0) {
				// Controlla se countdown corrisponde a uno dei numeri nel vettore
				for (i = 0; i < 6; i++) {
					if (countdown == numeri[i]) {
						numeri[i]=0;
						int riga = posizioni[conteggioCiboGrande][0];
						int colonna = posizioni[conteggioCiboGrande][1];
						conteggioCiboGrande++;
						LCD_DrawFilledCircle(riga + 3, colonna + 3, 1, 0x5268); // Cibo grande Darkgray 
						mat[riga][colonna] = 3; 
					} }
				PrintNumber(30, 40, countdown, Purple, White);
        countdown--; // Decrementa il countdown
    } else {
	      GUI_Text(30, 40, gameover, Darkgray, White);
        disable_timer(1); // Ferma il countdown
				disable_timer(0); // Ferma pacman
    }
		
		// Resetta il flag di interruzione per TIMER1
    LPC_TIM1->IR = 1; 
}

/******************************************************************************
**                            End Of File
******************************************************************************/
