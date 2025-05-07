/**************************************************************************//**
 *
 * @file lock-controller.c
 *
 * @author Cale Sigerson
 * @author (Brett Johnson)
 *
 * @brief Code to implement the "combination lock" mode.
 *
 ******************************************************************************/

/*
 * ComboLock GroupLab assignment and starter code (c) 2022-24 Christopher A. Bohn
 * ComboLock solution (c) the above-named students
 */

//#include <stdlib.h>
#include <CowPi.h>
#include <stdbool.h>
#include <stdio.h>
#include "boards/rp2040.h"
#include "display.h"
#include "interrupt_support.h"
#include "io/cowpi_io.h"
#include "lock-controller.h"
#include "rotary-encoder.h"
#include "servomotor.h"

static uint8_t combination[3] __attribute__((section (".uninitialized_ram.")));
enum {
	LOCKED, UNLOCKED, ALARMED, CHANGING,
};
static bool started;

static volatile int system_status;
static volatile uint8_t working_combination[3];
static volatile uint8_t combo_passes[5];
static volatile uint8_t working_index;
static volatile uint8_t warnings = 0;
static volatile uint8_t proposed_combination[3];
static volatile uint8_t confirm_proposed_combination[3];

void send_alarm() {
	cowpi_timer_t volatile *timer = (cowpi_timer_t *) 0x40054000;
	while(1) {
		display_string(0, "ALERT");
		display_string(1, "");
		refresh_display();
		uint32_t then = timer->raw_lower_word;
		cowpi_illuminate_left_led();
		cowpi_deluminate_right_led();
		then = timer->raw_lower_word;
		while(timer->raw_lower_word - then < 250000) {}
		cowpi_illuminate_right_led();
		cowpi_deluminate_left_led();
		then = timer->raw_lower_word;
		while(timer->raw_lower_word - then < 250000) {}
	}
}

void display_lock() {
	char display_working_combination[16];
	sprintf(display_working_combination, "%02d-%02d-%02d", working_combination[0], working_combination[1], working_combination[2]);
	if (started && system_status == LOCKED) {
		display_string(0, display_working_combination);
	} else if (system_status == UNLOCKED) {
		display_string(0, "OPEN");
		display_string(1, "");
	} else if (system_status == ALARMED) {
		refresh_display();
		send_alarm();
	} else {
		display_string(0, "  -  -  ");
	}
}

uint8_t const *get_combination() {
    return combination;
}

void force_combination_reset() {
    combination[0] = 5;
    combination[1] = 10;
    combination[2] = 15;
}

void reset_lock() {
	for (int i = 0; i < 3; i++) {working_combination[i] = 0;}
	for (int i = 0; i < 5; i++) {combo_passes[i] = 0;}
	started = false;
	working_index = 0;
}

void handle_left_button(void) {
	if (cowpi_left_button_is_pressed()) {
		bool is_valid_combination = false;
		if (working_combination[0] == combination[0] &&
			working_combination[1] == combination[1] &&
			working_combination[2] == combination[2]) {
			is_valid_combination = true;
		}

		if (!(combo_passes[0] > 2 && combo_passes[1] == 2 && combo_passes[2] == 1)) {
			is_valid_combination = false;
		}

		if (is_valid_combination) {
			system_status = UNLOCKED;
			for (int i = 0; i < 3; i++) {working_combination[i] = 0; combo_passes[i] = 0;}
		} else {
			if (system_status != UNLOCKED) {
				warnings++;
				char buffer[16];
				sprintf(buffer, "bad try %d", warnings);
				display_string(1, buffer);
			}
		}
		
		if (warnings > 2) {
			system_status = ALARMED;
		}
	}
	if (cowpi_left_button_is_pressed() && cowpi_right_button_is_pressed()) {
		system_status = LOCKED;
		reset_lock();
		started = false;
		warnings = 0;
		display_string(1, "");
	}
}

void handle_right_button() {
	if (cowpi_right_button_is_pressed()) {
		if (cowpi_left_button_is_pressed() && system_status == UNLOCKED && cowpi_left_switch_is_in_left_position()) {
			system_status = LOCKED;
			reset_lock();
			started = false;
			warnings = 0;
			display_string(1, "");
		} else if (cowpi_left_switch_is_in_right_position() && system_status == UNLOCKED) {
			system_status = CHANGING;
		}
	}
}

void initialize_lock_controller() {
	force_combination_reset();
	system_status = LOCKED;
	started = false;
	reset_lock();
	register_pin_ISR((1L << 2), handle_left_button);
	register_pin_ISR((1L << 3), handle_right_button);
}

void update_working_combination(direction_t direction) {
	if (direction == CLOCKWISE) {
		started = true;
		if (working_index == 1) {
			working_index++;
			working_combination[working_index] = working_combination[working_index - 1];
		} 

		working_combination[working_index]++;
		if (working_combination[working_index] > 15) {
			working_combination[working_index] = 0;
		}
		if (working_index == 2) {
			if (working_combination[working_index] == combination[2]) {combo_passes[2]++;}
			if (working_combination[working_index] == 0) {combo_passes[3]++;}
			if (combo_passes[3] == 2) {reset_lock();}
		}
		if (working_index == 0 && working_combination[working_index] == combination[0]) {combo_passes[0]++;}
	} else if (direction == COUNTERCLOCKWISE) {
		started = true;
		if (working_index == 0) {
			working_index++;
			working_combination[working_index] = working_combination[working_index - 1];
		} else if (working_index == 2) {
			reset_lock();
		}
		working_combination[working_index]--;
		if (working_combination[working_index] > 15) {
			working_combination[working_index] = 15;
		}
		if (working_combination[working_index] == combination[1]) {combo_passes[1]++;}
		if (working_combination[working_index] == 0) {combo_passes[4]++;}
		if (combo_passes[4] == 3) {reset_lock();}
	}
}

void control_lock() {
    direction_t direction = get_direction();
	update_working_combination(direction);
	display_lock();

	if (system_status == LOCKED) {
		cowpi_illuminate_left_led();
	} else {
		cowpi_deluminate_left_led();
	}

	if (system_status == UNLOCKED) {
		cowpi_illuminate_right_led();
	} else {
		cowpi_deluminate_right_led();
	}

	if (system_status == CHANGING && cowpi_left_switch_is_in_left_position()) {
		bool matching_combos = true;
		bool are_values_less_fifteen = true;
		bool is_combo_complete = true;

		for (int i = 0; i < 3; i++) {
			if (proposed_combination[i] != confirm_proposed_combination[i]) {matching_combos = false;}
		}

		for (int i = 0; i < 3; i++) {
			if (proposed_combination[i] > 15 || confirm_proposed_combination[i] > 15) {are_values_less_fifteen = false;}
		}

		if (matching_combos && are_values_less_fifteen && is_combo_complete) {
			display_string(2, "Changed");
			for (int i = 0; i < 3; i++) {
				combination[i] = confirm_proposed_combination[i];
			}
			system_status = UNLOCKED;
		}
	}
}


