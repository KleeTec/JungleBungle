#include <stdbool.h>

#include <SDL2/SDL.h>

#include "include/main.h"
#include "include/util.h"


/**
 * Der Schreiber-Thread, der sich um die Spiellogik kümmert.
 * @param game ==> das Spiel
 */
_Noreturn void JB_EventHandler() {
	SDL_Event event;
	long t1 = currentTimeMillis();
	long t2 = t1;
	while(true) {
		SDL_GetWindowSize(Game.window, &Game.windowSize.width, &Game.windowSize.height);

		if(SDL_PollEvent(&event)) Game.eventHandlerFunctions[Game.modeType](&event);

		/**
		 * Positionsberechnung der GameObjects
		 */
		if(Game.modeType == JB_MODE_ROUND) {
			double delta_time = (double) ( t2 - t1 );
			double restTime = ( 1000.0 / JB_MAX_FPS ) - delta_time;
			if(restTime <= 0) {
				t1 = currentTimeMillis();
				JB_GameObject* player = Game.data.round.player;

				// wenn Knopf gedrückt
				if(Game.controls.dHeld && player->motion.x < JB_MAX_MOTION_RIGHT) player->motion.x++;
				if(Game.controls.aHeld && player->motion.x > -JB_MAX_MOTION_RIGHT) player->motion.x--;
				// wenn Knopf losgelassen
				if(!Game.controls.dHeld && player->motion.x > 0) player->motion.x--;
				if(!Game.controls.aHeld && player->motion.x < 0) player->motion.x++;

				JB_GameObject* currentObj = Game.gameObjects;
				while(currentObj != NULL) {
					// TODO: Kollisionswahrnehmung
					currentObj = currentObj->next;
				}

				// Position aufgrund der Bewegung bestimmen
				int newX = player->hitBox.x + player->motion.x;
				int newY = player->hitBox.y + player->motion.y + Game.data.round.fallspeed++;
				if(newX < Game.windowSize.width - player->hitBox.w && newX > 0) {
					player->hitBox.x = newX;
					player->assets->rect->x = newX;
				}
				if(newY > 0) {
					player->hitBox.y = newY;
					player->assets->rect->y = newY;
				}
			}
			t2 = currentTimeMillis();
		}
	}
}

/**
 * Wenn über ein Objekt drüber gehalten wird, wird .hover=rue gesetzt.
 */
void JB_setHover(enum JB_ModeType modeType, SDL_Event* event) {
	JB_Button* object = Game.buttons[modeType];
	while(object != NULL) {
		if(event->motion.x >= object->rect.x &&
		   event->motion.y >= object->rect.y &&
		   event->motion.x <= object->rect.x + object->rect.w &&
		   event->motion.y <= object->rect.y + object->rect.h)
			object->hover = true;
		else object->hover = false;
		object = object->next;
	}
}