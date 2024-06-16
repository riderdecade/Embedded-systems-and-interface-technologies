#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_i2c.h"
#include "hw_types.h"
#include "i2c.h"
#include "pin_map.h"
#include "sysctl.h"
#include "systick.h"
#include "interrupt.h"
#include "uart.h"
#include "hw_ints.h"
#include "string.h"
#include "pwm.h"
#include "eeprom.h"


#define SYSTICK_FREQUENCY		1000			//1000hz

#define	I2C_FLASHTIME				500				//500mS
#define GPIO_FLASHTIME			300				//300mS
//*****************************************************************************
//
//I2C GPIO chip address and resigster define
//
//*****************************************************************************
#define TCA6424_I2CADDR 					0x22
#define PCA9557_I2CADDR						0x18

#define PCA9557_INPUT							0x00
#define	PCA9557_OUTPUT						0x01
#define PCA9557_POLINVERT					0x02
#define PCA9557_CONFIG						0x03

#define TCA6424_CONFIG_PORT0			0x0c
#define TCA6424_CONFIG_PORT1			0x0d
#define TCA6424_CONFIG_PORT2			0x0e

#define TCA6424_INPUT_PORT0				0x00
#define TCA6424_INPUT_PORT1				0x01
#define TCA6424_INPUT_PORT2				0x02

#define TCA6424_OUTPUT_PORT0			0x04
#define TCA6424_OUTPUT_PORT1			0x05
#define TCA6424_OUTPUT_PORT2			0x06


void reset(void); //系统重启
bool push(int order);
void DisplayDate(void); //数码管显示日期
void DisplayDate_left(void);
void DisplayDate_right(void);
void DisplayTime(void); //数码管显示时间
void DisplayAlarm(void);//Alarm
void DisplayTime_left(void);
void DisplayTime_right(void);
void jingwei(void); //
void INIT_CLOCK(void);
int check_run(int Y);
void ALARM_CALL(void);
void soundPWM(int tone);       //控制蜂鸣器音高
void PWM_Init(void);

void 		Delay(uint32_t value);
void 		S800_GPIO_Init(void);
uint8_t 	I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData);
uint8_t 	I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr);
void		S800_I2C0_Init(void);
void 		S800_UART_Init(void);
//systick software counter define
volatile uint16_t systick_10ms_couter,systick_100ms_couter;
volatile uint8_t	systick_10ms_status,systick_100ms_status;

volatile uint8_t result,cnt,key_value,gpio_status;
volatile uint8_t rightshift = 0x01;
volatile uint8_t rightshift1 = 0x01;
volatile uint8_t Flag_time = 0x01;
volatile uint8_t Flag_time1= 0x01;
volatile uint8_t Flag_date = 0x01;
uint32_t ui32SysClock;

uint8_t seg7[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x58,0x5e,0x79,0x71,0x40};  //无小数点0-F
uint8_t seg_7[] = {0xbf,0x86,0xdb,0xcf,0xe6,0xed,0xfd,0x87,0xff,0xef,0xf7,0xfc,0xb9,0xde,0xf9,0xf1};   //带小数点0-F
uint8_t led7[] = {0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe};
uint8_t ledlight[] = {0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};   //led亮1-8
uint8_t name[] = {0x67,0x0f,0x77,0x54,0x3d,0x78,0x77,0x5c};
int month_num[12]={31,28,31,30,31,30,31,31,30,31,30,31}; // month

uint8_t year=24,month=6,day=16,hour=12,minute=0,second=0,alarm_hour=11,alarm_minute=59,alarm_second=59;
uint32_t s_time[10],r_time[10];
uint32_t ui32EEPROMInit;

int func = -1;								//功能选择
int Delay_time_date = 200;
int Delay_time_time = 200;
int Delay_time_alarm= 200;
int temp_hour=0,temp_minute=0,temp_second=0;
int temp_year=0,temp_month=1,temp_day=1;
int temp_alarm_hour=0,temp_alarm_minute=0,temp_alarm_second=0;
int Count_time=0;
int show_time[10];
int show_date[10];
int speed1=1000;
int speed2=200;
int speed=1000;

uint8_t uart_receive_char;
uint8_t rx_flag=0;
uint32_t opening[10]={45454,19120,17021,15174,14316,12755,11364,10121,8511};

char *uart_receive_string;
char *KEY1="AT+CLASS";
char *KEY2="AT STUDENTCODE";
char *KEY_Init="INIT CLOCK";
char *KEY_get_time="GET TIME";
char *KEY_get_date="GET DATE";
char *KEY_set_time="SET TIME";
char *KEY_set_date="SET DATE";
char RxBuf[100];
char Out[100];
char standard[100];

int main(void)
{
	volatile uint16_t	i2c_flash_cnt,gpio_flash_cnt;
	int flag=1;
	int flag1=1;
	//use internal 16M oscillator, PIOSC
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 16000000);		
	//ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT |SYSCTL_USE_OSC), 8000000);		
	//use external 25M oscillator, MOSC
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |SYSCTL_USE_OSC), 25000000);		

	//use external 25M oscillator and PLL to 120M
   //ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 120000000);;		
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_OSC_INT | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 20000000);
	
  SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY);
	SysTickEnable();
	SysTickIntEnable();																		//Enable Systick interrupt
	  

	S800_GPIO_Init();
	S800_I2C0_Init();
	S800_UART_Init();
	PWM_Init();
	
	IntEnable(INT_UART0);
  UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);	//Enable UART0 RX,TX interrupt
  IntMasterEnable();		
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
	//
	// Wait for the EEPROM module to be ready.
	//
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0)){}
	//
	// Wait for the EEPROM Initialization to complete
	//
	ui32EEPROMInit = EEPROMInit();
	//
	// Check if the EEPROM Initialization returned an error
	// and inform the application
	//
	if(ui32EEPROMInit != EEPROM_INIT_OK)
	{
		while(1)
		{
		}
	}
	
	reset();
	
	while (1)
	{
		if(push(1)) 
		{
			func=func+1;
			func=func%6;
		}
		
		if(func==0) // 日期显示
		{
			if(push(3)) // the slow version
			{
				speed=1000;
			}
			if(push(4)) // the quick version
			{
				speed=200;
			}
			if(push(8)) // left
			{
				flag=0;
			}
			if(push(7)) // right
			{
				flag=2;
			}
			if(push(6)) // normal
			{
				flag=1;
			}
			if(flag==1) DisplayDate();
			else if(flag==0) DisplayDate_left();
			else if(flag==2) DisplayDate_right();
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,ledlight[func]);	
			Delay(Delay_time_date);
		}
		else if(func==1) //日期设置
		{
			DisplayDate();
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,ledlight[func]);	
			Delay(200);
			if(push(3))//day plus
			{
				if(day<month_num[month-1])
				{
					day=day+1;
					DisplayDate();
					Delay(100);
				}
			}
			if(push(4))//day minus
			{
				if(day>1)
				{
					day=day-1;
					DisplayDate();
					Delay(100);
				}
			}
			if(push(8))//month plus
			{
				if(month<12)
				{
					month=month+1;
					DisplayDate();
					Delay(100);
				}
			}
			if(push(7))//month minus
			{
				if(month>1)
				{
					month=month-1;
					DisplayDate();
					Delay(100);
				}
			}
			if(push(6))//year plus
			{
				if(year<99)
				{
					year=year+1;
					DisplayDate();
					Delay(100);
				}
			}
			if(push(5))//year minus
			{
				if(year>0)
				{
					year=year-1;
					DisplayDate();
					Delay(100);
				}
			}
		}
		else if(func==2) //时间显示
		{
			//DisplayTime();
			if(push(3)) // the slow version
			{
				speed=1000;
			}
			if(push(4)) // the quick version
			{
				speed=200;
			}
			
			if(push(8)) // left
			{
				flag=0;
			}
			if(push(7)) // right
			{
				flag=2;
			}
			if(push(6)) // normal
			{
				flag=1;
			}
			if(flag==1) DisplayTime();
			else if(flag==0) DisplayTime_left();
			else if(flag==2) DisplayTime_right();
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,ledlight[func]);	
			Delay(Delay_time_time);
		}
		else if(func==3) //：时间设置
		{
			DisplayTime();
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,ledlight[func]);	
			Delay(200);
			if(push(3))//second plus
			{
				if(second<59)
				{
					second=second+1;
					DisplayTime();
					Delay(100);
				}
			}
			if(push(4))//second minus
			{
				if(second>0)
				{
					second=second-1;
					DisplayTime();
					Delay(100);
				}
			}
			if(push(8))//minute plus
			{
				if(minute<59)
				{
					minute=minute+1;
					DisplayTime();
					Delay(100);
				}
			}
			if(push(7))//minute minus
			{
				if(minute>0)
				{
					minute=minute-1;
					DisplayTime();
					Delay(100);
				}
			}
			if(push(6))//hour plus
			{
				if(hour<23)
				{
					hour=hour+1;
					DisplayTime();
					Delay(100);
				}
			}
			if(push(5))//hour minus
			{
				if(hour>0)
				{
					hour=hour-1;
					DisplayTime();
					Delay(100);
				}
			}
		}
		else if(func==4)//：闹钟显示及设置
		{
			DisplayAlarm();
			if(push(3)) // the slow version
			{
				Delay_time_alarm=1000000;
			}
			if(push(4)) // the quick version
			{
				Delay_time_alarm=300000;
			}
			if(push(5)) // the quickest version
			{
				Delay_time_alarm=200;
			}
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,ledlight[func]);	
			Delay(Delay_time_alarm);
		}
		else if(func==5)// set the alarm
		{
			DisplayAlarm();
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,ledlight[func]);	
			Delay(200);
			if(push(3))//alarm_second plus
			{
				if(alarm_second<59)
				{
					alarm_second=alarm_second+1;
					DisplayAlarm();
					Delay(100);
				}
			}
			if(push(4))//alarm_second minus
			{
				if(alarm_second>0)
				{
					alarm_second=alarm_second-1;
					DisplayAlarm();
					Delay(100);
				}
			}
			if(push(8))//alarm_minute plus
			{
				if(alarm_minute<59)
				{
					alarm_minute=alarm_minute+1;
					DisplayAlarm();
					Delay(100);
				}
			}
			if(push(7))//alarm_minute minus
			{
				if(alarm_minute>0)
				{
					alarm_minute=alarm_minute-1;
					DisplayAlarm();
					Delay(100);
				}
			}
			if(push(6))//alarm_hour plus
			{
				if(alarm_hour<23)
				{
					alarm_hour=alarm_hour+1;
					DisplayAlarm();
					Delay(100);
				}
			}
			if(push(5))//alarm_hour minus
			{
				if(alarm_hour>0)
				{
					alarm_hour=alarm_hour-1;
					DisplayAlarm();
					Delay(100);
				}
			}
		}
	}
}

bool push(int order)       //按键检测
{
	if(I2C0_ReadByte(TCA6424_I2CADDR, TCA6424_INPUT_PORT0) == 0xff)	
		return false;
	if(I2C0_ReadByte(TCA6424_I2CADDR, TCA6424_INPUT_PORT0) == led7[8-order])
	{	
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	
		Delay(1000000); 
		while(I2C0_ReadByte(TCA6424_I2CADDR, TCA6424_INPUT_PORT0) == led7[8-order])
		GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_PIN_0);
		Delay(1000);
		GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,0x0);
		return true;
	}
	return false;
}

void DisplayTime(void)
{
	int i=0;
	if(rightshift==0x01)
	{
		i=hour/10;
	}
	else if(rightshift==0x02)
	{
		i=hour%10;
	}
	else if(rightshift==0x04)
	{
		i=16;
	}
	else if(rightshift==0x08)
	{
		i=minute/10;
	}
	else if(rightshift==0x10)
	{
		i=minute%10;
	}
	else if(rightshift==0x20)
	{
		i=16;
	}
	else if(rightshift==0x40)
	{
		i=second/10;
	}
	else if(rightshift==0x80)
	{
		i=second%10;
	}
	
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[i]);	//write port 1 				
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);	//write port 2
	
	if(rightshift!=0x80)
	{
		rightshift=rightshift<<1;
	}
	else
	{
		rightshift=0x01;
	}
}

void DisplayTime_left(void)
{
	int i=0;
	if(rightshift==0x01)
	{
		i=hour/10;
	}
	else if(rightshift==0x02)
	{
		i=hour%10;
	}
	else if(rightshift==0x04)
	{
		i=16;
	}
	else if(rightshift==0x08)
	{
		i=minute/10;
	}
	else if(rightshift==0x10)
	{
		i=minute%10;
	}
	else if(rightshift==0x20)
	{
		i=16;
	}
	else if(rightshift==0x40)
	{
		i=second/10;
	}
	else if(rightshift==0x80)
	{
		i=second%10;
	}
	
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[i]);	//write port 1 				
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);	//write port 2
	
	if(rightshift<Flag_time)
	{
		rightshift=rightshift<<1;
	}
	else
	{
		rightshift=0x01;
	}
}

void DisplayTime_right(void)
{
	int i=0;
	if(rightshift==0x01)
	{
		i=hour/10;
	}
	else if(rightshift==0x02)
	{
		i=hour%10;
	}
	else if(rightshift==0x04)
	{
		i=16;
	}
	else if(rightshift==0x08)
	{
		i=minute/10;
	}
	else if(rightshift==0x10)
	{
		i=minute%10;
	}
	else if(rightshift==0x20)
	{
		i=16;
	}
	else if(rightshift==0x40)
	{
		i=second/10;
	}
	else if(rightshift==0x80)
	{
		i=second%10;
	}
	
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[i]);	//write port 1 				
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);	//write port 2
	
	if(rightshift>Flag_time1)
	{
		rightshift=rightshift>>1;
	}
	else
	{
		rightshift=0x80;
	}
}

void DisplayAlarm(void)
{
	int i=0;
	if(rightshift==0x01)
	{
		i=alarm_hour/10;
	}
	else if(rightshift==0x02)
	{
		i=alarm_hour%10;
	}
	else if(rightshift==0x04)
	{
		i=16;
	}
	else if(rightshift==0x08)
	{
		i=alarm_minute/10;
	}
	else if(rightshift==0x10)
	{
		i=alarm_minute%10;
	}
	else if(rightshift==0x20)
	{
		i=16;
	}
	else if(rightshift==0x40)
	{
		i=alarm_second/10;
	}
	else if(rightshift==0x80)
	{
		i=alarm_second%10;
	}
	
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[i]);	//write port 1 				
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);	//write port 2
	
	if(rightshift!=0x80)
	{
		rightshift=rightshift<<1;
	}
	else
	{
		rightshift=0x01;
	}
}

void DisplayDate(void)
{
	int i=0;
	if(rightshift==0x01)
	{
		i=year/10;
	}
	else if(rightshift==0x02)
	{
		i=year%10;
	}
	else if(rightshift==0x04)
	{
		i=16;
	}
	else if(rightshift==0x08)
	{
		i=month/10;
	}
	else if(rightshift==0x10)
	{
		i=month%10;
	}
	else if(rightshift==0x20)
	{
		i=16;
	}
	else if(rightshift==0x40)
	{
		i=day/10;
	}
	else if(rightshift==0x80)
	{
		i=day%10;
	}
	
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[i]);	//write port 1 				
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);	//write port 2
	
	if(rightshift!=0x80)
	{
		rightshift=rightshift<<1;
	}
	else
	{
		rightshift=0x01;
	}
}

void DisplayDate_left(void)
{
	int i=0;
	if(rightshift==0x01)
	{
		i=year/10;
	}
	else if(rightshift==0x02)
	{
		i=year%10;
	}
	else if(rightshift==0x04)
	{
		i=16;
	}
	else if(rightshift==0x08)
	{
		i=month/10;
	}
	else if(rightshift==0x10)
	{
		i=month%10;
	}
	else if(rightshift==0x20)
	{
		i=16;
	}
	else if(rightshift==0x40)
	{
		i=day/10;
	}
	else if(rightshift==0x80)
	{
		i=day%10;
	}
	
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[i]);	//write port 1 				
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);	//write port 2
	
	if(rightshift<Flag_time)
	{
		rightshift=rightshift<<1;
	}
	else
	{
		rightshift=0x01;
	}
}

void DisplayDate_right(void)
{
	int i=0;
	if(rightshift==0x01)
	{
		i=year/10;
	}
	else if(rightshift==0x02)
	{
		i=year%10;
	}
	else if(rightshift==0x04)
	{
		i=16;
	}
	else if(rightshift==0x08)
	{
		i=month/10;
	}
	else if(rightshift==0x10)
	{
		i=month%10;
	}
	else if(rightshift==0x20)
	{
		i=16;
	}
	else if(rightshift==0x40)
	{
		i=day/10;
	}
	else if(rightshift==0x80)
	{
		i=day%10;
	}
	
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[i]);	//write port 1 				
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);	//write port 2
	
	if(rightshift>Flag_time1)
	{
		rightshift=rightshift>>1;
	}
	else
	{
		rightshift=0x80;
	}
}

void jingwei(void)
{
	//time
	if(second>=60)
	{
		second=second-60;
		minute=minute+1;
	}
	if(minute>=60)
	{
		minute=minute-60;
		hour=hour+1;
	}
	if(hour>=24)
	{
		hour=hour-24;
		day=day+1;
	}
	//date
	if(year%400==0||((year%100!=0)&&(year%4==0)))
	{
		month_num[1]=29;
	}
	else
	{
		month_num[1]=28;
	}
	if(day>month_num[month-1])
	{
		day=day-month_num[month-1];
		month=month+1;
	}
	if(month>12)
	{
		month=month-12;
		year=year+1;
	}
}

void reset(void)                     //重启
{
	int i = 0,j = 0;	
	int StuID=31910206;
	int cnt1=0;
	EEPROMRead(r_time, 0x400, sizeof(r_time));
	year=r_time[0];
	month=r_time[1];
	day=r_time[2];
	hour=r_time[3];
	minute=r_time[4];
	second=r_time[5];
	alarm_hour=r_time[6];
	alarm_minute=r_time[7];
	alarm_second=r_time[8];
	Count_time=0;
	for(j=0;j<420;j++)
	{
		rightshift1=0x01;
		StuID=31910206;     //学号后八位倒置
		StuID=60201913;
		for(i=0;i<8;i++)
		{
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[StuID%10]);	//write port 1 				
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift1);	//write port 2
			Delay(4000);
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//write port 2
			StuID/=10;

			rightshift1 = rightshift1<<1;
		}
		if((0<j&&j<60)||(120<j&&j<180)||(240<j&&j<300)||(360<j&&j<420))
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0);			//全亮
		else
			result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0ff);			//全灭
	}
	for(j=0;j<420;j++)
	{
		rightshift1=0x01;
		for(i=0;i<8;i++)
		{
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,name[i]);	//write port 1 				
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift1);	//write port 2

			Delay(4000);
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//write port 2

			rightshift1 = rightshift1<<1;
			}
			if((0<j&&j<60)||(120<j&&j<180)||(240<j&&j<300)||(360<j&&j<420))
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0);			//全亮
			else
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0ff);			//全灭
	}
	for(j=0;j<420;j++)//version
	{
		int now[10];
		now[0]=1;
		now[1]=16;
		now[2]=1;
		rightshift1=0x01;
		for(i=0;i<3;i++)
		{
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[now[i]]);	//write port 1 				
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift1);	//write port 2

			Delay(4000);
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//write port 2

			rightshift1 = rightshift1<<1;
			}
			if((0<j&&j<60)||(120<j&&j<180)||(240<j&&j<300)||(360<j&&j<420))
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0);			//全亮
			else
				result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0ff);			//全灭
	}
}


void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}


void UARTStringPut(uint8_t *cMessage)
{
	while(*cMessage!='\0')
		UARTCharPut(UART0_BASE,*(cMessage++));
}
void UARTStringPutNonBlocking(const char *cMessage)
{
	while(*cMessage!='\0')
		UARTCharPutNonBlocking(UART0_BASE,*(cMessage++));
}

void PWM_Init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);

  GPIOPinConfigure(GPIO_PK5_M0PWM7);

	GPIOPinTypePWM(GPIO_PORTK_BASE, GPIO_PIN_5);
  PWMGenConfigure(PWM0_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN |PWM_GEN_MODE_NO_SYNC);	
  PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, 20000);	
	
    PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_1);
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7,20000/2);
	PWMOutputState(PWM0_BASE, PWM_OUT_7_BIT, false);
	PWMGenEnable(PWM0_BASE, PWM_GEN_3);
}


void S800_UART_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);						//Enable PortA
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));			//Wait for the GPIO moduleA ready

	GPIOPinConfigure(GPIO_PA0_U0RX);												// Set GPIO A0 and A1 as UART pins.
  GPIOPinConfigure(GPIO_PA1_U0TX);    			

  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Configure the UART for 115,200, 8-N-1 operation.
  UARTConfigSetExpClk(UART0_BASE, ui32SysClock,115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));
	UARTFIFOLevelSet(UART0_BASE,UART_FIFO_RX1_8,UART_FIFO_RX7_8);
	UARTStringPut((uint8_t *)"\r\nHello, world! Welcome!!\r\n");
}
void S800_GPIO_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);						//Enable PortN	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));			//Wait for the GPIO moduleN ready		
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);			//Set PF0 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);			//Set PN0 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);		//Set PN1 as Output pin	

	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}

void S800_I2C0_Init(void)
{
	uint8_t result;
  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);
  GPIOPinConfigure(GPIO_PB3_I2C0SDA);
  GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
  GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

	I2CMasterInitExpClk(I2C0_BASE,ui32SysClock, true);										//config I2C0 400k
	I2CMasterEnable(I2C0_BASE);	

	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT0,0x0ff);		//config port 0 as input
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT1,0x0);			//config port 1 as output
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT2,0x0);			//config port 2 as output 

	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_CONFIG,0x00);					//config port as output
	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0ff);				//turn off the LED1-8
	
}


uint8_t I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData)
{
	uint8_t rop;
	while(I2CMasterBusy(I2C0_BASE)){};
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C0_BASE)){};
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);

	I2CMasterDataPut(I2C0_BASE, WriteData);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(I2CMasterBusy(I2C0_BASE)){};

	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	return rop;
}

uint8_t I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr)
{
	uint8_t value,rop;
	while(I2CMasterBusy(I2C0_BASE)){};	
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
//	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);		
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_SEND);
	while(I2CMasterBusBusy(I2C0_BASE));
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	Delay(100);
	//receive data
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, true);
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_RECEIVE);
	while(I2CMasterBusBusy(I2C0_BASE));
	value=I2CMasterDataGet(I2C0_BASE);
		Delay(100);
	return value;
}

/*
	Corresponding to the startup_TM4C129.s vector table systick interrupt program name
*/
void SysTick_Handler(void)
{
	if (systick_100ms_couter	!= 0)
		systick_100ms_couter--;
	else
	{
		systick_100ms_couter	= SYSTICK_FREQUENCY/10;
		systick_100ms_status 	= 1;
	}
	
	if (systick_10ms_couter	!= 0)
		systick_10ms_couter--;
	else
	{
		systick_10ms_couter		= SYSTICK_FREQUENCY/100;
		systick_10ms_status 	= 1;
	}
	if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0)
	{
		systick_100ms_status	= systick_10ms_status = 0;
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,GPIO_PIN_0);		
	}
	else
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,0);		
	
	Count_time=Count_time+1;
	if(func!=3)
	{
		if(Count_time%speed==0)
		{
			if(Flag_time<0x80)
				Flag_time=Flag_time<<1;
			else
				Flag_time=0x01;
			
			if(Flag_time1>0x01)
				Flag_time1=Flag_time1>>1;
			else
				Flag_time1=0x80;
		}
		if(Count_time%1000==0)
		{
			second=second+1;
			Count_time=0;
			jingwei();
			s_time[0]=year;
			s_time[1]=month;
			s_time[2]=day;
			s_time[3]=hour;
			s_time[4]=minute;
			s_time[5]=second;
			s_time[6]=alarm_hour;
			s_time[7]=alarm_minute;
			s_time[8]=alarm_second;
			EEPROMProgram(s_time, 0x400, sizeof(s_time));
		}
	}
	if(func!=5)
	{
		if(Count_time%1000==0)
		{
			if(alarm_hour==hour&&alarm_minute==minute&&alarm_second==second)
			{
				UARTStringPut((uint8_t *)"\r\nIt's the time!! Wake up!!\n\r");
				ALARM_CALL();
				func=2;
			}
		}
	}
}

void soundPWM(int tone)
{
	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, tone);
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7,tone/2);
	PWMOutputState(PWM0_BASE,PWM_OUT_7_BIT,true);
}

void ALARM_CALL(void)
{
	uint8_t cnt1=0;
	for(cnt1 = 0; cnt1 < 8; cnt1++)
	{
		uint8_t alarm[50]={1,3,5,3,4,3,2,1};
		int bat[50] = {1000000,1000000,1000000,1000000,1000000,1000000,1000000,1000000};
		DisplayTime();
		soundPWM(opening[alarm[cnt1]]);
		Count_time=Count_time+100;
		result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,ledlight[cnt1]);	
		Delay(bat[cnt1]);
	}
	PWMOutputState(PWM0_BASE,PWM_OUT_7_BIT,false);
	
}

/*
	Corresponding to the startup_TM4C129.s vector table UART0_Handler interrupt program name
*/

/*
void UART0_Handler(void)
{
	int32_t uart0_int_status;
  uart0_int_status 		= UARTIntStatus(UART0_BASE, true);		// Get the interrrupt status.

  UARTIntClear(UART0_BASE, uart0_int_status);								//Clear the asserted interrupts

  while(UARTCharsAvail(UART0_BASE))    											// Loop while there are characters in the receive FIFO.
  {
		///Read the next character from the UART and write it back to the UART.
    //UARTCharPutNonBlocking(UART0_BASE,UARTCharGetNonBlocking(UART0_BASE));
		//GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,GPIO_PIN_1 );		
//		Delay(1000);
		
	}
	//GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,0 );	
}
*/
void UARTStringGet(uint32_t ui32Base,char *cMessage,const char Iden)
{
    while(1)
	{
	  *cMessage=UARTCharGet(ui32Base);
	  if(*cMessage!=Iden)
	  {
    		cMessage=cMessage+1;
	  }
  	  else
	  {
	   	*cMessage='\0';
  	   	 break;
	  }		
	}	
}


void UART0_Handler(void)
{	uint8_t cnt = 0,flag1=0,flag2=0, I=0;
	uint32_t ulStatus;
	ulStatus = UARTIntStatus(UART0_BASE, true);
	UARTIntClear(UART0_BASE, ulStatus);

	while( UARTCharsAvail(UART0_BASE) )  {
		UARTStringGet(UART0_BASE,RxBuf,'@');
		//RxBuf[cnt++]= UARTCharGetNonBlocking(UART0_BASE); 
  } 
	// RxBuf[cnt]='\0';
	//UARTStringPutNonBlocking(RxBuf);
	if(strcmp(RxBuf, KEY1)==0)UARTStringPut((uint8_t *)"\r\nCLASS2212\n\r");
	if(strcmp(RxBuf, KEY2)==0)UARTStringPut((uint8_t *)"\r\nCODE522031910206\n\r");
	if(strcmp(RxBuf,"?")==0)
	{
		UARTStringPut((uint8_t *)"\r\nPlease add [@] at the end of each command\r\n");
		UARTStringPut((uint8_t *)"\r\nPlease enter [INIT CLOCK]to initialize the clock.\r\n");
		UARTStringPut((uint8_t *)"\r\nPlease enter [SET TIME **:**:**] to set the clock.\r\n");
		UARTStringPut((uint8_t *)"\r\nPlease enter [SET DATE **:**:**] to set the date.\r\n");
		UARTStringPut((uint8_t *)"\r\nPlease enter [SET ALARM **:**:**] to set the ALARM.\r\n");
		UARTStringPut((uint8_t *)"\r\nPlease enter [GET TIME] to get the current time.\r\n");
		UARTStringPut((uint8_t *)"\r\nPlease enter [GET DATE] to get the date.\r\n");
		UARTStringPut((uint8_t *)"\r\nPlease enter [GET ALARM] to get the alarm.\r\n");
	}
	if(strcmp(RxBuf, "INIT CLOCK")==0)
	{
		INIT_CLOCK();
		UARTStringPut((uint8_t *)"\r\nInit Complete!\n\r");
	}
	if(strcmp(RxBuf, "GET TIME") == 0) 
	{
		sprintf(Out, "\r\n%4s%02u%s%02u%s%02u\n\r", "TIME:", hour, ":", minute, ":", second);
    UARTStringPutNonBlocking(Out);
  }
	if(strcmp(RxBuf, "GET ALARM") == 0) 
	{
		sprintf(Out, "\r\n%4s%02u%s%02u%s%02u\n\r", "ALARM:", alarm_hour, ":", alarm_minute, ":", alarm_second);
    UARTStringPutNonBlocking(Out);
  }
	if(strcmp(RxBuf, "GET DATE") == 0) 
	{
		sprintf(Out, "\r\n%4s%02u%s%02u%s%02u\n\r", "DATE:", year, ":", month, ":", day);
    UARTStringPutNonBlocking(Out);
  }
	if(RxBuf[0]=='S'&&RxBuf[1]=='E'&&RxBuf[2]=='T'&&RxBuf[4]=='T'&&RxBuf[5]=='I'&&RxBuf[6]=='M'&&RxBuf[7]=='E')//set time
	{
		temp_hour=(RxBuf[9]-'0')*10+(RxBuf[10]-'0');
		temp_minute=(RxBuf[12]-'0')*10+(RxBuf[13]-'0');
		temp_second=(RxBuf[15]-'0')*10+(RxBuf[16]-'0');
		if(temp_hour<24&&temp_hour>=0&&temp_minute<60&&temp_minute>=0&&temp_second<60&&temp_second>=0)
		{
			hour=temp_hour;
			minute=temp_minute;
			second=temp_second;
			//sprintf(Out, "\r\n%4s%02u%02u%s%02u%02u%s%02u%02u\n\r", "SET TIME:", hour/10,hour%10, ":", minute/10,minute%10, ":", second/10,second%10);
      sprintf(Out, "%4s%02u%s%02u%s%02u", "SETTIME:", hour, ":", minute, ":", second);
      //UARTStringPutNonBlocking("\n");
			UARTStringPutNonBlocking(Out);
		}
		else
		{
			UARTStringPutNonBlocking("\r\nInvalid Inputs\n\r");
		}
	}
	if(RxBuf[0]=='S'&&RxBuf[1]=='E'&&RxBuf[2]=='T'&&RxBuf[4]=='D'&&RxBuf[5]=='A'&&RxBuf[6]=='T'&&RxBuf[7]=='E')//set date
	{
		temp_year=(RxBuf[9]-'0')*10+(RxBuf[10]-'0');
		temp_month=(RxBuf[12]-'0')*10+(RxBuf[13]-'0');
		temp_day=(RxBuf[15]-'0')*10+(RxBuf[16]-'0');
		if(temp_year<100&&temp_year>=0&&temp_month<=12&&temp_month>=1&&temp_day<=month_num[temp_month-1]+check_run(temp_year+2000)&&temp_day>=1)
		{
			year=temp_year;
			month=temp_month;
			day=temp_day;
			//sprintf(Out, "\r\n%4s%02u%02u%s%02u%02u%s%02u%02u\n\r", "SET TIME:", year/10,year%10, ":", month/10,month%10, ":", day/10,day%10);
      sprintf(Out, "%4s%02u%s%02u%s%02u", "SETDATE:", year, ":", month, ":", day);
			//UARTStringPutNonBlocking("\n");
      UARTStringPutNonBlocking(Out);
		}
		else
		{
			UARTStringPutNonBlocking("\r\nInvalid Inputs\n\r");
		}
	}
	if(RxBuf[0]=='S'&&RxBuf[1]=='E'&&RxBuf[2]=='T'&&RxBuf[4]=='A'&&RxBuf[5]=='L'&&RxBuf[6]=='A'&&RxBuf[7]=='R'&&RxBuf[8]=='M')//set alarm
	{
		temp_alarm_hour=(RxBuf[10]-'0')*10+(RxBuf[11]-'0');
		temp_alarm_minute=(RxBuf[13]-'0')*10+(RxBuf[14]-'0');
		temp_alarm_second=(RxBuf[16]-'0')*10+(RxBuf[17]-'0');
		if(temp_alarm_hour<24&&temp_alarm_hour>=0&&temp_alarm_minute<60&&temp_alarm_minute>=0&&temp_alarm_second<60&&temp_alarm_second>=0)
		{
			alarm_hour=temp_alarm_hour;
			alarm_minute=temp_alarm_minute;
			alarm_second=temp_alarm_second;
			//sprintf(Out, "\r\n%4s%02u%02u%s%02u%02u%s%02u%02u\n\r", "SET TIME:", hour/10,hour%10, ":", minute/10,minute%10, ":", second/10,second%10);
      sprintf(Out, "%4s%02u%s%02u%s%02u", "SETALARM:", alarm_hour, ":", alarm_minute, ":", alarm_second);
      //UARTStringPutNonBlocking("\n");
			UARTStringPutNonBlocking(Out);
		}
		else
		{
			UARTStringPutNonBlocking("\r\nInvalid Inputs\n\r");
		}
	}
	
	if(RxBuf[2]=='+')//plus
	{
		flag1=(RxBuf[0]-'0')*10+(RxBuf[1]-'0');
		flag2=(RxBuf[3]-'0')*10+(RxBuf[4]-'0');
		sprintf(Out, "%4s%02u%s%02u%s%02u\n", "ANSWER:", flag1, "+", flag2, "=", flag1+flag2);
    //UARTStringPutNonBlocking("\n");
		UARTStringPutNonBlocking(Out);
	}
	else if(RxBuf[2]=='-')
	{
		flag1=(RxBuf[0]-'0')*10+(RxBuf[1]-'0');
		flag2=(RxBuf[3]-'0')*10+(RxBuf[4]-'0');
		if(flag1>flag2)
			sprintf(Out, "%4s%02u%s%02u%s%02u\n", "ANSWER:", flag1, "-", flag2, "=", flag1-flag2);
    else
			sprintf(Out, "%4s%02u%s%02u%s%s%02u\n", "ANSWER:", flag1, "-", flag2, "=","-", flag2-flag1);
		//UARTStringPutNonBlocking("\n");
		UARTStringPutNonBlocking(Out);
	}
	
	for(I=0;I<strlen(RxBuf);++I)
	{
		if(RxBuf[I]!=' ')
		{
			standard[cnt++]=RxBuf[I];
			if(standard[cnt-1]>'a')
			{
				standard[cnt-1]=standard[cnt-1]-'a'+'A';
			}
		}
	}
	standard[cnt]='\0';
	if(strcmp(standard, "GETTIME")==0)
	{
		sprintf(Out, "\r\n%4s%02u%s%02u%s%02u\n\r", "TIME:", hour, ":", minute, ":", second);
    UARTStringPutNonBlocking(Out);
	}
	//memset(RxBuf, 0, sizeof(RxBuf));
}

int check_run(int Y)
{
	if(Y%400==0)return 1;
	if(Y%100==0)return 0;
	if(Y%4==0)return 1;
	return 0;
}

void INIT_CLOCK(void)
{
	year=24;
	month=6;
	day=16;
	hour=12;
	minute=0;
	second=0;
	alarm_hour=11;
	alarm_minute=59;
	alarm_second=59;
}