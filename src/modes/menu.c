#include <SDL2/SDL.h>

#include "../include/modes.h"
#include "../include/asset.h"
#include "../include/game_logic.h"
#include "../include/main.h"


void JB_changeModeToMenu() {
	static JB_Button button2 = {
			.rect={ 1, 1, 100, 100 },
			.onclick=JB_changeModeToLevelEditor
	};
	static JB_Button button1 = {
			.rect ={ 100, 100, 200, 200 },
			.onclick=JB_onTestButtonClick,
			.next=&button2
	};
	JB_Game.buttons[JB_MODE_MENU] = &button1;
	JB_Game.modeType = JB_MODE_MENU;
}

void JB_render_menu() {
	JB_Button* currentButton = JB_Game.buttons[JB_MODE_MENU];
	/**
 	* Für die Ereignisbehandlung von Mausklicks bzw Mausbewegungen werden Listen an anklickbaren Objekten für jeden Spielmodus
	* erstellt. Bei einem Mausereignis wird dann die aktuelle Liste durchiteriert und bei jedem Objekt geschaut, was das
	* Ereignis für einen Effekt auf dieses hat. Ist z.B. die Maus über einem Objekt, wird .hover=true gesetzt, damit der
	* Renderer dann darauf reagieren kann und das Objekt vielleicht anders dargestellt wird. Wird jedoch ein Klick-Ereignis
	* gesendet, muss die Funktion ausgeführt werden, die das Objekt als Eigenschaft trägt.
 	*/
	while(currentButton != NULL) {
		SDL_SetRenderDrawColor(JB_Game.renderer, 50, 255 * currentButton->hover, 100, 255);
		SDL_RenderDrawRect(JB_Game.renderer, &currentButton->rect);
		JB_renderAssets(currentButton->textureElements);
		currentButton = currentButton->next;
	}
}

void JB_handleEvents_menu(SDL_Event* event) {
	switch(event->type) {
		case SDL_MOUSEMOTION:
			JB_setHover(JB_MODE_MENU, event);
			break;
		case SDL_MOUSEBUTTONDOWN:
			({
				if(event->button.button != SDL_BUTTON_LEFT) break;
				JB_Button* object = JB_Game.buttons[JB_MODE_MENU];
				while(object != NULL) {
					if(event->button.x >= object->rect.x &&
					   event->button.y >= object->rect.y &&
					   event->button.x <= object->rect.x + object->rect.w &&
					   event->button.y <= object->rect.y + object->rect.h) {
						object->onclick(object);
					}
					object = object->next;
				}
			});
			break;
	}
}


void JB_onTestButtonClick(JB_Button* this) {
	JB_Asset* text = JB_new_Text("Test", (SDL_Colour) { 255, 255, 255 }, JB_Game.fonts.defaultFont);
	JB_updateAsset(text, (JB_Asset) { .rect=&this->rect }, JB_AssetUpdate_rect);

	JB_Asset* currentAsset = this->textureElements;
	if(currentAsset == NULL) this->textureElements = text;
	else {
		while(currentAsset->next != NULL) currentAsset = currentAsset->next;
		currentAsset->next = text;
	}
}