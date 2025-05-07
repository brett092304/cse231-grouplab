/**************************************************************************//**
 *
 * @file rotary-encoder.c
 *
 * @author Cale Sigerson
 * @author (Brett Johnson)
 *
 * @brief Code to control a servomotor.
 *
 ******************************************************************************/

/*
 * ComboLock GroupLab assignment and starter code (c) 2022-24 Christopher A. Bohn
 * ComboLock solution (c) the above-named students
 */

#include <CowPi.h>
#include "servomotor.h"
#include "interrupt_support.h"
#include "display.h"

#define SERVO_PIN           (22)
#define PULSE_INCREMENT_uS  (500)
#define SIGNAL_PERIOD_uS    (20000)

static int volatile pulse_width_us;
volatile uint32_t isr_call_count = 0;

static void handle_timer_interrupt();

void initialize_servo() {
    cowpi_set_output_pins(1 << SERVO_PIN);
    center_servo();
    register_periodic_timer_ISR(0, PULSE_INCREMENT_uS, handle_timer_interrupt);
}

char *test_servo(char *buffer) {
    static char buff[25] = {0};
    sprintf(buff, "%d", isr_call_count);
    display_string(4, buff);
    
    if(cowpi_left_button_is_pressed()) {
        center_servo();
        sprintf(buffer, "SERVO: center");
    } else {
        if(!cowpi_left_switch_is_in_left_position()) {
            rotate_full_counterclockwise();
            sprintf(buffer, "SERVO: right");
        } else {
            rotate_full_clockwise();
            sprintf(buffer, "SERVO: left");
        }
    }
    return buffer;
}

void center_servo() {
    pulse_width_us = PULSE_INCREMENT_uS * 3;
}

void rotate_full_clockwise() {
    pulse_width_us = PULSE_INCREMENT_uS;
}

void rotate_full_counterclockwise() {
    pulse_width_us = PULSE_INCREMENT_uS * 5;
}

static void handle_timer_interrupt() {
    //static int volatile timeUntilRise = 0;
    //static int volatile timeUntilFall = 0;
    isr_call_count++;

    static int time_to_rise = PULSE_INCREMENT_uS;
    static int time_to_fall = PULSE_INCREMENT_uS;

    cowpi_ioport_t volatile *ioport = (cowpi_ioport_t *)(0xD0000000);

   
    time_to_rise -= PULSE_INCREMENT_uS;
    time_to_fall -= PULSE_INCREMENT_uS;

   
    if (time_to_rise <= 0) {
        ioport->output |= (1 << SERVO_PIN);
        time_to_rise = SIGNAL_PERIOD_uS;    
        time_to_fall = pulse_width_us;      
    }

   
    if (time_to_fall <= 0) {
        ioport->output &= ~(1 << SERVO_PIN);
    }
    
    /*if(timeUntilRise == 0) {
        digitalWrite(SERVO_PIN, HIGH);
        timeUntilRise = SIGNAL_PERIOD_uS;
        timeUntilFall = pulse_width_us;
    }

    if(timeUntilFall == 0) {
        digitalWrite(SERVO_PIN, LOW);
    }

    if(timeUntilRise > 0) {
        timeUntilRise -= PULSE_INCREMENT_uS;
    }
    
    if(timeUntilFall > 0) {
        timeUntilFall -= PULSE_INCREMENT_uS;
    }*/
    
    /*static int time = 0;

    time += PULSE_INCREMENT_uS;

    if(time <= pulse_width_us) {
        digitalWrite(SERVO_PIN, HIGH);
    } else {
        digitalWrite(SERVO_PIN, LOW);
    } 
    if (time >= SIGNAL_PERIOD_uS) {
        time = 0;
    }*/
    
        
    

    
}
