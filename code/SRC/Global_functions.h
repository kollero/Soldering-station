
void delay_us(int us){
	for (int i=0; i < us; i++){
		_delay_us(1);
	}	
}

void delay_ms(int ms){
	for (int i=0; i < ms; i++){
		_delay_ms(1);
	}	
}

uint8_t TransactSPI(uint8_t data){
    SPDR = data;
    while(!(SPSR & (1<<SPIF)))
    {
    //do nothing, loop itself is a jump (2 instr. cycles?)
    }
    return SPDR;
}

void power_scale(void){
	float scale=0;
	uint8_t startx=10;
	uint8_t starty=64;
	uint8_t somex=10;
	scale = power_value/MAX_POWER;
	if (scale > 1){
		scale=1;
	}	
	for(int i=0; i <= 8; i++){
		if(i==0 || i== 4 || i== 8) {
			 u8g_DrawLine(&u8g,startx+i*somex,starty,startx+i*somex,starty-4);
		}
		if(i==1 || i== 3 || i== 5 || i== 7) {
			 u8g_DrawLine(&u8g,startx+i*somex,starty,startx+i*somex,starty-2);
		}
		if(i==2 || i== 6) {
			 u8g_DrawLine(&u8g,startx+i*somex,starty,startx+i*somex,starty-3);
		}	
	}
	u8g_DrawBox(&u8g,startx,starty-7,81*scale,2);
}

void RWLMP8358(){
INSTR_ACTIVE;
_delay_us(1);
//start value with x gain and min bandwidth
uint8_t start_val_LMP8358_1=0b00000000;
//0b00000110 for 1000amp,0b00000101 for 500amp, 0b00000100 for 200amp, 0b00000011 for 100amp, 
uint8_t start_val_LMP8358_2=0b00000101; 
TransactSPI(start_val_LMP8358_1);
TransactSPI(start_val_LMP8358_2);
TransactSPI(start_val_LMP8358_1);
spi_test=TransactSPI(start_val_LMP8358_2);
_delay_us(1);
INSTR_IDLE;
_delay_us(1);
if (spi_test!=start_val_LMP8358_2){
	WRONG_AMPLIFICATION=1;
}
}

//function that calculates current temp value from adc readout
double dsp_lookup(uint16_t (*table)[2], uint16_t x){
	uint16_t i=0;
	double m=0.0;
	
	//x=round((gain*x)/100); //if gain is changed to something else than the right table
	while(i < TABLE_SIZE ){  
		//find the point where adc value is smaller than table value
		if(x <= table[i][0]){
			break;
		}
		i++;
	}
	if ( i == TABLE_SIZE ){   //make sure the point isn't past the end of the table
		return table[i-1][1];
	}
	if ( i == 0 ){  //make sure the point isn't before the beginning of the table
		return table[i][1];
	}
	m = (double)(table[i][1] - table[i-1][1]) / ( table[i][0] - table[i-1][0]); //calculate the slope 87,60C
	return (double)(m * (x - table[i-1][0])) + table[i-1][1]; //this is the solution to the point slope formula
}

void save_eeprom(void){
	int justtobesure2=0;
	//write eeprom at start
	while(1){
		justtobesure2++;
		if(eeprom_is_ready()==1) {
			//update values to current time, so only write if they're not the same as before
			eeprom_update_word(&saved_temperature, TARGET_TEMPERATURE);
			break;
		}
		if (justtobesure2>=100){ //failed to write eeprom
			break;
		}
	}
}


void save_sign(void){
	uint8_t startx2=108;
	uint8_t starty2=1;
	
	u8g_DrawVLine(&u8g,startx2+5,starty2,3);
	u8g_DrawHLine(&u8g,startx2+2,starty2+3,7);
	
	u8g_DrawPixel(&u8g,startx2+3,starty2+4);
	u8g_DrawPixel(&u8g,startx2+7,starty2+4);
	u8g_DrawPixel(&u8g,startx2+4,starty2+5);
	u8g_DrawPixel(&u8g,startx2+6,starty2+5);
	u8g_DrawPixel(&u8g,startx2+5,starty2+6);
	
	u8g_DrawHLine(&u8g,startx2+4,starty2+8,3);
	u8g_DrawHLine(&u8g,startx2+2,starty2+9,7);
	u8g_DrawHLine(&u8g,startx2,starty2+10,4);
	u8g_DrawHLine(&u8g,startx2+7,starty2+10,4);
	u8g_DrawHLine(&u8g,startx2+2,starty2+11,7);
	u8g_DrawHLine(&u8g,startx2+4,starty2+12,3);
	
	
}

void disconnected_sign_moving(void){
	uint16_t startx21=10;
	uint16_t starty21=16;	
	
	//u8g_Delay(10);
	times++;
	if(startxmov >= 15 && times > 20){
		startxmov-=15;
		times=0;
	}
	if(startxmov < 15 ){
		startxmov=startmov;
	}
	
	u8g_DrawVLine(&u8g,startx21,starty21,42);
	u8g_DrawVLine(&u8g,startx21+1,starty21,42);
	u8g_DrawVLine(&u8g,startx21+2,starty21,42);
	u8g_DrawVLine(&u8g,startx21+3,starty21,42);
	
	u8g_DrawHLine(&u8g,startx21+startxmov+15,starty21+11,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+14,starty21+12,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+13,starty21+13,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+12,starty21+14,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+11,starty21+15,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+10,starty21+16,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+9,starty21+17,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+8,starty21+18,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+7,starty21+19,24);
	u8g_DrawHLine(&u8g,startx21+startxmov+6,starty21+20,25);
	u8g_DrawHLine(&u8g,startx21+startxmov+6,starty21+21,25);
	u8g_DrawHLine(&u8g,startx21+startxmov+6,starty21+22,25);	
	u8g_DrawHLine(&u8g,startx21+startxmov+7,starty21+23,24);
	u8g_DrawHLine(&u8g,startx21+startxmov+8,starty21+24,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+9,starty21+25,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+10,starty21+26,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+11,starty21+27,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+12,starty21+28,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+13,starty21+29,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+14,starty21+30,6);
	u8g_DrawHLine(&u8g,startx21+startxmov+15,starty21+31,6);
	
	
}


void LCD_clear(void){
	
	u8g_SetDefaultBackgroundColor(&u8g);
	u8g_DrawBox(&u8g,0,0,132,64); //clears the screen
	u8g_SetDefaultForegroundColor(&u8g);
	//u8g_SetFont(&u8g, u8g_font_7x13);
		
}




// reverses a string 'str' of length 'len'
void reverse(char *str, int len){
	int i=0, j=len-1, temp;
	while (i<j){
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++; j--;
	}
}
// Converts a given integer x to string str[].  d is the number
// of digits required in output. If d is more than the number
// of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d) {

	int i = 0;
	while (x) {
		str[i++] = (x%10) + '0';
		x = x/10;
	}
	// If number of digits required is more, then
	// add 0s at the beginning
	while (i < d)
	str[i++] = '0';
	reverse(str, i);
	str[i] = '\0';
	
	
	return i;
}
// Converts a floating point number to string.
void ftoa(float n, char *res, int afterpoint) {
	// Extract integer part
	int ipart = (int)n;
	// Extract floating part
	float fpart =(float) (n - (float)ipart);
	// convert integer part to string
	//int i = intToStr(ipart, res, 0);
	int i = intToStr(ipart, res, 1); //forces to show 0 at the beginning
	// check for display option after point
	if (afterpoint != 0){
		res[i] = '.';  // add dot
		// Get the value of fraction part upto given no.
		// of points after dot. The third parameter is needed
		// to handle cases like 233.007
		fpart = fpart * pow(10, afterpoint);
		intToStr((int)fpart, res + i + 1, afterpoint);
	}
}