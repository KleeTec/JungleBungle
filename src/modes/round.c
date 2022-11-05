#include <SDL2/SDL.h>
#include "../include/things/game_objects.h"
#include "../include/game_logic.h"
#include "../include/main.h"

void JB_changeModeToRound() {
	if(Game.gameObjects) {
		Game.modeType = JB_MODE_ROUND;
		return;
	}

	JB_GameObject* ground = calloc(1, sizeof *ground);
	ground->hitBox.w = 280;
	ground->hitBox.h = 160;
	ground->hitBox.x = Game.windowSize.w / 2 + ground->hitBox.w;
	ground->hitBox.y = Game.windowSize.h - ground->hitBox.h * 2;
	ground->assets = JB_new_Image("assets/sprites/ground.png");
	JB_updateAsset(ground->assets, (JB_Asset) { .rect=&ground->hitBox }, JB_AssetUpdate_rect);
	JB_appendGameObject(ground);

	JB_GameObject* ground1 = calloc(1, sizeof *ground1);
	ground1->hitBox.w = 280;
	ground1->hitBox.h = 160;
	ground1->hitBox.x = Game.windowSize.w / 2 - ground1->hitBox.w / 2;
	ground1->hitBox.y = Game.windowSize.h - ground1->hitBox.h;
	ground1->assets = JB_new_Image("assets/sprites/ground.png");
	JB_updateAsset(ground1->assets, (JB_Asset) { .rect=&ground1->hitBox }, JB_AssetUpdate_rect);
	JB_appendGameObject(ground1);

	// TODO: Spieler an Bildschirmgröße anpassen
	static JB_GameObject player = {};
	player.hitBox.w = 128;
	player.hitBox.h = 128;
	player.hitBox.x = ( Game.windowSize.w - player.hitBox.w ) / 2;
	player.hitBox.y = ( Game.windowSize.h - player.hitBox.h ) / 2;
	player.assets = JB_new_Image("assets/sprites/player.png");
	static SDL_Rect size = {};
	size.x = player.hitBox.x;
	size.y = player.hitBox.y;
	size.w = 128;
	size.h = 128;
	JB_updateAsset(player.assets, (JB_Asset) { .rect=&size},
				   JB_AssetUpdate_rect);
	Game.data.round.player = &player;

	Game.modeType = JB_MODE_ROUND;
}


void JB_render_round() {
	SDL_SetRenderDrawColor(Game.renderer, 50, 255, 100, 255);
	JB_renderAssets(Game.assets);
	JB_GameObject* currentObject = Game.gameObjects;
	while(currentObject) {
		JB_renderAssets(currentObject->assets);
		SDL_RenderDrawRect(Game.renderer, &currentObject->hitBox);
		currentObject = currentObject->next;
	}
	SDL_RenderDrawRect(Game.renderer, &Game.data.round.player->hitBox);
	JB_renderAssets(Game.data.round.player->assets);
}

void JB_handleEvents_round(SDL_Event* event) {
	if(!( event->type == SDL_KEYDOWN || event->type == SDL_KEYUP )) return;
	switch(event->key.keysym.sym) {
		case SDLK_a:
			Game.controls.aHeld = event->type == SDL_KEYDOWN;
			break;
		case SDLK_d:
			Game.controls.dHeld = event->type == SDL_KEYDOWN;
			break;
		case SDLK_SPACE:
			if (!Game.data.round.grounded) break;
			Game.data.round.player->motion.y -= 20;
			Game.data.round.grounded = false;
			break;
		case SDLK_ESCAPE:
			JB_changeModeToMenu(true);
			break;
		default:
			return;
	}
}
