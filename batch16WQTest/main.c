/*
 * batch16WQTest.c
 *
 * Created: 12/13/2019 9:35:45 PM
 * Author :chathuraherath
 */
#define F_CPU 8000000UL
#define INTERVAL 1000
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include "keypad.h"
#include "lcd.h"
#include "delay.h"


void InitADC();
void TempControlFunction();
void turbControlFunction();
void phControlFunction();
void gsm_init();
void gsm_comm(char *a);
void send_sms();
void itemcode();
void mobileNumberEntering();
int startProcess();
int function00();
int function01();
int function02();
int chartonum(char x);
char numtochar(int x);
uint16_t ReadADC(uint8_t ch);

uint16_t adc_result;
uint8_t c = 0;
uint8_t key;
int num;
double phValue;
double tempValue;
double turbidityValue;
char mobileNumber[11];
char sampleNumber[3];
char qualityStatus[8];
char mobileMessageData[200];


int main()
{
	MCUCSR = (1 << JTD);
	MCUCSR = (1 << JTD);

	DDRA |= (1 << PORTA4);
	PORTA |= (1 << PORTA4);
	
	
	/*Connect RS->PB0, RW->PB1, EN->PB2 and data bus PORTB.4 to PORTB.7*/
	LCD_SetUp(PC_0, PC_1, PC_2, P_NC, P_NC, P_NC, P_NC, PC_4, PC_5, PC_6, PC_7);
	LCD_Init(2, 16);

	/*Connect R1->PD0, R2->PD1, R3->PD2 R4->PD3, C1->PD4, C2->PD5 C3->PD6, C4->PD7 */
	KEYPAD_Init(PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7);

	
	LCD_Clear();
	LCD_Printf("  Water Quality ");
	LCD_GoToLine(1);
	LCD_Printf("     Tester     ");
	DELAY_ms(2000);
	LCD_Clear();
	LCD_Printf("Initializing .");
	DELAY_ms(800);
	LCD_Printf(".");
	DELAY_ms(800);
	LCD_Printf(".");
	DELAY_ms(1500);
	LCD_Clear();

	itemcode();

	return 0;
}

void port_init(void)
{
	DDRB = 0xBF;
	PORTB = 0x00;

}
void init_devices(void)
{
	port_init();
	/*MCUCR = 0x00;
	GICR  = 0x00;
	TIMSK = 0x00; */
	
}

void InitADC()
{
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void itemcode()
{
	
	DDRA &= ~(1 << PORTA6);
	PORTA &= ~(1 << PORTA6);
	DDRA &= ~(1 << PORTA7);
	PORTA &= ~(1 << PORTA7);


	KEYPAD_Init(PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7);
	
	mobileNumberEntering();
}
void mobileNumberEntering()
{
	LCD_Clear();
	LCD_Printf("    WELCOME     ");
	DELAY_ms(1000);
	LCD_Clear();
	LCD_Printf("  ENTER  MOBILE ");
	LCD_GoToLine(1);
	LCD_Printf("   NUMBER -->   ");

	
	char n2[10];
	int i = 0;
	strcpy(mobileNumber, "0");
	for (; i < 9; i++)
	{
		key = KEYPAD_GetKey();
		if (i == 0)	LCD_Clear();
		num = chartonum(key);
		LCD_DisplayNumber(10, num, C_DisplayDefaultDigits_U8);
		itoa(num, n2, 10);
		strcat(mobileNumber, n2);
		DELAY_ms(100);
	}
	LCD_GoToLine(1);
	LCD_Clear();
	LCD_Printf(" Mobile number :");
	LCD_GoToLine(1);
	LCD_Printf("   ");
	LCD_Printf(mobileNumber);
	DELAY_ms(3000);
	LCD_Clear();
	strcat(mobileNumber, "\"");
	
	LCD_Printf("IF MOBILE NUMBER CORRECT PRESS 1");
	DELAY_ms(3000);
	LCD_Clear();
	LCD_Printf(" IF NOT PRESS 2 ");

	key = KEYPAD_GetKey();
	if (key == '2')	mobileNumberEntering();
	else
	{
		LCD_Clear();
		LCD_Printf("  ENTER SAMPLE       NUMBER     ");

		for (i = 0; i < 2; i++)
		{
			key = KEYPAD_GetKey();
			if (i == 0) LCD_Clear();			
			num = chartonum(key);
			LCD_DisplayNumber(10, num, C_DisplayDefaultDigits_U8);
			itoa(num, n2, 10);
			strcat(sampleNumber, n2);
			DELAY_ms(100);
		}
	startProcess();
	}

}

int startProcess()
{
	DDRA &= ~(1 << PORTA1);
	InitADC();
	phControlFunction();
	return 0;
}

void phControlFunction()
{
	char t[5];
	ReadADC(1);
	adc_result = ADC;
	phValue = ( (adc_result + 2)- 1093.59 )/(-62.94);
	dtostrf(phValue, 5, 2, t);
	LCD_Clear();
	LCD_Printf("pH =");
	LCD_Printf(t);
	DELAY_ms(2000);
	// 730=5.8    557=8.5  568=~8  900=2.9   940=2.5
	
	function00();
}
int function00()
{

	DDRA &= ~(1 << PORTA3);
	InitADC();
	TempControlFunction();
	return 0;
}

uint16_t ReadADC(uint8_t ch)
{
	ch = ch & 0b00000111;
	ADMUX |= ch;
	ADCSRA |= (1 << ADSC);
	while (!(ADCSRA & (1 << ADIF)))
		;
	ADCSRA |= (1 << ADIF);
	_delay_ms(100);
	return (ADC);
}

void TempControlFunction()
{
	char t[5];
	
	
	ReadADC(3);
	adc_result = ADC;
	// tempValue = 58-((adc_result*500/1024));
	//tempValue = (adc_result / 1.4) - 13;
	tempValue = (adc_result - 13.5)/1.45;
	// dtostrf(tempValue,5,2,t);  157-100 64-29  12-0  134-84  130-79 120-73  102-62 97-58  91-53 80-44
	dtostrf(tempValue, 5, 2, t);
	LCD_GoToLine(1);
	LCD_Printf("Temp = ");
	LCD_Printf(t);
	LCD_Printf(" C");
	DELAY_ms(2000);
	function01();
	
}
int function01()
{
	LCD_Clear();
	LCD_Printf(" Press key A to Continue Process");
	while (1)
	{
		key = KEYPAD_GetKey();
		if (key == 'A')
		{
			LCD_Clear();
			function02();
		}
		else if (key == '3')
		{
			LCD_Clear();
			TempControlFunction();
		}
		else if (key == '4')
		{
			startProcess();
		}
		else
		{
			LCD_Clear();
			LCD_Printf("  Press key A  ");
			function01();
		}
	}
	return 0;
}

int function02()
{
	InitADC();
	turbControlFunction();
	return 0;
}
void turbControlFunction()
{
	char t[5];
	ReadADC(2);
	adc_result = ADC;
	turbidityValue = 175 - ((adc_result * 200.0) / 1024);
	dtostrf(turbidityValue, 5, 2, t);
	LCD_Clear();
	LCD_Printf("TURB =");
	LCD_Printf(t);
	LCD_Printf("NTU");
	DELAY_ms(2000);
	

	if ((turbidityValue <= 12.0) && ((phValue > 6.5) && (phValue < 7.5)))
	{
		strcpy(qualityStatus, "Good");
		DDRA |= (1 << PORTA6);
		PORTA |= (1 << PORTA6);
		DELAY_ms(2000);
		
	}
	else
	{
		strcpy(qualityStatus, "Weak");
		DDRA |= (1 << PORTA7);
		PORTA |= (1 << PORTA7);
		DDRA |= (1 << PORTA5);
		PORTA |= (1 << PORTA5);
		DELAY_ms(2000);
		DDRA &= ~(1 << PORTA5);
		PORTA &= ~(1 << PORTA5);
	}

	
	LCD_Clear();
	//itemcode();
	LCD_Printf("  WANT TO SEND    REPORT AS SMS ");
	DELAY_ms(3000);
	LCD_Clear();
	LCD_GoToLine(1);
	LCD_Printf("  PRESS KEY 1   ");
	DELAY_ms(2000);
	LCD_Clear();
	LCD_Printf("     IF NOT        PRESS KEY 2  ");
	key = KEYPAD_GetKey();
	if(key == '1') gsm_init();
	else if (key == '3')
	{
		turbControlFunction();
	}
	
	else{
		LCD_Clear();
		itemcode();
	}
	
}

void gsm_init()
{
	UBRRL = 25;
	UCSRB |= (1 << RXEN) | (1 << TXEN);
	UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);

	send_sms();
}

void gsm_comm(char *a)
{

	while (*a > 0)
	{
		while (!(UCSRA & (1 << UDRE)))
			;
		UDR = *a++;
		DELAY_ms(10);
	}
}

void send_sms()
{
	LCD_Clear();
	char t[5];
	LCD_Printf("SENDING.");

	gsm_comm("AT");
	UDR = ('\r');
	LCD_Printf(" .");
	DELAY_ms(100);

	gsm_comm("AT+CMGF=1");
	UDR = ('\r');
	LCD_Printf(" .");
	DELAY_ms(100);

	gsm_comm("AT+CMGS=\"");
	LCD_Printf(" .");
	DELAY_ms(100);

	// gsm_comm("0714493567\"");
	gsm_comm(mobileNumber);
	LCD_Printf(" .");
	LCD_Printf(mobileNumber);
	DELAY_ms(100);
	UDR = ('\r');

	strcpy(mobileMessageData, "Water Quality Tester Report");
	strcat(mobileMessageData, "\nSample Number :");
	strcat(mobileMessageData, sampleNumber);
	strcat(mobileMessageData, "\nQuality Status :");
	strcat(mobileMessageData, qualityStatus);
	strcat(mobileMessageData, "\n  PH : ");
	strcat(mobileMessageData, dtostrf(phValue, 5, 2, t));
	strcat(mobileMessageData, "\n  Temperature : ");
	strcat(mobileMessageData, dtostrf(tempValue, 5, 2, t));
	strcat(mobileMessageData, " C");
	strcat(mobileMessageData, "\n  Tubidity level: ");
	strcat(mobileMessageData, dtostrf(turbidityValue, 5, 2, t));
	DELAY_ms(100);

	gsm_comm(mobileMessageData);
	UDR = (26);
	LCD_GoToLine(1);
	LCD_Printf("     sms sent   ");
	DELAY_ms(1000);
	LCD_GoToLine(1);
	LCD_Printf(" Next Testing");
	DELAY_ms(800);
	LCD_Printf(".");
	DELAY_ms(800);
	LCD_Printf(".");
	DELAY_ms(400);
	LCD_Printf(".");
	DELAY_ms(400);
	itemcode();

}

int chartonum(char x)
{
	if (x == '1')
		return 1;
	else if (x == '2')
		return 2;
	else if (x == '3')
		return 3;
	else if (x == '4')
		return 4;
	else if (x == '5')
		return 5;
	else if (x == '6')
		return 6;
	else if (x == '7')
		return 7;
	else if (x == '8')
		return 8;
	else if (x == '9')
		return 9;

	return (0);
}

// char numtochar(int x)
// {
// 	if (x == 0)
// 		return '0';
// 	if (x == 1)
// 		return '1';
// 	if (x == 2)
// 		return '2';
// 	if (x == 3)
// 		return '3';
// 	if (x == 4)
// 		return '4';
// 	if (x == 5)
// 		return '5';
// 	if (x == 6)
// 		return '6';
// 	if (x == 7)
// 		return '7';
// 	if (x == 8)
// 		return '8';
// 	if (x == 9)
// 		return '9';
// }
