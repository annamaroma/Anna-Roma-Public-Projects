/****************************************Copyright (c)**************************************************                         
**
**                                 http://www.powermcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			GLCD.c
** Descriptions:		Has been tested SSD1289¡¢ILI9320¡¢R61505U¡¢SSD1298¡¢ST7781¡¢SPFD5408B¡¢ILI9325¡¢ILI9328¡¢
**						HX8346A¡¢HX8347A
**------------------------------------------------------------------------------------------------------
** 
********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "GLCD.h" 
#include "AsciiLib.h"


/* variables ---------------------------------------------------------*/
static uint8_t LCD_Code;  //private
int PacmanX = 0;				
int PacmanY = 0;
int mat[31][28] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1},
    {1, 2, 2, 2, 2, 1, 0, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 0, 1, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 1, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 1, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 1, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 1, 2, 2, 2, 2, 1},
    {1, 1, 1, 1, 1, 1, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 1, 1, 1, 1, 1, 1},
    {2, 2, 2, 4, 2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2},
    {1, 1, 1, 1, 1, 1, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 1, 1, 1, 1, 1, 1},
    {1, 2, 2, 2, 2, 1, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 1, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 1, 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 1, 2, 2, 2, 2, 1},
    {1, 2, 2, 2, 2, 1, 0, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 0, 1, 2, 2, 2, 2, 1},
    {1, 1, 1, 1, 1, 1, 0, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 0, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1},
    {1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1},
    {1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};
int score = 0;
extern int count;
/*
1: Muro
2: Corridoio vuoto
0: Cibo piccolo
3: Cibo grande (power food)
4: Pacman 
*/
/* Private define ------------------------------------------------------------*/
#define  ILI9320    0  /* 0x9320 */
#define  ILI9325    1  /* 0x9325 */
#define  ILI9328    2  /* 0x9328 */
#define  ILI9331    3  /* 0x9331 */
#define  SSD1298    4  /* 0x8999 */
#define  SSD1289    5  /* 0x8989 */
#define  ST7781     6  /* 0x7783 */
#define  LGDP4531   7  /* 0x4531 */
#define  SPFD5408B  8  /* 0x5408 */
#define  R61505U    9  /* 0x1505 0x0505 */
#define  HX8346A		10 /* 0x0046 */  
#define  HX8347D    11 /* 0x0047 */
#define  HX8347A    12 /* 0x0047 */	
#define  LGDP4535   13 /* 0x4535 */  
#define  SSD2119    14 /* 3.5 LCD 0x9919 */

/*******************************************************************************
* Function Name  : Lcd_Configuration
* Description    : Configures LCD Control lines
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_Configuration(void)
{
	/* Configure the LCD Control pins */
	
	/* EN = P0.19 , LE = P0.20 , DIR = P0.21 , CS = P0.22 , RS = P0.23 , RS = P0.23 */
	/* RS = P0.23 , WR = P0.24 , RD = P0.25 , DB[0.7] = P2.0...P2.7 , DB[8.15]= P2.0...P2.7 */  
	LPC_GPIO0->FIODIR   |= 0x03f80000;
	LPC_GPIO0->FIOSET    = 0x03f80000;
}

/*******************************************************************************
* Function Name  : LCD_Send
* Description    : LCDÐ´Êý¾Ý
* Input          : - byte: byte to be sent
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_Send (uint16_t byte) 
{
	LPC_GPIO2->FIODIR |= 0xFF;          /* P2.0...P2.7 Output */
	LCD_DIR(1)		   				    				/* Interface A->B */
	LCD_EN(0)	                        	/* Enable 2A->2B */
	LPC_GPIO2->FIOPIN =  byte;          /* Write D0..D7 */
	LCD_LE(1)                         
	LCD_LE(0)														/* latch D0..D7	*/
	LPC_GPIO2->FIOPIN =  byte >> 8;     /* Write D8..D15 */
}

/*******************************************************************************
* Function Name  : wait_delay
* Description    : Delay Time
* Input          : - nCount: Delay Time
* Output         : None
* Return         : None
* Return         : None
* Attention		 : None 
*******************************************************************************/
static void wait_delay(int count)
{
	while(count--);
}

/*******************************************************************************
* Function Name  : LCD_Read
* Description    : LCD¶ÁÊý¾Ý
* Input          : - byte: byte to be read
* Output         : None
* Return         : ·µ»Ø¶ÁÈ¡µ½µÄÊý¾Ý
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_Read (void) 
{
	uint16_t value;
	
	LPC_GPIO2->FIODIR &= ~(0xFF);              /* P2.0...P2.7 Input */
	LCD_DIR(0);		   				           				 /* Interface B->A */
	LCD_EN(0);	                               /* Enable 2B->2A */
	wait_delay(30);							   						 /* delay some times */
	value = LPC_GPIO2->FIOPIN0;                /* Read D8..D15 */
	LCD_EN(1);	                               /* Enable 1B->1A */
	wait_delay(30);							   						 /* delay some times */
	value = (value << 8) | LPC_GPIO2->FIOPIN0; /* Read D0..D7 */
	LCD_DIR(1);
	return  value;
}

/*******************************************************************************
* Function Name  : LCD_WriteIndex
* Description    : LCDÐ´¼Ä´æÆ÷µØÖ·
* Input          : - index: ¼Ä´æÆ÷µØÖ·
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteIndex(uint16_t index)
{
	LCD_CS(0);
	LCD_RS(0);
	LCD_RD(1);
	LCD_Send( index ); 
	wait_delay(22);	
	LCD_WR(0);  
	wait_delay(1);
	LCD_WR(1);
	LCD_CS(1);
}

/*******************************************************************************
* Function Name  : LCD_WriteData
* Description    : LCDÐ´¼Ä´æÆ÷Êý¾Ý
* Input          : - index: ¼Ä´æÆ÷Êý¾Ý
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteData(uint16_t data)
{				
	LCD_CS(0);
	LCD_RS(1);   
	LCD_Send( data );
	LCD_WR(0);     
	wait_delay(1);
	LCD_WR(1);
	LCD_CS(1);
}

/*******************************************************************************
* Function Name  : LCD_ReadData
* Description    : ¶ÁÈ¡¿ØÖÆÆ÷Êý¾Ý
* Input          : None
* Output         : None
* Return         : ·µ»Ø¶ÁÈ¡µ½µÄÊý¾Ý
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_ReadData(void)
{ 
	uint16_t value;
	
	LCD_CS(0);
	LCD_RS(1);
	LCD_WR(1);
	LCD_RD(0);
	value = LCD_Read();
	
	LCD_RD(1);
	LCD_CS(1);
	
	return value;
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Writes to the selected LCD register.
* Input          : - LCD_Reg: address of the selected register.
*                  - LCD_RegValue: value to write to the selected register.
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{ 
	/* Write 16-bit Index, then Write Reg */  
	LCD_WriteIndex(LCD_Reg);         
	/* Write 16-bit Reg */
	LCD_WriteData(LCD_RegValue);  
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Reads the selected LCD Register.
* Input          : None
* Output         : None
* Return         : LCD Register Value.
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
	uint16_t LCD_RAM;
	
	/* Write 16-bit Index (then Read Reg) */
	LCD_WriteIndex(LCD_Reg);
	/* Read 16-bit Reg */
	LCD_RAM = LCD_ReadData();      	
	return LCD_RAM;
}

/*******************************************************************************
* Function Name  : LCD_SetCursor
* Description    : Sets the cursor position.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position. 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_SetCursor(uint16_t Xpos,uint16_t Ypos)
{
    #if  ( DISP_ORIENTATION == 90 ) || ( DISP_ORIENTATION == 270 )
	
 	uint16_t temp = Xpos;

			 Xpos = Ypos;
			 Ypos = ( MAX_X - 1 ) - temp;  

	#elif  ( DISP_ORIENTATION == 0 ) || ( DISP_ORIENTATION == 180 )
		
	#endif

  switch( LCD_Code )
  {
     default:		 /* 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x7783 0x4531 0x4535 */
          LCD_WriteReg(0x0020, Xpos );     
          LCD_WriteReg(0x0021, Ypos );     
	      break; 

     case SSD1298: 	 /* 0x8999 */
     case SSD1289:   /* 0x8989 */
	      LCD_WriteReg(0x004e, Xpos );      
          LCD_WriteReg(0x004f, Ypos );          
	      break;  

     case HX8346A: 	 /* 0x0046 */
     case HX8347A: 	 /* 0x0047 */
     case HX8347D: 	 /* 0x0047 */
	      LCD_WriteReg(0x02, Xpos>>8 );                                                  
	      LCD_WriteReg(0x03, Xpos );  

	      LCD_WriteReg(0x06, Ypos>>8 );                           
	      LCD_WriteReg(0x07, Ypos );    
	
	      break;     
     case SSD2119:	 /* 3.5 LCD 0x9919 */
	      break; 
  }
}

/*******************************************************************************
* Function Name  : LCD_Delay
* Description    : Delay Time
* Input          : - nCount: Delay Time
* Output         : None
* Return         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void delay_ms(uint16_t ms)    
{ 
	uint16_t i,j; 
	for( i = 0; i < ms; i++ )
	{ 
		for( j = 0; j < 1141; j++ );
	}
} 


/*******************************************************************************
* Function Name  : LCD_Initializtion
* Description    : Initialize TFT Controller.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Initialization(void)
{
	uint16_t DeviceCode;
	
	LCD_Configuration();
	delay_ms(100);
	DeviceCode = LCD_ReadReg(0x0000);		/* ¶ÁÈ¡ÆÁID	*/	
	
	if( DeviceCode == 0x9325 || DeviceCode == 0x9328 )	
	{
		LCD_Code = ILI9325;
		LCD_WriteReg(0x00e7,0x0010);      
		LCD_WriteReg(0x0000,0x0001);  	/* start internal osc */
		LCD_WriteReg(0x0001,0x0100);     
		LCD_WriteReg(0x0002,0x0700); 	/* power on sequence */
		LCD_WriteReg(0x0003,(1<<12)|(1<<5)|(1<<4)|(0<<3) ); 	/* importance */
		LCD_WriteReg(0x0004,0x0000);                                   
		LCD_WriteReg(0x0008,0x0207);	           
		LCD_WriteReg(0x0009,0x0000);         
		LCD_WriteReg(0x000a,0x0000); 	/* display setting */        
		LCD_WriteReg(0x000c,0x0001);	/* display setting */        
		LCD_WriteReg(0x000d,0x0000); 			        
		LCD_WriteReg(0x000f,0x0000);
		/* Power On sequence */
		LCD_WriteReg(0x0010,0x0000);   
		LCD_WriteReg(0x0011,0x0007);
		LCD_WriteReg(0x0012,0x0000);                                                                 
		LCD_WriteReg(0x0013,0x0000);                 
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0010,0x1590);   
		LCD_WriteReg(0x0011,0x0227);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0012,0x009c);                  
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0013,0x1900);   
		LCD_WriteReg(0x0029,0x0023);
		LCD_WriteReg(0x002b,0x000e);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0020,0x0000);                                                            
		LCD_WriteReg(0x0021,0x0000);           
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0030,0x0007); 
		LCD_WriteReg(0x0031,0x0707);   
		LCD_WriteReg(0x0032,0x0006);
		LCD_WriteReg(0x0035,0x0704);
		LCD_WriteReg(0x0036,0x1f04); 
		LCD_WriteReg(0x0037,0x0004);
		LCD_WriteReg(0x0038,0x0000);        
		LCD_WriteReg(0x0039,0x0706);     
		LCD_WriteReg(0x003c,0x0701);
		LCD_WriteReg(0x003d,0x000f);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0050,0x0000);        
		LCD_WriteReg(0x0051,0x00ef);   
		LCD_WriteReg(0x0052,0x0000);     
		LCD_WriteReg(0x0053,0x013f);
		LCD_WriteReg(0x0060,0xa700);        
		LCD_WriteReg(0x0061,0x0001); 
		LCD_WriteReg(0x006a,0x0000);
		LCD_WriteReg(0x0080,0x0000);
		LCD_WriteReg(0x0081,0x0000);
		LCD_WriteReg(0x0082,0x0000);
		LCD_WriteReg(0x0083,0x0000);
		LCD_WriteReg(0x0084,0x0000);
		LCD_WriteReg(0x0085,0x0000);
		  
		LCD_WriteReg(0x0090,0x0010);     
		LCD_WriteReg(0x0092,0x0000);  
		LCD_WriteReg(0x0093,0x0003);
		LCD_WriteReg(0x0095,0x0110);
		LCD_WriteReg(0x0097,0x0000);        
		LCD_WriteReg(0x0098,0x0000);  
		/* display on sequence */    
		LCD_WriteReg(0x0007,0x0133);
		
		LCD_WriteReg(0x0020,0x0000);  /* ÐÐÊ×Ö·0 */                                                          
		LCD_WriteReg(0x0021,0x0000);  /* ÁÐÊ×Ö·0 */     
	}

    delay_ms(50);   /* delay 50 ms */	
}

/*******************************************************************************
* Function Name  : LCD_Clear
* Description    : ½«ÆÁÄ»Ìî³ä³ÉÖ¸¶¨µÄÑÕÉ«£¬ÈçÇåÆÁ£¬ÔòÌî³ä 0xffff
* Input          : - Color: Screen Color
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Clear(uint16_t Color)
{
	uint32_t index;
	
	if( LCD_Code == HX8347D || LCD_Code == HX8347A )
	{
		LCD_WriteReg(0x02,0x00);                                                  
		LCD_WriteReg(0x03,0x00);  
		                
		LCD_WriteReg(0x04,0x00);                           
		LCD_WriteReg(0x05,0xEF);  
		                 
		LCD_WriteReg(0x06,0x00);                           
		LCD_WriteReg(0x07,0x00);    
		               
		LCD_WriteReg(0x08,0x01);                           
		LCD_WriteReg(0x09,0x3F);     
	}
	else
	{	
		LCD_SetCursor(0,0); 
	}	

	LCD_WriteIndex(0x0022);
	for( index = 0; index < MAX_X * MAX_Y; index++ )
	{
		LCD_WriteData(Color);
	}
}

/******************************************************************************
* Function Name  : LCD_BGR2RGB
* Description    : RRRRRGGGGGGBBBBB ¸ÄÎª BBBBBGGGGGGRRRRR ¸ñÊ½
* Input          : - color: BRG ÑÕÉ«Öµ  
* Output         : None
* Return         : RGB ÑÕÉ«Öµ
* Attention		 : ÄÚ²¿º¯Êýµ÷ÓÃ
*******************************************************************************/
static uint16_t LCD_BGR2RGB(uint16_t color)
{
	uint16_t  r, g, b, rgb;
	
	b = ( color>>0 )  & 0x1f;
	g = ( color>>5 )  & 0x3f;
	r = ( color>>11 ) & 0x1f;
	
	rgb =  (b<<11) + (g<<5) + (r<<0);
	
	return( rgb );
}

/******************************************************************************
* Function Name  : LCD_GetPoint
* Description    : »ñÈ¡Ö¸¶¨×ù±êµÄÑÕÉ«Öµ
* Input          : - Xpos: Row Coordinate
*                  - Xpos: Line Coordinate 
* Output         : None
* Return         : Screen Color
* Attention		 : None
*******************************************************************************/
uint16_t LCD_GetPoint(uint16_t Xpos,uint16_t Ypos)
{
	uint16_t dummy;
	
	LCD_SetCursor(Xpos,Ypos);
	LCD_WriteIndex(0x0022);  
	
	switch( LCD_Code )
	{
		case ST7781:
		case LGDP4531:
		case LGDP4535:
		case SSD1289:
		case SSD1298:
             dummy = LCD_ReadData();   /* Empty read */
             dummy = LCD_ReadData(); 	
 		     return  dummy;	      
	    case HX8347A:
	    case HX8347D:
             {
		        uint8_t red,green,blue;
				
				dummy = LCD_ReadData();   /* Empty read */

		        red = LCD_ReadData() >> 3; 
                green = LCD_ReadData() >> 2; 
                blue = LCD_ReadData() >> 3; 
                dummy = (uint16_t) ( ( red<<11 ) | ( green << 5 ) | blue ); 
		     }	
	         return  dummy;

        default:	/* 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x9919 */
             dummy = LCD_ReadData();   /* Empty read */
             dummy = LCD_ReadData(); 	
 		     return  LCD_BGR2RGB( dummy );
	}
}

/******************************************************************************
* Function Name  : LCD_SetPoint
* Description    : ÔÚÖ¸¶¨×ù±ê»­µã
* Input          : - Xpos: Row Coordinate
*                  - Ypos: Line Coordinate 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_SetPoint(uint16_t Xpos,uint16_t Ypos,uint16_t point)
{
	if( Xpos >= MAX_X || Ypos >= MAX_Y )
	{
		return;
	}
	LCD_SetCursor(Xpos,Ypos);
	LCD_WriteReg(0x0022,point);
}

/******************************************************************************
* Function Name  : LCD_DrawLine
* Description    : Bresenham's line algorithm
* Input          : - x1: AµãÐÐ×ù±ê
*                  - y1: AµãÁÐ×ù±ê 
*				   - x2: BµãÐÐ×ù±ê
*				   - y2: BµãÁÐ×ù±ê 
*				   - color: ÏßÑÕÉ«
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/	 
void LCD_DrawLine( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color )
{
    short dx,dy;      /* ¶¨ÒåX YÖáÉÏÔö¼ÓµÄ±äÁ¿Öµ */
    short temp;       /* Æðµã ÖÕµã´óÐ¡±È½Ï ½»»»Êý¾ÝÊ±µÄÖÐ¼ä±äÁ¿ */

    if( x0 > x1 )     /* XÖáÉÏÆðµã´óÓÚÖÕµã ½»»»Êý¾Ý */
    {
	    temp = x1;
		x1 = x0;
		x0 = temp;   
    }
    if( y0 > y1 )     /* YÖáÉÏÆðµã´óÓÚÖÕµã ½»»»Êý¾Ý */
    {
		temp = y1;
		y1 = y0;
		y0 = temp;   
    }
  
	dx = x1-x0;       /* XÖá·½ÏòÉÏµÄÔöÁ¿ */
	dy = y1-y0;       /* YÖá·½ÏòÉÏµÄÔöÁ¿ */

    if( dx == 0 )     /* XÖáÉÏÃ»ÓÐÔöÁ¿ »­´¹Ö±Ïß */ 
    {
        do
        { 
            LCD_SetPoint(x0, y0, color);   /* ÖðµãÏÔÊ¾ Ãè´¹Ö±Ïß */
            y0++;
        }
        while( y1 >= y0 ); 
		return; 
    }
    if( dy == 0 )     /* YÖáÉÏÃ»ÓÐÔöÁ¿ »­Ë®Æ½Ö±Ïß */ 
    {
        do
        {
            LCD_SetPoint(x0, y0, color);   /* ÖðµãÏÔÊ¾ ÃèË®Æ½Ïß */
            x0++;
        }
        while( x1 >= x0 ); 
		return;
    }
	/* ²¼À¼É­ººÄ·(Bresenham)Ëã·¨»­Ïß */
    if( dx > dy )                         /* ¿¿½üXÖá */
    {
	    temp = 2 * dy - dx;               /* ¼ÆËãÏÂ¸öµãµÄÎ»ÖÃ */         
        while( x0 != x1 )
        {
	        LCD_SetPoint(x0,y0,color);    /* »­Æðµã */ 
	        x0++;                         /* XÖáÉÏ¼Ó1 */
	        if( temp > 0 )                /* ÅÐ¶ÏÏÂÏÂ¸öµãµÄÎ»ÖÃ */
	        {
	            y0++;                     /* ÎªÓÒÉÏÏàÁÚµã£¬¼´£¨x0+1,y0+1£© */ 
	            temp += 2 * dy - 2 * dx; 
	 	    }
            else         
            {
			    temp += 2 * dy;           /* ÅÐ¶ÏÏÂÏÂ¸öµãµÄÎ»ÖÃ */  
			}       
        }
        LCD_SetPoint(x0,y0,color);
    }  
    else
    {
	    temp = 2 * dx - dy;                      /* ¿¿½üYÖá */       
        while( y0 != y1 )
        {
	 	    LCD_SetPoint(x0,y0,color);     
            y0++;                 
            if( temp > 0 )           
            {
                x0++;               
                temp+=2*dy-2*dx; 
            }
            else
			{
                temp += 2 * dy;
			}
        } 
        LCD_SetPoint(x0,y0,color);
	}
} 

/******************************************************************************
* Function Name  : PutChar
* Description    : ½«LcdÆÁÉÏÈÎÒâÎ»ÖÃÏÔÊ¾Ò»¸ö×Ö·û
* Input          : - Xpos: Ë®Æ½×ø±ê 
*                  - Ypos: ´¹Ö±×ø±ê  
*				   - ASCI: ÏÔÊ¾µÄ×Ö·û
*				   - charColor: ×Ö·ûÑÕÉ«   
*				   - bkColor: ±³¾°ÑÕÉ« 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void PutChar( uint16_t Xpos, uint16_t Ypos, uint8_t ASCI, uint16_t charColor, uint16_t bkColor )
{
	uint16_t i, j;
    uint8_t buffer[16], tmp_char;
    GetASCIICode(buffer,ASCI);  /* È¡×ÖÄ£Êý¾Ý */
    for( i=0; i<16; i++ )
    {
        tmp_char = buffer[i];
        for( j=0; j<8; j++ )
        {
            if( ((tmp_char >> (7 - j)) & 0x01) == 0x01 )
            {
                LCD_SetPoint( Xpos + j, Ypos + i, charColor );  /* ×Ö·ûÑÕÉ« */
            }
            else
            {
                LCD_SetPoint( Xpos + j, Ypos + i, bkColor );  /* ±³¾°ÑÕÉ« */
            }
        }
    }
}

void PrintNumber(uint16_t Xpos, uint16_t Ypos, int number, uint16_t charColor, uint16_t bkColor) {
    char buffer[12]; // Buffer per contenere il numero convertito in stringa (fino a 10 cifre più segno e terminatore)
    int i = 0;

    // Converte il numero in stringa (supporta anche numeri negativi)
    sprintf(buffer, "%d", number);

    // Cicla sui caratteri della stringa e li stampa uno alla volta
    while (buffer[i] != '\0') {
        PutChar(Xpos, Ypos, buffer[i], charColor, bkColor); // Stampa il carattere
        Xpos += 8; // Avanza la posizione orizzontale (supponendo che ogni carattere sia largo 8 pixel)
        i++;
    }
}


/******************************************************************************
* Function Name  : GUI_Text
* Description    : ÔÚÖ¸¶¨×ù±êÏÔÊ¾×Ö·û´®
* Input          : - Xpos: ÐÐ×ù±ê
*                  - Ypos: ÁÐ×ù±ê 
*				   - str: ×Ö·û´®
*				   - charColor: ×Ö·ûÑÕÉ«   
*				   - bkColor: ±³¾°ÑÕÉ« 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void GUI_Text(uint16_t Xpos, uint16_t Ypos, uint8_t *str,uint16_t Color, uint16_t bkColor)
{
    uint8_t TempChar;
    do
    {
        TempChar = *str++;  
        PutChar( Xpos, Ypos, TempChar, Color, bkColor );    
        if( Xpos < MAX_X - 8 )
        {
            Xpos += 8;
        } 
        else if ( Ypos < MAX_Y - 16 )
        {
            Xpos = 0;
            Ypos += 16;
        }   
        else
        {
            Xpos = 0;
            Ypos = 0;
        }    
    }
    while ( *str != 0 );
}

void TopInitialize(void){
	GUI_Text(20, 10, (uint8_t *) "Game over in", Purple, White);
	GUI_Text(130, 10, (uint8_t *) "Score", Purple, White);
}

void LCD_DrawFilledCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t color) {
    int x = 0;
    int y = radius;
    int decision = 1 - radius; // Valore iniziale del criterio di decisione

    while (y >= x) {
        // Riempie le righe verticali per ogni ottante
        LCD_DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, color); // Ottante 1 e 2
        LCD_DrawLine(x0 - x, y0 - y, x0 + x, y0 - y, color); // Ottante 3 e 4
        LCD_DrawLine(x0 - y, y0 + x, x0 + y, y0 + x, color); // Ottante 5 e 6
        LCD_DrawLine(x0 - y, y0 - x, x0 + y, y0 - x, color); // Ottante 7 e 8

        x++; // Incrementa x per muoverti sul perimetro del cerchio
        if (decision <= 0) {
            // Decidi se restare sullo stesso y o decrementarlo
            decision += 2 * x + 1;
        } else {
            y--;
            decision += 2 * (x - y) + 1;
        }
    }
}


void LCD_DrawFilledRectangle(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height, uint16_t color) {
		static int i = 0;
		static int j = 0;
    for (i=0; i < height; i++) {
        for (j = 0; j < width; j++) {
            // Disegna un punto per ogni pixel del rettangolo
            LCD_SetPoint(x0 + j, y0 + i, color);
        }
    }
}


void DrawMaze(int startX, int startY, unsigned short wallColor, unsigned short foodColor, unsigned short powerFoodColor, unsigned short pacmanColor, unsigned short backgroundColor) {
	
		
		int numeri[6];
		int posizioniCiboGrande[6][2];
		estraiNumeriCasuali(numeri, 6, 58);
		estraiPosizioniCasuali(mat, posizioniCiboGrande, 240);
	
    /* // Step 1: Genera 6 numeri casuali tra 0 e 240
    int NumeriCasuali[6];
    srand(1234); // Inizializza il generatore di numeri casuali
		static int i = 0;
		static int j = 0;
    for (i = 0; i < 6; i++) {
        int numero;
        int isDuplicate;
        do {
            numero = rand() % 240; // Estrai un numero tra 0 e 240
            isDuplicate = 0;
            // Controlla se è duplicato
            for (j = 0; j < i; j++) {
                if (NumeriCasuali[j] == numero) {
                    isDuplicate = 1;
                    break; }}
        } while (isDuplicate); // Ripeti finché non trovi un numero unico
        NumeriCasuali[i] = numero; } */ 

    // Step 2: Inizializza il contatore per i cibi piccoli
    int countdown = 0;

    // Step 3: Itera su tutta la matrice
		static int a = 0;
		static int b = 0;
		static int k = 0;
    for (a = 0; a < 31; a++) {
        for (b = 0; b < 28; b++) {
            int x = startX + b * 7; // Calcola la posizione X
            int y = startY + a * 7; // Calcola la posizione Y

            if (mat[a][b] == 1) {  // Muro
                LCD_DrawFilledRectangle(x, y, 7, 7, wallColor);  // Disegna il muro
            } 
            else if (mat[a][b] == 2) {  // Corridoio vuoto
                LCD_SetPoint(x, y, backgroundColor); // Disegna lo sfondo
            } 
            else if (mat[a][b] == 0) {   // Cibo piccolo o grande       
                    LCD_DrawFilledCircle(x + 3, y + 3, 1, foodColor); // Cibo piccolo 
										//cibo grande è stampato nel timer1 con posizione e a intervalli casuali
						} 
						else if (mat[a][b] == 4) {   // Pacman
								LCD_DrawFilledCircle(x + 3, y + 3, 3, pacmanColor);  // Disegna Pacman
								PacmanX = a;
								PacmanY = b;
            }
        }
    }
}

void DownInitialize(uint16_t Vite){
	
	 uint16_t y_position = 295; // Posizione verticale dei pallini
   uint16_t x_position = 15; // Posizione iniziale orizzontale (con un po' di margine)

	  // Pulizia della striscia in basso
    LCD_DrawFilledCircle(x_position, y_position, 7, White); // Cancella l'area (x=0, y=280, larghezza=240, altezza=40)
		LCD_DrawFilledCircle(x_position+30, y_position, 7, White);
		LCD_DrawFilledCircle(x_position+60, y_position, 7, White);
		int i = 1;
    for (i = 1; i <= Vite; i++) {      

        // Disegna il pallino rosso
        LCD_DrawFilledCircle(x_position, y_position, 7, Purple);

        // Aggiorna la posizione x per il prossimo pallino
        x_position += 30;
    }
		uint8_t Count[] = "vite restanti" ;
		GUI_Text(115, 290, Count ,Darkblue, White);
		PrintNumber(100, 290, Vite, Darkblue, White);

}



//estraggo 6 numeri casuali
void estraiNumeriCasuali(int numeri[], int n, int max) {
    int i, j;
    int estratto;

    // Inizializza il generatore di numeri casuali
    srand(1234);
    for (i = 0; i < n; i++) {
        do {
            estratto = rand() % (max + 1); // Genera un numero casuale tra 0 e max
            // Verifica che il numero non sia già stato estratto
            for (j = 0; j < i; j++) {
                if (numeri[j] == estratto) {
                    break;
                } }
        } while (j < i); // Ripeti finché trovi un numero già estratto
        numeri[i] = estratto; // Salva il numero estratto
    } }

		
void estraiPosizioniCasuali(int mat[31][28], int posizioni[6][2], int n) {
			int i, j, k = 0;
			int totale_posizioni = 0;

			// Array temporaneo per salvare tutte le posizioni valide (valore 1 nella matrice)
			int posizioni_valide[31 * 28][2];

			// Trova tutte le posizioni in cui il valore nella matrice è 1
			for (i = 0; i < 31; i++) {
					for (j = 0; j < 28; j++) {
							if (mat[i][j] == 1) {
									posizioni_valide[totale_posizioni][0] = i; // Salva la riga
									posizioni_valide[totale_posizioni][1] = j; // Salva la colonna
									totale_posizioni++;
							} } }
			// Inizializza il generatore di numeri casuali
			srand(1234);
			// Estrai n posizioni casuali senza ripetizioni
			for (i = 0; i <n; i++) {
					int indice_casuale;
					do {
							indice_casuale = rand() % totale_posizioni; // Genera un indice casuale
							// Verifica che la posizione non sia già stata scelta
							for (j = 0; j < i; j++) {
									if (posizioni[j][0] == posizioni_valide[indice_casuale][0] &&
											posizioni[j][1] == posizioni_valide[indice_casuale][1]) {
											break; } }
					} while (j < i); // Ripeti finché trovi una posizione già scelta

					// Salva la posizione scelta
					posizioni[i][0] = posizioni_valide[indice_casuale][0];
					posizioni[i][1] = posizioni_valide[indice_casuale][1];
	} }

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
