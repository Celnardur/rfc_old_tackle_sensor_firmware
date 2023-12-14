#include <system.h>
#include "config4620.h"
#include <stdlib.h>
#include "serial1.h"

// v5.0	2/15/2010
// parameters that can affect sensitivity are:
// samples, threshold, mag, jerk, qty, alpha, points, max_points
//
// samples is the # of measurements to make after a level detection
// threshold is the value considered to be a tackle
// mag is the vector magnitude of the X/Y acceleration
// jerk is the computed change in acceleration from one sample to the next
// qty is the value that is compared to the threshold
// qty can be assigned the value of mag or jerk
// v6.0 2/24/2010
// alpha is the weight given the exponential moving average of magnitude in %
// max_points is the number of successive measurements that must exceed threshold for pain
// v6.6 3/19/2010
// tackle is based on straight X/Y level detection only, independent of measurements

#pragma CLOCK_FREQ 16000000

#include "accel.h"

// global variables
		 bit TEAM = 1;
volatile bit BLUE_GOLD@PORTD.RD7;	// sets team color ?
volatile bit T2on@T2CON.TMR2ON;
volatile bit tmr2if@PIR1.TMR2IF;
volatile bit ADC_ON@ADCON0.ADON;
volatile bit adgo@ADCON0.GO_DONE;
volatile bit rbpu@INTCON2.RBPU;		// low enables weak pullups on B
volatile bit RED@PORTC.RC2;			// PAIN light on ECCP1
volatile bit GREEN@PORTC.RC1;		// STATUS light on CCP2
volatile bit BLUE@PORTC.RC0;		// TACKLE light on C.0
volatile bit STOP@LATE.LATE2;		// output signal to stop action when tackled
volatile bit gie@INTCON.GIE;
volatile bit t0ie@INTCON.T0IE;
volatile bit t0if@INTCON.T0IF;
volatile bit t1if@PIR1.TMR1IF;
volatile bit t1ie@PIE1.TMR1IE;
volatile bit tmr1on@T1CON.TMR1ON;
volatile bit peie@INTCON.PEIE;
volatile bit tmr0on@INTCON.TMR0ON;
volatile bit eccpase@ECCP1AS.ECCPASE;
volatile bit marker@PORTD.6;		// put a timing mark on this output
volatile bit jmpr_in@PORTD.5;		// jumper selectable input
volatile bit jmpr_hi@PORTD.6;
volatile bit jmpr_lo@PORTD.4;
volatile bit a_INT2@PORTB.1;

unsigned char pain, max_pain, volts, pulse, throb;
#define low_bat	185			// battery cutoff voltage is 7.2 for 6-cell rechargeable pack (2.40V at ADC input)
#define toggle marker = 1; nop(); marker = 0;

const char tackle_time = 2;	// # of seconds to hold tackle light on and signal active

void init_ports()
{
	rbpu = 1;				// disable port B pullup resistors
	trise = 00001011B;		// enable STOP output pin
	trisd = 10111110B;		// D0 is CS for accel. (see accel.h)
	trisc = 10011000B;		// C7 is RX (input), C6 is TX (output), C5 is MOSI (output), C4 is MISO (input)
							// C0 and C1 are PWM outputs, C2 is LED output
							// don't enable PWM outputs until PWM is initialized
	trisb = 11111111B;		// port B is all inputs
	cmcon = 0x07;			// comparators off
	portc = 0x00;			// LED outputs off
}

void init_ADC()
{
	adcon1 = 0x0D;			// AN0, AN1 are analog inputs, Vdd/Vss refs.
	trisa = 00000011B;		//
	adcon2 = 00110010B;		// 16 TAD, FOSC/32
	ADC_ON = true;			// turn A/D on
}

char check_battery()
{
	adcon0 = 00000001B;		// sets channel 0
	delay_us(64);
	adgo = true;
	while(adgo);
	return adresh;
}

void init_PWM()
{
	trisc |= 00000111B;		// see datasheet paragraph 16.4.9
	pr2 = 0xFF;				// sets PWM frequency
	ccp1con = 00001100B;	// set PWM on ECCP1, timer 2
	ccpr1l = 0x00;			// sets PWM duty cycle for RED channel
	tmr2if = 0;				// clear int flag
	t2con = 00000001B;		// timer2 1:1 postscale, 1:4 prescale
	T2on = true;			// turn on timer 2
	while(!tmr2if);			// wait for overflow
	trisc &= 11111000B;		// enable PWM output ping
	eccpase = 0;
	ccpr2l = 0x00;			// sets PWM duty cycle for GREEN channel
	ccp2con = 00001111B;	// set PWM on CCP2, timer 2
}

init_timer()
{
	t0con = 11000101B;		// 8- bit timer 0, 64x prescaler
	tmr0on = true;
	t1con = 10110000B;		// prescaler = 1:8, 16-bit read/write
	tmr1on = true;
}

void interrupt()
{
	if(t0if)				// controls PWM fade-out
	{
		if(pain)	
		pain--;
		tmr0h = 255 - pulse;
		if(pain == 0)
		{
		if(throb > 0)
		{	
			throb--;
			if(max_pain > 4)
			max_pain -= 4;	// diminshing intensity with each throb
			pain = max_pain;
		}

		}
		
		ccpr1l = pain;		// sets PWM duty cycle for RED channel
		t0if = 0;
	}

	if(t1if)				// controls throb rate
	{
/*
		if(max_pain)
		max_pain--;
		pain = max_pain;
*/
		t1if = 0;
	}
	if(tmr2if)
	{
		tmr2if = 0;
	}
}

void lamp_test()
{
	latc = 0x07;
	delay_s(1);
	latc = 0x00;
	return;
}

void acc_cal(void)
{
	 char data;
	 char done;

	printf("\r\n Calibration of X");	
	do
	{
		done = calibrate(0, 0);
		while(!acc_DRDY);
		data = read_short(0); // read accel value
		p_int(data);
		printf("\r\n");
	}	
	while(done == 0);

	printf("\r\n Calibration of Y");

	do
	{
		done = calibrate(1, 0);
		while(!acc_DRDY);
		data = read_short(2); // read accel value
		p_int(data);
		printf("\r\n");
	}	
	while(done == 0);


	printf("\r\n Calibration of Z");	
	
	do
	{
		done = calibrate(2, 64);	// null gravity
		while(!acc_DRDY);
		data = read_short(4); // read accel value
		p_int(data);
		printf("\r\n");
	}	

	while(done == 0);
	printf("\r\n");

}
		
void clear_ints()
{
	acc_reg_write(0x17, 0x03);
	acc_reg_write(0x17, 0x00);	// clear and enable both interrupts
}

void clear_int2()
{
	acc_reg_write(0x17, 0x02);
	acc_reg_write(0x17, 0x00);  //clear interrupt flag
}

void tackle()
{
	gie = false;	// disable interrupts
	ccp1con = 0;
	ccp2con = 0;	// disable PWMs
	STOP = true;	// signal player to stop moving
	latc = 0;		// all LEDs off
	nop();
	if(TEAM) BLUE = true;
	else { GREEN = true; RED = true; }
	delay_s(tackle_time);
	latc = 0x00;
	init_PWM();
//	t0if = 0;		// clear interrupt flags
//	t1if = 0;
//	tmr2if = 0;
	STOP = false;	// okay to move again
	gie = true;
}

// ********************************************************************************************		
void main(void)
{
	TEAM = 1;
// ********************************************************************************************		
	unsigned short bat_test;	// used to sum battery volts results for averaging
	unsigned char bat_avg = 0;	// counter for averaging battery volts
	signed short x, y;			// can be up to +/-512
	
	init_ports();
	max_pain = 0x00;		// initial values
	pain = 0x00;
	pulse = 0;
	throb = 0;
	bat_test = check_battery();	

// ********************************************************************************************	
	
	 char data;
	 char done;
	 char i, j;
	 short xave = 0;
	 short yave = 0;
	 unsigned short mag;
	 unsigned short mags = 0;
	 signed short jerk;
	 unsigned char points = 0;
	 unsigned short qty = 0;
	 unsigned char hurt = 12;	// default pain value on startup
	 const char alpha = 95;	// weight given to previous average magnitude
	 const char threshold = 35;
	 const char peak = 80;
	 	
	osccon |= 0b01100000; // 4 mhz
	osctune.6 = 1;  // 4xPLL
	delay_ms(50);

	set_active_usart(1);  // usb usart
	serial_init(34);  // 115200 at 16 MHz
	ACC_spi_init(2); // init accel spi

	ACC_init(0); // init accelerometer measurement mode
	acc_cal();		// cal 
	
// ********************************************************************************************		
	
	portd = 01000000B;	// set jumper select pins hi & lo
	nop(); nop();
	if(!jmpr_in)		// test jumper pin
	{
		gie = true;			// enable interrupts for pain indicator
		t0ie = true;
		t1ie = true;
//		peie = true;
		printf("PAIN active");
	}
			
	else printf("PAIN disabled");
	printf("\r\n");

	lamp_test();			// white for 1 sec.
	init_ADC();
	init_PWM();
	init_timer();
	ACC_init(1);	// level detect mode
	clear_ints();
	
// ********************************************************************************************		
	for(;;)
	{
		do
		{
			x = get_accel(0);	// get measurements
			y = get_accel(1);
			xave = smooth(x, xave, alpha);
			yave = smooth(y, yave, alpha);
			mag = mag_accel(x, y);
			mags = mag_accel(xave, yave);
			
			if(mag > peak)
			{
				points++;
				if(mag > qty) qty = mag;
			}
		}
		while(mags > threshold);
		
			if(points)
			{
				throb += (qty / 37);
				hurt = throb;	// save the value
				pain = 255; max_pain =255;
				printf("\r\nqty="); p_dec(qty);
				printf(", points="); p_dec(points);
				printf(", throb="); p_dec(throb);
				points = 0;
				qty = 0;
			}
		
			if(a_INT2)			// check for level detect interrupt pin set
			{
				tackle();
				clear_ints();
				max_pain = 255;
				pain = 255;
				throb = hurt + 2;	// restore pain to previous value, plus a little extra
			}

		
// ********************************************************************************************		

		bat_avg--;
		if(bat_avg) 
			bat_test += check_battery();	//sum the samples for averaging
		else
		{
			volts = bat_test / 256;	//get the last average value
			bat_test = 0;
		}
		if(volts < low_bat) volts = 0;	//turn off green lights if low battery
		ccpr2l = volts / 16;		// dims running lights proportional to battery volts

	}	
		
// ********************************************************************************************
}

	