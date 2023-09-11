#include <xc.h>
#include <stdbool.h>            //just only,,, false == 0, true == 1

// CONFIG1
#pragma config FOSC = INTOSC, WDTE = OFF, PWRTE = ON, MCLRE = OFF, CP = OFF, CPD = OFF, BOREN = ON, CLKOUTEN = OFF, IESO = ON, FCMEN = ON
// CONFIG2
#pragma config WRT = OFF, PLLEN = ON, STVREN = ON, BORV = HI,LVP = OFF

#define _XTAL_FREQ  32000000
#define analog_high  185	//?160->178->185
#define analog_low  78	//?	107->100->78

#define led_pin LATB0
#define buzzer_pin LATB1

#define cmd_pin LATA1
#define sel_pin LATA2
#define clk_pin LATA3
#define dat_pin RB7

uint8_t ps_dat[9], data[3];

void get_ps_con_new(void);
uint8_t ps_data(uint8_t pst);
void putch(uint8_t c);

int main (void) {
    OPTION_REG = 0b11010100;
    TMR0 = 0x00;
    OSCCON = 0b11110000;
    ///// UART/////
    TXCKSEL = true;//Set TX to RB5
    CREN = true;
    BRGH = true;
    BRG16 = true;
    SPBRG = 208;
    TXEN = true;
    SPEN = true;
	TRISA = 0b00000000;
	TRISB = 0b10000100;	
    ANSELA = 0x00;
    ANSELB = 0x00;
    T1CON = 0b00100001;//use instruction cycle, 1:4 Prescale	
			
    buzzer_pin = true;  __delay_ms(50);
    buzzer_pin = false; __delay_ms(50);
    buzzer_pin = true;  __delay_ms(50);
    buzzer_pin = false; __delay_ms(50);
		
	while(1){
		__delay_us(500);
		get_ps_con_new();	//Get Data From Controller

		data[0]=0;
		data[1]=0;
		data[2]=0;
	
		if(ps_dat[3] & 0x10)        {data[0] |= 0b10000000;}//UP
		if(ps_dat[3] & 0x40)        {data[0] |= 0b01000000;}//DOWN
		if(ps_dat[3] & 0x80)        {data[0] |= 0b00100000;}//Left
		if(ps_dat[3] & 0x20)        {data[0] |= 0b00010000;}//Right
		if(ps_dat[4] & 0x10)        {data[0] |= 0b00001000;}//Triangle
		if(ps_dat[4] & 0x40)        {data[0] |= 0b00000100;}//×
		if(ps_dat[4] & 0x80)        {data[0] |= 0b00000010;}//Square
		if(ps_dat[4] & 0x20)        {data[0] |= 0b00000001;}//Circle
	
		if(ps_dat[4] & 0x04)        {data[1] |= 0b10000000;}//L1
		if(ps_dat[4] & 0x01)        {data[1] |= 0b01000000;}//L2
		if(ps_dat[4] & 0x08)        {data[1] |= 0b00100000;}//R1
		if(ps_dat[4] & 0x02)        {data[1] |= 0b00010000;}//R2
		if(ps_dat[3] & 0x01)        {data[1] |= 0b00001000;}//Select
		if(ps_dat[3] & 0x08)        {data[1] |= 0b00000100;}//Start
		if(ps_dat[3] & 0x02)        {data[1] |= 0b00000010;}//Left SW
		if(ps_dat[3] & 0x04)        {data[1] |= 0b00000001;}//Right SW
	
		if(ps_dat[8]<=analog_low)   {data[2] |= 0b10000000;}
		if(ps_dat[8]>=analog_high)  {data[2] |= 0b01000000;}
		if(ps_dat[7]<=analog_low)   {data[2] |= 0b00100000;}
		if(ps_dat[7]>=analog_high)  {data[2] |= 0b00010000;}
		if(ps_dat[6]<=analog_low)   {data[2] |= 0b00001000;}
		if(ps_dat[6]>=analog_high)  {data[2] |= 0b00000100;}
		if(ps_dat[5]<=analog_low)   {data[2] |= 0b00000010;}
		if(ps_dat[5]>=analog_high)  {data[2] |= 0b00000001;}
		
        if((data[0] | data[1] | data[2]) == 0){
            led_pin = true;
        }
        else{
            led_pin = false;
            putch(0x99);
            putch(data[0]);
            putch(data[1]);
            putch(data[2]);
            putch(0x4f);
        }
	}
}

//;***************************************************
//;    PS Controller receive protocol 
//;***************************************************
void get_ps_con_new(void){
	uint8_t i,x;

	sel_pin = false;	
	ps_dat[0]=ps_data(0x01);
	ps_dat[1]=ps_data(0x42);
	x=((ps_dat[1] & 0x03) * 2) + 3;
        for(i=2;i<x;i++)    {ps_dat[i] = ps_data(0x00);}
	sel_pin = true;
	if(i==5){
		for(i=5;i<9;i++)   {ps_dat[i] = 0x7f;}//Fill 0x7f(neutral) when ANALOG is OFF
	}
	ps_dat[3] = ~ps_dat[3];
	ps_dat[4] = ~ps_dat[4];
}

//;***************************************************
//;   get PS data
//;   pst:sending data for PS con.
//;***************************************************
uint8_t ps_data(uint8_t pst){
	uint8_t i;

	for(i=8;i>0;i--){
		__delay_us(15);clk_pin = false;	//>7us
        cmd_pin = pst & 0x01;
        pst = pst >> 1;
        __delay_us(15); clk_pin = true;		//>7us
		if(dat_pin) {pst |= 0x80;}
    }
	return (pst);
}

void putch(uint8_t c){
    while(!TRMT);
    TXREG = c;
}