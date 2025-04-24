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

#define SERVO_PIN           (22)
#define PULSE_INCREMENT_uS  (50)
#define SIGNAL_PERIOD_uS    (20000)

static int volatile pulse_width_us;

static void handle_timer_interrupt();

void initialize_servo() {
    cowpi_set_output_pins(1 << SERVO_PIN);
    center_servo();
//    register_periodic_timer_ISR(0, PULSE_INCREMENT_uS, handle_timer_interrupt);
}

char *test_servo(char *buffer) {
    volatile cowpi_ioport_t *ioport = (cowpi_ioport_t *)(0xD0000000);
    uint8_t leftButton = ioport->input & (1 << 2); // 1 is unpressed
    uint8_t leftSwitch = !((ioport->input & (1 << 14)) != (1 << 14)); // 1 is right position
    if(!leftButton) {
        center_servo();
        sprintf(buffer, "SERVO: center");
    } else {
        if(leftSwitch) {
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
    ;
}

void rotate_full_clockwise() {
    ;
}

void rotate_full_counterclockwise() {
    ;
}

static void handle_timer_interrupt() {
    ;
}
