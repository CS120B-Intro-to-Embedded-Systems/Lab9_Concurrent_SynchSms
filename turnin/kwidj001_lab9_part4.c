/*	Author: lab
 *  Partner(s) Name:
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
// 0.954 hz is lowest frequency possible with this function,
// based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound
void set_PWM(double frequency) {
	static double current_frequency; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR0B &= 0x08; } //stops timer/counter
		else { TCCR0B |= 0x03; } // resumes/continues timer/counter

		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR0A = 0xFFFF; }

		// prevents OCR0A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR0A = 0x0000; }

		// set OCR3A based on desired frequency
		else { OCR0A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT0 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}

void PWM_on() {
	TCCR0A = (1 << COM0A0);
	// COM3A0: Toggle PB3 on compare match between counter and OCR0A
	TCCR0B = (1 << WGM02) | (1 << CS01) | (1 << CS00);
	// WGM02: When counter (TCNT0) matches OCR0A, reset counter
	// CS01 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR0A = 0x00;
	TCCR0B = 0x00;
}

unsigned char tmpA = 0x00, tl_tmpB = 0x00, bl_tmpB = 0x00, cl_tmpB = 0x00;
double frequency = 0;
//THREE LED SM
enum three_led_States {tl_INIT, tl_LED1, tl_LED2, tl_LED3} tl_state;

void tl_tick(){
  switch(tl_state){ //transition
    case tl_INIT:
      tl_state = tl_LED1;
      break;
    case tl_LED1:
      tl_state = tl_LED2;
      break;
    case tl_LED2:
      tl_state = tl_LED3;
      break;
    case tl_LED3:
      tl_state = tl_LED1;
      break;
    default:
      break;
  }
  switch(tl_state){ //action
    case tl_INIT:
      break;
    case tl_LED1:
      tmpB = 0x01;
      break;
    case tl_LED2:
      tmpB = 0x02;
      break;
    case tl_LED3:
      tmpB = 0x04;
      break;
    default:
      break;
  }
}
//BLINKING LED SM
enum blinking_led_States{bl_INIT, bl_ON, bl_OFF} bl_state;

void bl_tick(){
  switch(bl_state){
    case bl_INIT:
      bl_state = bl_OFF;
      break;
    case bl_ON:
      bl_state = bl_OFF;
      break;
    case bl_OFF:
      bl_state = bl_ON;
    default:
      break;
  }
  switch(bl_state){
    case bl_INIT:
      break;
    case bl_OFF:
      bl_tmpB = 0x00;
      break;
    case bl_ON:
      bl_tmpB = 0x08;
      break;
    default:
      break;
  }
}

//COMBINE LED SM
enum combine_State {cl_INIT, cl_COMB} cl_state;

void cl_tick(){
  switch(cl_state){
    case cl_INIT:
      cl_state = cl_COMB;
      break;
    case cl_COMB:
      cl_state = cl_COMB;
      break;
    default:
      break;
  }
  switch(cl_state){
    case cl_INIT:
      break;
    case cl_COMB:
      cl_tmpB = bl_tmpB | tl_tmpB;
      set_PWM(frequency);
      break;
    default:
      break;
  }
  PORTB = cl_tmpB;
}
//speaker
enum sp_States {sp_INIT, sp_ON, sp_OFF} sp_state;

void sp_tick(){
  switch(sp_state){
    case sp_INIT:
      sp_state = sp_ON;
      break;
    case sp_ON;
      if(tmpA = 0x00){
        sp_state = sp_OFF;
      }
      break;
    case sp_OFF:
      if(tmpA = 0x01){
        sp_state = sp_ON;
      }
      break;
    default:
      break;
  }
  switch(sp_state){
    case sp_INIT:
      break;
    case sp_OFF:
      frequency = cf_val[0];
    case sp_ON:
      frequency = cf_val[1];
  }
}
//change frequency
enum cf_States{cf_INIT, cf_INC, cf_DEC, cf_HOLD} cf_state;
double cf_val[2] = {0.00, 261.63}; //min and max values for frequency
void cf_tick(){
  switch(cf_state){
      case cf_INIT:
        if(tmpA = 0x02){
          cf_state = cf_INC;
        }else if(tmpA = 0x04){
          cf_state = cf_DEC;
        }
        break;
      case cf_INC:
        cf_state = cf_HOLD;
        break;
      case cf_DEC:
        cf_state = cf_HOLD;
        break;
      case cf_HOLD:
        if(tmpA = 0x00){
          cf_state = cf_INIT;
        }
        break;
      default:
        break;
  }
  switch(cf_state){
    case cf_INIT:
      break;
    case cf_INC:
      cf_val[1] += 20.00;
      break;
    case cf_DEC:
      cf_val[1] -= 20.00;
			break;
		case cf_HOLD:
			break;
		default:
			break;
  }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0xFF; PORTB = 0x00;
		DDRA = 0x00; PORTA = 0xFF;
    unsigned long tl_elapsed = 0, bl_elapsed = 0, sp_elapsed = 0, cf_elapsed = 0;
    const unsigned long timer_period = 100;
    TimerSet(1); //1 second ticks
    TimerOn();

    tl_state = tl_INIT;
    bl_state = bl_INIT;
    cl_state = cl_INIT;
    sp_state = sp_INIT;
		cf_state = cf_INIT;
    tl_tmpB = 0x00;
    bl_tmpB = 0x00;
    cl_tmpB = 0x00;

    /* Insert your solution below */
    while (1) {
      tmpA = ~PINA;
      if(tl_elapsed >= 30000){ //300ms
        tl_tick();
        cl_tick();
        tl_elapsed = 0; //reset
      }
      if(bl_elapsed >= 100000){ //1 second
        bl_tick();
        cl_tick();
        bl_elapsed = 0; //reset
      }
			if(cf_elapsed >= 5000){ //50ms
				cf_tick();
				cl_tick();
				cf_elapsed = 0; //reset
			}
      if(sp_elapsed >= 100){ //1ms
        sp_tick();
        cl_tick();
        sp_elapsed = 0; //reset
      }
      while(!TimerFlag){}
      TimerFlag = 0;
      tl_elapsed += timer_period;
      bl_elapsed += timer_period;
      sp_elapsed += timer_period;
			cf_elapsed += timer_period;
    }
    return 1;
}
