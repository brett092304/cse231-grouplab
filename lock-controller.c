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

#include <CowPi.h>
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

static int system_status;
static uint8_t working_combination[3] = {0, 0, 0};

void display_lock() {
	char display_working_combination[16];
	sprintf(display_working_combination, "%02d-%02d-%02d", working_combination[0], working_combination[1], working_combination[2]);
	if (started && system_status == LOCKED) {
		display_string(0, display_working_combination);
	} else if (system_status == UNLOCKED) {
		display_string(0, "Open");
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

void handle_left_button() {
	bool is_valid_combination = true;
	for (int i = 0; i < 3 && is_valid_combination; i++) {
		if (working_combination[i] != combination[i]) {
			is_valid_combination = false;
		}
	}

	if (is_valid_combination) {
		system_status = UNLOCKED;
	}
}

void initialize_lock_controller() {
	system_status = LOCKED;
	started = false;
	display_string(0, "  -  -  ");
	register_pin_ISR((1 << 2), handle_left_button);
}

uint8_t update_working_combination(uint8_t index, direction_t direction) {
	if (direction == CLOCKWISE) {
		started = true;
		if (index == 1) {
			index++;
			working_combination[index] = working_combination[index - 1];
		}
		working_combination[index]++;
		if (working_combination[index] > 15) {
			working_combination[index] = 0;
		}
		return index;
	} else if (direction == COUNTERCLOCKWISE) {
		started = true;
		if (index == 0) {
			index++;
			working_combination[index] = working_combination[index - 1];
		} else if (index == 2) {
			for (int i = 0; i < 3; i++) {
				working_combination[i] = 0;
			}
			index = 0;
			started = false;
		}
		working_combination[index]--;
		if (working_combination[index] > 15) {
			working_combination[index] = 15;
		}
		return index;
	}
	return index;
}

void control_lock() {
	static uint8_t working_index = 0;
    direction_t direction = get_direction();
	working_index = update_working_combination(working_index, direction);
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
}


