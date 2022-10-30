#include <SDL2/SDL.h>
#include "../include/things/game_objects.h"
#include "../include/game_logic.h"
#include "../include/main.h"

void JB_changeModeToRound() {
	if(Game.gameObjects) {
		Game.modeType = JB_MODE_ROUND;
		return;
	}
	JB_Asset* background = JB_new_Image("assets/sprites/background.png");
	JB_updateAsset(background, (JB_Asset) { .rect=&Game.windowSize }, JB_AssetUpdate_rect);
	JB_appendAsset(background);

	JB_GameObject* ground = calloc(1, sizeof *ground);
	ground->hitBox.w = Game.windowSize.w;
	ground->hitBox.h = 40;
	ground->hitBox.x = 0;
	ground->hitBox.y = Game.windowSize.h - ground->hitBox.h;
	ground->assets = JB_new_Image("assets/sprites/ground.png");
	JB_updateAsset(ground->assets, (JB_Asset) { .rect=&ground->hitBox }, JB_AssetUpdate_rect);
	JB_appendGameObject(ground);

	// TODO: Spieler an Bildschirmgröße anpassen
	static JB_GameObject player = {};
	player.hitBox.w = 50;
	player.hitBox.h = 100;
	player.hitBox.x = ( Game.windowSize.w - player.hitBox.w ) / 2;
	player.hitBox.y = ( Game.windowSize.h - player.hitBox.h ) / 2;
	player.assets = JB_new_Image("assets/sprites/player.png");
	static SDL_Rect size = {};
	size.x = player.hitBox.x - 20;
	size.y = player.hitBox.y - 4;
	size.w = size.h = 100;
	static SDL_Rect clip = { 0, 0, 32, 32 };
	JB_updateAsset(player.assets, (JB_Asset) { .rect=&size, .clip=&clip },
				   JB_AssetUpdate_rect | JB_AssetUpdate_clip);
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
			Game.data.round.player->motion.y -= 10;
			break;
		case SDLK_ESCAPE:
			JB_changeModeToMenu();
			break;
		default:
			return;
	}
}
