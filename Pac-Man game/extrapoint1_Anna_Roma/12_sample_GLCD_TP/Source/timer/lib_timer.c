/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           lib_timer.h
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        atomic functions to be used by higher sw levels
** Correlated files:    lib_timer.c, funct_timer.c, IRQ_timer.c
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "LPC17xx.h"
#include "timer.h"

/******************************************************************************
** Function name:		enable_timer
**
** Descriptions:		Enable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void enable_timer( uint8_t timer_num )
{
  if ( timer_num == 0 )
  {
	LPC_TIM0->TCR = 1;
  }
  else if (timer_num == 1)
  {
	LPC_TIM1->TCR = 1;
  }
	else if (timer_num == 2)
  {
	LPC_TIM2->TCR = 1;
  }
	else 
  {
	LPC_TIM3->TCR = 1;
  }
  return;
}

/******************************************************************************
** Function name:		disable_timer
**
** Descriptions:		Disable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void disable_timer( uint8_t timer_num )
{
  if ( timer_num == 0 )
  {
	LPC_TIM0->TCR = 0;
  }
  else if ( timer_num == 1)
  {
	LPC_TIM1->TCR = 0;
  }
	else if ( timer_num == 2)
  {
	LPC_TIM2->TCR = 0;
  }
	else 
  {
	LPC_TIM3->TCR = 0;
  }
  return;
}

/******************************************************************************
** Function name:		reset_timer
**
** Descriptions:		Reset timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void reset_timer( uint8_t timer_num )  //? il secondo bit, il reset non deve toccare il primo bit
{
  uint32_t regVal;

  if ( timer_num == 0 )
  {
	regVal = LPC_TIM0->TCR;  //salvo 
	regVal |= 0x02;  //bitwise con 2, cioè? 10 in binario
	LPC_TIM0->TCR = regVal;  //sovrascrive il valore del registro
  }
  else if (timer_num == 1)
  {
	regVal = LPC_TIM1->TCR;
	regVal |= 0x02;
	LPC_TIM1->TCR = regVal;
  }
	  else if (timer_num == 2)
  {
	regVal = LPC_TIM2->TCR;
	regVal |= 0x02;
	LPC_TIM2->TCR = regVal;
  }
	  else 
  {
	regVal = LPC_TIM3->TCR;
	regVal |= 0x02;
	LPC_TIM3->TCR = regVal;
  }
  return;
}


//qui posso leggere timer_num e impostare il timer
//configurazione del match register, il timerIntervsl
uint32_t init_timer ( uint8_t timer_num, uint32_t TimerInterval )
{
  if ( timer_num == 0 )
  {
	LPC_TIM0->MR0 = TimerInterval;
	LPC_TIM0->MCR = 3;

	NVIC_EnableIRQ(TIMER0_IRQn);
	/*NVIC_SetPriority(TIMER0_IRQn, 4);*/		/* less priority than buttons */
	NVIC_SetPriority(TIMER0_IRQn, 2);		/* more priority than buttons */
	return (1);
  }
  else if ( timer_num == 1 )
  {
	LPC_TIM1->MR0 = TimerInterval;
	LPC_TIM1->MCR = 3;				

	NVIC_EnableIRQ(TIMER1_IRQn);
	NVIC_SetPriority(TIMER1_IRQn, 0);	/* less priority than buttons and timer0*/
	return (1);
  }
  return (0);
}

/*
uint32_t init_timer ( uint8_t timer_num, uint32_t Prescaler, uint8_t MatchRegister, uint8_t SRImatchReg, uint32_t TimerInterval )
{
  if ( timer_num == 0 )
  {
		LPC_TIM0 -> PR = Prescaler;
		if (MatchRegister == 0){
			LPC_TIM0 -> MR0 = TimerInterval;
			LPC_TIM0 -> MCR |= SRImatchReg << 3*MatchRegister ; //metti l'or | per non cancellare gli altri
		}
		else if (MatchRegister == 1){
			LPC_TIM1 -> MR0 = TimerInterval;
			LPC_TIM1 -> MCR |= SRImatchReg << 3*MatchRegister;
		}
		else if (MatchRegister == 2){
			LPC_TIM2 -> MR0 = TimerInterval;
			LPC_TIM2 -> MCR |= SRImatchReg << 3*MatchRegister ;
		}
		else if (MatchRegister == 3){
			LPC_TIM3 -> MR0 = TimerInterval;
			LPC_TIM3 -> MCR |= SRImatchReg << 3*MatchRegister ;
		}
		
	
	LPC_TIM0->MR0 = TimerInterval;
 
	LPC_TIM0->MCR = 3;

	NVIC_EnableIRQ(TIMER0_IRQn);
	//NVIC_SetPriority(TIMER0_IRQn, 4); less priority than buttons 
	NVIC_SetPriority(TIMER0_IRQn, 0);		more priority than buttons 
	return (1);
  }
  else if ( timer_num == 1 )
  {
		LPC_TIM1 -> PR = Prescaler;
		if (MatchRegister == 0){
			LPC_TIM1 -> MR0 = TimerInterval;
			LPC_TIM1 -> MCR |= SRImatchReg << 3*MatchRegister ; //metti l'or | per non cancellare gli altri
		}
		else if (MatchRegister == 1){
			LPC_TIM1 -> MR0 = TimerInterval;
			LPC_TIM1 -> MCR |= SRImatchReg << 3*MatchRegister;
		}
		else if (MatchRegister == 2){
			LPC_TIM2 -> MR0 = TimerInterval;
			LPC_TIM2 -> MCR |= SRImatchReg << 3*MatchRegister ;
		}
		else if (MatchRegister == 3){
			LPC_TIM3 -> MR0 = TimerInterval;
			LPC_TIM3 -> MCR |= SRImatchReg << 3*MatchRegister ;
		}
	NVIC_EnableIRQ(TIMER1_IRQn);
	NVIC_SetPriority(TIMER1_IRQn, 5);	 less priority than buttons and timer0
	return (1);
  }
	else if ( timer_num == 2 )
  {
		LPC_TIM2 -> PR = Prescaler;
		if (MatchRegister == 0){
			LPC_TIM0 -> MR0 = TimerInterval;
			LPC_TIM0 -> MCR |= SRImatchReg << 3*MatchRegister ; //metti l'or | per non cancellare gli altri
		}
		else if (MatchRegister == 1){
			LPC_TIM1 -> MR0 = TimerInterval;
			LPC_TIM1 -> MCR |= SRImatchReg << 3*MatchRegister;
		}
		else if (MatchRegister == 2){
			LPC_TIM2 -> MR0 = TimerInterval;
			LPC_TIM2 -> MCR |= SRImatchReg << 3*MatchRegister ;
		}
		else if (MatchRegister == 3){
			LPC_TIM3 -> MR0 = TimerInterval;
			LPC_TIM3 -> MCR |= SRImatchReg << 3*MatchRegister ;
		}
	NVIC_EnableIRQ(TIMER1_IRQn);
	NVIC_SetPriority(TIMER1_IRQn, 5);	 less priority than buttons and timer0
	return (1);
  }
	else if ( timer_num == 3 )
  {
		LPC_TIM3 -> PR = Prescaler;
		if (MatchRegister == 0){
			LPC_TIM0 -> MR0 = TimerInterval;
			LPC_TIM0 -> MCR |= SRImatchReg << 3*MatchRegister ; //metti l'or | per non cancellare gli altri
		}
		else if (MatchRegister == 1){
			LPC_TIM1 -> MR0 = TimerInterval;
			LPC_TIM1 -> MCR |= SRImatchReg << 3*MatchRegister;
		}
		else if (MatchRegister == 2){
			LPC_TIM2 -> MR0 = TimerInterval;
			LPC_TIM2 -> MCR |= SRImatchReg << 3*MatchRegister ;
		}
		else if (MatchRegister == 3){
			LPC_TIM3 -> MR0 = TimerInterval;
			LPC_TIM3 -> MCR |= SRImatchReg << 3*MatchRegister ;
		}
	NVIC_EnableIRQ(TIMER1_IRQn);
	NVIC_SetPriority(TIMER1_IRQn, 5);	 less priority than buttons and timer0
	return (1);
  }
  return (0);
}
*/
/******************************************************************************
**                            End Of File
******************************************************************************/
