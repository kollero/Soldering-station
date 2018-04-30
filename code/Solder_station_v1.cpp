/*
 * Solder_station_v1.cpp
 *
 * Created: 13.4.2016 21:26:34
 *  Author: Panu Leinonen
 */ 



#include <string.h>
#include <stdlib.h>
//#include <math.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
//#include <avr/wdt.h>
#include <avr/io.h>
#include <stdint.h>
//#include <stdbool.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "u8g.h"
u8g_t u8g;

uint16_t EEMEM saved_temperature;
	
typedef struct{
	unsigned int bit0:1;
	unsigned int bit1:1;
	unsigned int bit2:1;
	unsigned int bit3:1;
	unsigned int bit4:1;
	unsigned int bit5:1;
	unsigned int bit6:1;
	unsigned int bit7:1;
} _io_reg;
#define REGISTER_BIT(rg,bt) ((volatile _io_reg*)&rg)->bit##bt

#define	ENCA REGISTER_BIT(PIND,2)
#define	ENCB REGISTER_BIT(PIND,3)
#define	ENCCLK REGISTER_BIT(PIND,1)

#define	INSTRUMENTATION_PORT REGISTER_BIT(PORTD,5)
#define INSTR_IDLE INSTRUMENTATION_PORT = 1   
#define INSTR_ACTIVE INSTRUMENTATION_PORT = 0

#define	HEATER_PORT REGISTER_BIT(PORTC,4)
#define HEATER_ACTIVE HEATER_PORT = 1
#define HEATER_IDLE HEATER_PORT = 0


#define	SPEAKER_PORT REGISTER_BIT(PORTD,4)
#define SPEAKER_IDLE SPEAKER_PORT = 0
#define SPEAKER_ACTIVE SPEAKER_PORT = 1

//I array size, total samples -1
#define TABLE_SIZE 41
//PID
#define P_val 30 //30
#define I_val 3 //3
#define D_val 50 //50

//max heater power
#define MAX_POWER 150
//how long to show the target temperature when encoder is rotated
#define TEMP_SHOW_TIME 4 //divided by 2
//how long to wait till saving new value to eeprom
#define TEMP_SAVE_TIME 10 //seconds divided by 2

//how long to show the saved sign on display
#define TEMP_SIGN_TIME 2 //seconds divided by 2
#define startmov 50 //disconnected sign tip

#define I_memory 40 //samples

#define OVERSAMPLING 4

//encoder look up table
const int8_t enc_LUT[17] ={0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
static uint8_t	old_AB = 0,
				old_click=0;

//volatiles
volatile int32_t	PID=0,
					encoder_value=0,
					err=0,
					err_old=0,
					P_err=0,
					D_err=0,
					mean_I_err[I_memory],
					mean_I_error=0.0,
					I_err=0.0;
					
volatile uint16_t	MAXIMUM_DUTY=1900;//1945 of 2048, 95% 1.3184ms left to do ADC etc. 20E6/(256*(2048-1945))=758Hz -> 1.3184ms
								
volatile uint16_t	seconds=0,
					temperature_adc[OVERSAMPLING],
					temperature_adc1=0,
					timer_1s=0,
					duty=0,													
					TARGET_TEMPERATURE=0,
					startxmov=startmov,
					temperature_adc_mean=0; 
				
volatile uint8_t	cmd_pulse=0,
					disconnected=0,
					keep_heater_off=0,
					clicked=0,
					show_temp=0,
					show_time=0,
					save_time=0,
					saved_sign=0,
					temp_value=0,
					speaker_now=0;
	

volatile long double 	temperature_value=0.0;
					


volatile uint16_t iji=0;
volatile double m=0.0;



//volatiles	
uint16_t	current_adc=0,
			current_adc1=0,
			voltage_adc=0,
			voltage_adc1=0,
			saved_temp=0,
			saved_temp_temp=0;
			
uint8_t		saved=1,
			times=0,
			spi_test=0,
			WRONG_AMPLIFICATION=0;
					
double	current_value=0.0,
		voltage_value=0.0,
		power_value=0.0,
		power_valuegah=0.0;
		
long double temperature_valuegah=0.0;		
uint16_t TARGET_TEMPERATUREgah=0;		

#include "Global_functions.h"
#include "Thermotable.h"

void u8g_setup() {
	// SCK: PORTB, Bit 5 --> PN(1,5)
	// MOSI: PORTB, Bit 3 --> PN(1,3)
	// CS: PD6 ->PN(3,6)
	// A0: PD7 --> PN(3,7) //DC!!
	// RST: PB0,  --> PN(1,0)
	// Arguments for u8g_InitHWSPI are: CS, A0, Reset
	//this one is SSD1306
		
	u8g_InitHWSPI(&u8g, &u8g_dev_ssd1306_128x64_hw_spi,PN(3,6),PN(3,7), PN(1,0));
	//u8g_InitHWSPI(&u8g, &u8g_dev_sh1106_128x64_hw_spi,U8G_PIN_NONE,PN(1,0), PN(3,7));
	// u8g_SetRot180(&u8g); //flip screen, if required
	//u8g_SetColorIndex(&u8g, 1);
}


void setup()
{	
	//pd1=encoders button (input), pd2= encoders B (input), pd3= encoders A (input),
	//pd4=speaker(out), pd5= instrumentation amp SPI select (active low)(out),
	//pd6=OLED displays SPI select(active low)(out), pd7=display D/C(out)
	DDRD=0b11110000;
	PORTD=0b01100000; 
		
	//pb0=reset display(out), pb1= solder tip PWM (out), pb2=ss(out and high),
	//pb3 =MOSI(out), pb4= MISO(in), pb5=sck(out)
	DDRB= 0b00101111;
	PORTB=0b00000100;
	
	//PC1=current in (in), pc2=vcc 24v (6.2/(51+6.2)) resistance divider (in)
	//PC4= non-volatile rheostat MCP4022 UD(out), PC5= non-volatile rheostat CS(out),
	DDRC =0b00110000;
	PORTC=0b00000000; //output 0 and input tri-state
	
	//ADC6 is thermal, ADC1 is current value, ADC2 is input voltage (24v)  
	ADMUX=0b01000110; //AVcc reference, 1nF cap in Aref pin, ADC6 thermal
	ADCSRA=0b11000111; //with 128 clk division from 20MHz is 156.25khz, should be between 50-200khz to get max resolution
	
	//ICR1 is the PWM counter max value, overflow(interrupt) at top, set (new) value in bot
	ICR1  = 2048; //max value of the counter
	OCR1A = 0; //zero at start
	TCCR1A = 0b10000010;  //fast PWM (mode 1110, ICR1 controls max period)
	TCCR1B = 0b00011100;  //prescale by 256 = total freq is 20MHz/(256*2048)= 38.14Hz < (100Hz/2) OK!
	TIMSK1 = 0b00000010;  //compare match A interrupts, then OCR1A=TCNT1 reads current temperature and calculates new value to be set for the PWM
		
	//OCR0A timer is for time calculation
	//output is 20MHz/(256*125)=1.6ms tick, 1sec every 625ticks
	OCR0A = 124; //runs to value 0:124, to get exact ms
	TCCR0A=0b00000010; //OCR0A compare match
	TCCR0B = 0b10000100; //force compare match A, prescaler to 256
	TIMSK0= 0b00000010; //masked compare match A
	//TCNT0 = 0x00; //set timer to 0
	
	//external interrupts
	EICRA=0b00000101; //any logical change generates interrupts
	EIMSK=0b00000011; //mask register
	//EIFR=0b00000000; //set interrupt flags to zero
	
	///PCINT17, pd1 or encoder button interrupt mask
	PCICR=0b00000100; //level change causes interrupt pins PCINT24-16
	PCMSK2=0b00000010; //PCINT17 masked
	
}

int main(void){
	setup();
	
	/*
	for (int i=0;i< 2;i++){
		delay_ms(1000); //wait 1 sec
		
	}
	*/
	//for hardware spi to work, have to redo these here
	DDRB = 0b00101111; //DDRB= 0b00101111;
	PORTB = 0b00000101; //output 0 and input tri-state
	//Hardware spi enabled, clk/16 mode, master mode, LMP8358 will require min 200ns for clk period
	SPCR=0b01010010; //0b01010001; //rising edge sampling
	SPSR=0b00000000;
	RWLMP8358(); //sets right values to the instrumentation amplifier
	//SPCR=0b01010000; //4x
	u8g_setup();
	
	
	//read eeprom at start
	int justtobesure3=0;
	while(1){
		justtobesure3++;
		if(eeprom_is_ready()==1) {
			//read values
			TARGET_TEMPERATURE=eeprom_read_word(&saved_temperature);
			break;
		}
		if (justtobesure3>=100){ //failed to read eeprom
			keep_heater_off=1;
			break;
		}
	}
	
	if(TARGET_TEMPERATURE <= 0 || TARGET_TEMPERATURE > 500){
		TARGET_TEMPERATURE=350;
	}
	
	for (int i=0;i< I_memory-1;i++){
		mean_I_err[i]=0;
	}
	

	cli(); //disable interrupts //was at beginning of system setup, start PWM can be glitchy if not next to each other
	sei(); //enable interrupts
	
	long double modulo=0;	
	char current1[6]={0,0,0,0,0};
	char voltage1[6]={0,0,0,0,0};
	char power1[6]={0,0,0,0,0};
	char temp1[6]={0,0,0,0,0};
	//char temp11[6]={0,0,0,0,0};
	char temp2[6]={0,0,0,0,0};
	//char temp21[6]={0,0,0,0,0};
		
	//char spi_testing[6]={0,0,0,0,0};	
	//intToStr(spi_test, spi_testing, 0);		
		
	char wtf[6]={0,0,0,0,0};
	char wtf2[6]={0,0,0,0,0};
	char wtf3[6]={0,0,0,0,0};
	
    while(1)
    {
		if(cmd_pulse==1){
			delay_ms(10);
			cmd_pulse=0;
						
			ADMUX=0b01000001; //check current ADC1
			ADCSRA |= (1 << ADSC);	 // start an ADC conversion
			while(ADCSRA & _BV(ADSC));    // wait until conversion is complete
			current_adc1=ADC;
			ADMUX=0b01000001; //check current ADC1
			ADCSRA |= (1 << ADSC);	 // start an ADC conversion
			while(ADCSRA & _BV(ADSC));    // wait until conversion is complete
			current_adc=(ADC+current_adc1)/2;
			//with ACS712 20 amp version
			//y=0.1v/A+2.5v, ADC value is 1024/5v=204.8/v
			//(adc-(1024/5v*2.5))*(1/(0.1*1024/5v))
			current_value=abs(current_adc-512)*0.048828125;//accuracy is 49mA  513
			if(current_value < 0 ){
				current_value=(double)0.0;
			}
			
			if (current_value > 8 && current_value < 20){
				current_value=(double)0.0;
				//over going value causes max duty cycle to be recalculated
				//MAXIMUM_DUTY=2048*0.9*((9-current_value)/current_value); 	
			}
			if (current_value >= 20){
				//short circuit
				keep_heater_off=1;	
			}
			ADMUX=0b01000010; //check input voltage ADC2
			ADCSRA |= (1 << ADSC);	 // start an ADC conversion
			while(ADCSRA & _BV(ADSC));    // wait until conversion is complete
			voltage_adc1=ADC;
			ADMUX=0b01000010; //check input voltage ADC2
			ADCSRA |= (1 << ADSC);	 // start an ADC conversion
			while(ADCSRA & _BV(ADSC));    // wait until conversion is complete
			voltage_adc=(ADC+voltage_adc1)/2;
			
			// voltage division from input with 6.2k/(6.2k+51k)
			// x*0.108391608391608*1024/5 =  22.198601398601397
			// 1/22.198601398601397=0.045047883064516
			voltage_value=(double)(voltage_adc*0.045047883064516);
			if(voltage_value>60){ //goodbye MOSFET
				voltage_value=(double)0.0;
			}
			
			power_value=(double)voltage_value*current_value;
			
			if(power_value>MAX_POWER+50){
				power_value=(double)0.0;	
			}
			if(clicked==1){ //save with click
				clicked=0;
				save_eeprom();
				saved_sign=TEMP_SIGN_TIME;
			}
			
			if( save_time == 0 && saved==0){
				saved=1;
				int justtobesure4=0;
				while(1){
					justtobesure4++;
					if(eeprom_is_ready()==1) {
						//read values
						saved_temp_temp=eeprom_read_word(&saved_temperature);
						break;
					}
					if(justtobesure4>=100){ //failed to read eeprom
						break;
					}
				}
				
				if(  TARGET_TEMPERATURE >= saved_temp_temp +5 || TARGET_TEMPERATURE+5 <= saved_temp_temp  ){ 
					save_eeprom();
					saved_sign=TEMP_SIGN_TIME;
					
				}
				
			}
			if( save_time > 0){
				save_time--;
				saved=0;
			}
			
			if( show_time>0){
				
				show_time--;
			}
			if(saved_sign >0){
				saved_sign--;
				
			}

			//intToStr(current_adc, current1, 3);						
			ftoa(current_value, current1, 3);
			ftoa(voltage_value, voltage1, 1);
					
			intToStr(temperature_adc_mean, wtf, 0);	
			intToStr(iji, wtf2, 0);
			ftoa(m, wtf3, 3);
					
		}
		
		
		memset(&temp2[0], 0, sizeof( temp2));
		TARGET_TEMPERATUREgah=TARGET_TEMPERATURE;
		intToStr(TARGET_TEMPERATUREgah, temp2, 0);
		
		memset(&temp1[0], 0, sizeof( temp1));
		temperature_valuegah=round(temperature_value);
		
		modulo=(uint16_t)temperature_valuegah % 5;
		if(modulo >=3){
			temperature_valuegah+=5-modulo;
		}
		else if(modulo <3){
			temperature_valuegah-=modulo;
		}

		ftoa(temperature_valuegah, temp1, 0);
		
		memset(& power1[0], 0, sizeof(power1));
		power_valuegah=power_value;
		ftoa(power_valuegah, power1, 0);
			
		

		u8g_FirstPage(&u8g);
		do{	//x,y
			//LCD_clear();
			//u8g_Delay(50);
			//disconnected_sign_moving();
						
			if(disconnected==1){
			//please check connection
				u8g_SetFont(&u8g, u8g_font_fur25);
				//LCD_clear();
				//u8g_Delay(50);
				//disconnected_sign_moving();
				u8g_DrawStr(&u8g, 19, 41,"check");
				u8g_DrawStr(&u8g, 0, 61,"connect");
			}
			else if(disconnected==0){
				
				if(saved_sign >0){ 
					save_sign();
				}
				
				u8g_SetFont(&u8g, u8g_font_7x13);
				//u8g_DrawStr(&u8g, 58, 13,current1);
				//u8g_DrawStr(&u8g, 8, 13,wtf);
				//	u8g_DrawStr(&u8g, 50, 13,wtf2);
				//u8g_DrawStr(&u8g, 8, 13,voltage1);
				//u8g_DrawStr(&u8g, 18, 13,"target:");
				//u8g_DrawStr(&u8g, 68, 13,temp2);
				
				u8g_SetFont(&u8g, u8g_font_fur30n); //54x30 pixels
				
				if( show_time>0){
					show_temp=0;					
					//strncpy(temp2,temp1,6);			
					if(TARGET_TEMPERATUREgah>= 100){
						
						u8g_DrawStr(&u8g, 18, 49,  temp2);				
						//delay_ms(30);
					}
					else if(TARGET_TEMPERATUREgah < 100 && TARGET_TEMPERATUREgah >= 10){
					
						u8g_DrawStr(&u8g, 42, 49,  temp2);
						//delay_ms(30);
					}
					else if(TARGET_TEMPERATUREgah < 10){
						u8g_DrawStr(&u8g, 66, 49,  temp2);
					}	
				}		
							
				else if(show_temp==0 && show_time==0){
					
					if(temperature_valuegah >= 100 ){//&& temperature_value < 1000){	
						u8g_DrawStr(&u8g, 18, 49,  temp1);
						//delay_ms(30);	
					}
					else if(temperature_valuegah < 100 && temperature_valuegah >= 10){
						//u8g_SetDefaultBackgroundColor(&u8g);
						//u8g_DrawBox(&u8g,18,19,25,30); //clears first digit
						//u8g_SetDefaultForegroundColor(&u8g);
						//cli(); //disable interrupts //was at beginning of system setup, start PWM can be glitchy if not next to each other
						u8g_DrawStr(&u8g, 42, 49,  temp1);
						//delay_ms(30);					
						//sei(); //enable interrupts
					}
					else if(temperature_valuegah < 10){
						u8g_DrawStr(&u8g, 66, 49,  temp1);
					}
					//u8g_SetFont(&u8g, u8g_font_7x13);
					//u8g_DrawStr(&u8g, 8, 13,wtf);
			
					
					//u8g_DrawStr(&u8g, 30, 13,wtf2);
					//u8g_DrawStr(&u8g, 60, 13,wtf3);
					
				}
				//u8g_SetFont(&u8g, u8g_font_7x13);
				//u8g_DrawStr(&u8g, 60, 13,spi_testing);	
				
					
				u8g_SetFont(&u8g, u8g_font_fur25);
				u8g_DrawStr(&u8g, 90, 48,  "°C");
			
				u8g_SetFont(&u8g, u8g_font_7x13);	
				power_scale();
				if(power_valuegah < 10){
					u8g_DrawStr(&u8g, 113, 63, power1);
				}
				else if(power_valuegah < 100 && power_valuegah >= 10){
					//u8g_SetDefaultBackgroundColor(&u8g);
					//u8g_DrawBox(&u8g,99,50,7,13); //clears first digit
					//u8g_SetDefaultForegroundColor(&u8g);
					u8g_DrawStr(&u8g, 106, 63, power1);
					
				}
				else if( power_valuegah >= 100){
				
					u8g_DrawStr(&u8g, 99, 63, power1);
					
				}
				u8g_DrawStr(&u8g, 121, 63, "w");	
				}
			
	
		} while ( u8g_NextPage(&u8g) );
		u8g_Delay(330);
	}
	
		
				
}

//OCR1A is double buffered so can set a new value anytime, ICR1 is not double buffered
ISR(TIMER1_COMPA_vect){ //Output Compare A Match Interrupt
HEATER_IDLE; //don't use unless testing
delay_us(800); //wait at least xxx us for thermocouple voltage to get normal
//checking temp and adjusting PWM takes around 800us
ADMUX=0b01000110; //AVcc reference, 1nF cap in Aref pin, ADC6 thermal
temp_value=0;
temperature_adc_mean=0;
while(temp_value < OVERSAMPLING){
	delay_us(1);	
	ADCSRA |= (1 << ADSC);	 //start an ADC conversion
	while(ADCSRA & _BV(ADSC));    //wait until conversion is complete
	temperature_adc_mean+=ADC;
	temp_value++;
}
temperature_adc_mean=round(temperature_adc_mean/OVERSAMPLING);

if( keep_heater_off==1 || temperature_adc_mean > 1000 || WRONG_AMPLIFICATION==1){
	duty=0;
	disconnected=1;
	OCR1A=duty;
}
else if( keep_heater_off==0 && WRONG_AMPLIFICATION==0 ){ 
	disconnected=0;
	//calculate temperature	
	temperature_value = dsp_lookup(IrRh500amp,temperature_adc_mean);
		
	//temperature_value =abs(temperature_value);
	//temperature_value+=25; //due offset voltage temperature drift compensation in op amp 
	
	//PID	
	err_old=err; //save old value
	err=(int32_t)TARGET_TEMPERATURE-temperature_value; //calculate the temperature difference
	P_err=err; //Proportional is directly the difference times P_val (bang bang value)
	
	mean_I_err[I_memory-1]=(int32_t)err;
	mean_I_error=0;
	for (int i=0;i< I_memory-1;i++){
		mean_I_err[i]=(int32_t)mean_I_err[i+1];		
		mean_I_error+=(int32_t)mean_I_err[i];
	}	
	mean_I_error+=(int32_t)err;
		
	I_err=(int32_t)mean_I_error;
	
	//I_err+=err_old; //integral is all the previous values summed (this will cause it to overshoot a lot)
	D_err=(int32_t)err-err_old;	//derivative is the difference between earlier and this one (helps lower overshooting)
	PID=(int32_t)P_val*P_err + I_val*I_err + D_val*D_err;
	if(PID > MAXIMUM_DUTY){ //95% duty cycle max, recalculate here if current drain is over 9A
		PID=MAXIMUM_DUTY;
	}
	else if(PID < 0){ 
		PID=0;
	} 
	duty=(uint16_t)PID;
	OCR1A=duty;

	if((temperature_value <= TARGET_TEMPERATURE+5 && temperature_value +5 >= TARGET_TEMPERATURE ) && speaker_now==0){
		speaker_now=1;
		SPEAKER_ACTIVE;
		delay_ms(3);
		SPEAKER_IDLE;
		delay_ms(3);
		SPEAKER_ACTIVE;
		delay_ms(3);
		SPEAKER_IDLE;		
	}
}

HEATER_ACTIVE;
}

ISR(TIMER0_COMPA_vect) { //1.6ms tick 1s with 625 rounds for 1sec

timer_1s++;
if(timer_1s == 313){
	cmd_pulse=1; //time to refresh display
}

if(timer_1s >= 625){ //1 seconds has passed
	cmd_pulse=1; //time to refresh display
	timer_1s=0; //zero the timer if 1 second has been reached
	seconds++; //increase if under 59 sec
	if(seconds > 59){
		seconds=0; //zero if
		//one minute has passed here
	}
}
}

//external interrupts for encoder
ISR(INT0_vect) { //encoder A
old_AB <<= 2;                //remember previous state
old_AB |= ( ENCA | ENCB<<1 );  //add current states
TARGET_TEMPERATURE+=enc_LUT[( old_AB & 0x0f )];
if(TARGET_TEMPERATURE > 1000 || TARGET_TEMPERATURE <0 ){ //if negative
	TARGET_TEMPERATURE=0;
}
else if(TARGET_TEMPERATURE <= 1000){
	show_temp=1;
	show_time=TEMP_SHOW_TIME;
	save_time=TEMP_SAVE_TIME;
}

}

ISR(INT1_vect) { //encoder B
old_AB <<= 2;                //remember previous state
old_AB |= ( ENCA | ENCB<<1 );  //add current states
//old_AB |=( ENCA>>1 | ENCB>>3 );  //add current states
TARGET_TEMPERATURE+=enc_LUT[( old_AB & 0x0f )];
if(TARGET_TEMPERATURE > 1000 || TARGET_TEMPERATURE <0){ //if negative
	TARGET_TEMPERATURE=0;
}	
else if(TARGET_TEMPERATURE <= 1000){
show_temp=1;
show_time=TEMP_SHOW_TIME;
save_time=TEMP_SAVE_TIME;
}

}
//clicker interrupt
ISR(PCINT2_vect){
old_click <<= 1; 
old_click |=  ENCCLK;  //add current state
	if(old_click&0x02){ //if pulled down
		 //clicked
		 clicked=1;
		 show_temp=1;
		 show_time=TEMP_SHOW_TIME;
	}	          
}