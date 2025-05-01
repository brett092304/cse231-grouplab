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

void send_alarm() {
	cowpi_timer_t *timer = (cowpi_timer_t *) 0x40054000;
	while(1) {
		display_string(0, "ALERT");
		int now = timer->raw_lower_word;
		while (timer->raw_lower_word > now - 250000U) {}
		cowpi_illuminate_left_led();
		cowpi_deluminate_right_led();
		now = timer->raw_lower_word;
		while(timer->raw_lower_word > now - 250000U) {}
		cowpi_illuminate_right_led();
		cowpi_deluminate_left_led();
	}
}

void display_lock() {
	char display_working_combination[16];
	sprintf(display_working_combination, "%02d-%02d-%02d", working_combination[0], working_combination[1], working_combination[2]);
	if (started && system_status == LOCKED) {
		display_string(0, display_working_combination);
	} else if (system_status == UNLOCKED) {
		display_string(0, "OPEN");
	} else if (system_status == ALARMED) {
		display_string(0, "ALERT");
		display_string(1, "");
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

void handle_left_button(void) {
	if (cowpi_left_button_is_pressed()) {
		static uint8_t warnings = 0;
		bool is_valid_combination = false;
		if (working_combination[0] == combination[0] &&
			working_combination[1] == combination[1] &&
			working_combination[2] == combination[2]) {
			is_valid_combination = true;
		}

		if (is_valid_combination) {
			system_status = UNLOCKED;
			for (int i = 0; i < 3; i++) {working_combination[i] = 0;}
		} else {
			warnings++;
			char buffer[16];
			sprintf(buffer, "bad try %d", warnings);
			display_string(1, buffer);
		}
		
		if (warnings > 2) {
			system_status = ALARMED;
		}
	}
}

void initialize_lock_controller() {
	force_combination_reset();
	system_status = LOCKED;
	started = false;
	for (int i = 0; i < 3; i++) {working_combination[i] = 0;}
	register_pin_ISR((1L << 2), handle_left_button);
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


