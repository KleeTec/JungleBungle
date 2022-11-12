#include <SDL2/SDL.h>

#include "../include/modes.h"
#include "../include/game_logic.h"
#include "../include/main.h"


void JB_changeModeToLevelEditor() {
	static JB_Button button1 = {
			.rect={ 1, 1, 100, 100 },
			.onclick=JB_changeModeToMenu
	};
	static JB_Button button2 = {
			.rect ={ 100, 100, 200, 200 },
			.onclick=JB_initDrag,
			.next=&button1
	};
	static JB_Button button3 = {
			.rect ={ 100, 100, 200, 200 },
			.onclick=JB_initDrag,
			.next=&button2
	};
	Game.buttons[JB_MODE_LEVEL_EDITOR] = &button3;
	Game.modeType = JB_MODE_LEVEL_EDITOR;
}

void JB_render_levelEditor() {
	JB_Button* current = Game.buttons[JB_MODE_LEVEL_EDITOR];
	/**
 	* Für die Ereignisbehandlung von Mausklicks bzw Mausbewegungen werden Listen an anklickbaren Objekten für jeden Spielmodus
	* erstellt. Bei einem Mausereignis wird dann die aktuelle Liste durchiteriert und bei jedem Objekt geschaut, was das
	* Ereignis für einen Effekt auf dieses hat. Ist z.B. die Maus über einem Objekt, wird .hover=true gesetzt, damit der
	* Renderer dann darauf reagieren kann und das Objekt vielleicht anders dargestellt wird. Wird jedoch ein Klick-Ereignis
	* gesendet, muss die Funktion ausgeführt werden, die das Objekt als Eigenschaft trägt.
 	*/
	while(current != NULL) {
		SDL_Rect rect = current->rect;
		rect.x += current->alignment.x;
		rect.y += current->alignment.y;
		/*SDL_SetRenderDrawColor(Game.renderer, 255, 255 * current->hover, 10, 255);*/
		SDL_RenderDrawRect(Game.renderer, &rect);
		JB_renderAssets(current->assets);
		current = current->next;
	}
}

void JB_handleEvents_levelEditor(SDL_Event* event) {
	switch(event->type) {
		case SDL_MOUSEMOTION:
			if(Game.data.levelEditor.selectedClickableObject == NULL)
				JB_setHover(JB_MODE_LEVEL_EDITOR, event);
			else {
				JB_drag(event->motion, true);
				JB_Button* object = Game.buttons[JB_MODE_LEVEL_EDITOR];
				while(object != NULL) {
					object->hover = false;
					object = object->next;
				}
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			({
				if(event->button.button != SDL_BUTTON_LEFT) break;
				Game.data.levelEditor.oldMousePos.x = event->button.x;
				Game.data.levelEditor.oldMousePos.y = event->button.y;
				JB_Button* object = Game.buttons[JB_MODE_LEVEL_EDITOR];
				while(object != NULL) {
					if(event->button.x >= object->rect.x &&
					   event->button.y >= object->rect.y &&
					   event->button.x <= object->rect.x + object->rect.w &&
					   event->button.y <= object->rect.y + object->rect.h)
						object->onclick(object);
					object = object->next;
				}
			});
			break;
		case SDL_MOUSEBUTTONUP:
			Game.data.levelEditor.selectedClickableObject = NULL;
			break;
	}
}

void JB_initDrag(JB_Button* this) {
	Game.data.levelEditor.selectedClickableObject = this;
}

void JB_drag(SDL_MouseMotionEvent motion, bool aligned) {
	JB_Button* obj = Game.data.levelEditor.selectedClickableObject;
	int windowSizeX, windowSizeY;
	SDL_GetWindowSize(Game.window, &windowSizeX, &windowSizeY);

	int deltaMouseX = motion.x - Game.data.levelEditor.oldMousePos.x;
	int newX = obj->rect.x + deltaMouseX;
	if(newX > 0 && newX + obj->rect.w < windowSizeX) obj->rect.x = newX;
	Game.data.levelEditor.oldMousePos.x += deltaMouseX;

	int deltaMouseY = motion.y - Game.data.levelEditor.oldMousePos.y;
	int newY = obj->rect.y + deltaMouseY;
	if(newY > 0 && newY + obj->rect.h < windowSizeY) obj->rect.y = newY;
	Game.data.levelEditor.oldMousePos.y += deltaMouseY;

	if(aligned) {
		int alignFactor = 50;
		int deltaX = obj->rect.x % alignFactor;
		if(deltaX >= alignFactor >> 1) obj->alignment.x = alignFactor - deltaX;
		else obj->alignment.x = -deltaX;

		int deltaY = obj->rect.y % alignFactor;
		if(deltaY >= alignFactor >> 1) obj->alignment.y = alignFactor - deltaY;
		else obj->alignment.y = -deltaY;
	}
}