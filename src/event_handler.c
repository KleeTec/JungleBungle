#include <stdbool.h>

#include <SDL2/SDL.h>

#include "include/main.h"


/**
 * Der Schreiber-Thread, der sich um die Spiellogik kümmert.
 * @param game ==> das Spiel
 */
_Noreturn void JB_EventHandler() {
	SDL_Event event;
	while(true) if(SDL_PollEvent(&event)) JB_Game.eventHandlerFunctions[JB_Game.modeType](&event);
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