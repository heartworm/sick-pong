#include <cab202_timers.h>

#include "interface.h"

// Configuration
#define DELAY (10) /* Millisecond delay between game updates */

bool game_over = false;
bool update_screen = true;

// Setup game.
void setup(void) {
	initWindow();
	initGame(false);
}

// Play one turn of game.
void process(void) {
	int c = get_char();
	handleKey(c);
	if (!game_over) {
		stepGame();
		drawGame();
	}
}


// Clean up game
void cleanup(void) {
	// STATEMENTS
}

// Program entry point.
int main(void) {
	signal(SIGINT, quitGame); //clean up resources
	
	setup_screen();
	setup();
	show_screen();

	while ( !game_over ) {
		process();

		timer_pause(DELAY);
	}

	cleanup(); 
	
	return 0;
}
