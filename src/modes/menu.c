#include <SDL2/SDL.h>

#include "../include/modes.h"
#include "../include/game_logic.h"
#include "../include/main.h"


void JB_changeModeToMenu() {
	if(Game.buttons[JB_MODE_MENU]) {
		Game.modeType = JB_MODE_MENU;
		return;
	}
	JB_Button* button3 = calloc(1, sizeof *button3);
	button3->onclick = JB_quit;
	JB_new_MenuButton(button3, "Exit");

	JB_Button* button2 = calloc(1, sizeof *button2);
	button2->onclick = JB_changeModeToLevelEditor;
	button2->next = button3;
	JB_new_MenuButton(button2, "Level Editor - Alpha");

	JB_Button* button1 = calloc(1, sizeof *button1);
	button1->onclick = JB_changeModeToRound;
	button1->next = button2;
	JB_new_MenuButton(button1, "Start Game");

	Game.buttons[JB_MODE_MENU] = button1;
	Game.modeType = JB_MODE_MENU;
}

void JB_new_MenuButton(JB_Button* button, char* string) {
	JB_Asset* text = JB_new_Text(string, (SDL_Colour) { 255, 255, 255 }, Game.fonts.defaultFont);
	text->centered = true;
	JB_updateAsset(text,
				   (JB_Asset) { .fontFitRect=true, .rect=&button->rect },
				   JB_AssetUpdate_fontFitRect | JB_AssetUpdate_rect);
	button->assets = JB_new_Image("assets/button.png");
	JB_updateAsset(button->assets, (JB_Asset) { .rect = &button->rect, .next=text },
				   JB_AssetUpdate_rect | JB_AssetUpdate_next);
}

void JB_render_menu() {
	JB_renderAssets(Game.assetsHardcoded.background);
	SDL_Rect titleRect = { ( Game.windowSize.w - 1200 ) / 2,
						   ( Game.windowSize.h - 300 ) / 2 - 300,
						   1200, 300 };
	JB_updateAsset(Game.assetsHardcoded.title, (JB_Asset) { .rect = &titleRect }, JB_AssetUpdate_rect);
	JB_renderAssets(Game.assetsHardcoded.title);

	JB_Button* currentButton = Game.buttons[JB_MODE_MENU];
	/**
 	* Für die Ereignisbehandlung von Mausklicks bzw Mausbewegungen werden Listen an anklickbaren Objekten für jeden Spielmodus
	* erstellt. Bei einem Mausereignis wird dann die aktuelle Liste durchiteriert und bei jedem Objekt geschaut, was das
	* Ereignis für einen Effekt auf dieses hat. Ist z.B. die Maus über einem Objekt, wird .hover=true gesetzt, damit der
	* Renderer dann darauf reagieren kann und das Objekt vielleicht anders dargestellt wird. Wird jedoch ein Klick-Ereignis
	* gesendet, muss die Funktion ausgeführt werden, die das Objekt als Eigenschaft trägt.
 	*/
	int counter = 0;
	while(currentButton != NULL) {
		SDL_Rect r = { Game.windowSize.w / 2 - 200, Game.windowSize.h / 2 + counter * 100, 400, 80 };
		currentButton->rect = r;
		if(currentButton->hover) {
			SDL_SetTextureAlphaMod(currentButton->assets->texture, 200);
			SDL_SetTextureBlendMode(currentButton->assets->texture, SDL_BLENDMODE_BLEND);
		} else {
			SDL_SetTextureAlphaMod(currentButton->assets->texture, 255);
			SDL_SetTextureBlendMode(currentButton->assets->texture, SDL_BLENDMODE_NONE);
		}

		JB_renderAssets(currentButton->assets);
		currentButton = currentButton->next;
		counter++;
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
				JB_Button* object = Game.buttons[JB_MODE_MENU];
				while(object != NULL) {
					if(event->button.x >= object->rect.x &&
					   event->button.y >= object->rect.y &&
					   event->button.x <= object->rect.x + object->rect.w &&
					   event->button.y <= object->rect.y + object->rect.h) {
						object->onclick(object);
						return;
					}
					object = object->next;
				}
			});
			break;
	}
}


void JB_onTestButtonClick(JB_Button* this) {
	JB_Asset* text = JB_new_Text("Test", (SDL_Colour) { 255, 255, 255 }, Game.fonts.defaultFont);
	JB_updateAsset(text, (JB_Asset) { .rect=&this->rect }, JB_AssetUpdate_rect);

	JB_Asset* currentAsset = this->assets;
	if(currentAsset == NULL) this->assets = text;
	else {
		while(currentAsset->next != NULL) currentAsset = currentAsset->next;
		currentAsset->next = text;
	}
}