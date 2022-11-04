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

	JB_GameObject* block = calloc(1, sizeof *block);
	block->hitBox.w = 80;
	block->hitBox.h = 40;
	block->hitBox.x = 600;
	block->hitBox.y = Game.windowSize.h - 80;
	block->assets = JB_new_Image("assets/sprites/ground.png");
	JB_updateAsset(block->assets, (JB_Asset) { .rect=&block->hitBox }, JB_AssetUpdate_rect);
	JB_appendGameObject(block);

	JB_GameObject* block2 = calloc(1, sizeof *block2);
	block2->hitBox.w = 80;
	block2->hitBox.h = 40;
	block2->hitBox.x = 500;
	block2->hitBox.y = Game.windowSize.h - 150;
	block2->assets = JB_new_Image("assets/sprites/ground.png");
	JB_updateAsset(block2->assets, (JB_Asset) { .rect=&block2->hitBox }, JB_AssetUpdate_rect);
	JB_appendGameObject(block2);

	JB_GameObject* block3 = calloc(1, sizeof *block3);
	block3->hitBox.w = 80;
	block3->hitBox.h = 40;
	block3->hitBox.x = 400;
	block3->hitBox.y = Game.windowSize.h - 200;
	block3->assets = JB_new_Image("assets/sprites/ground.png");
	JB_updateAsset(block3->assets, (JB_Asset) { .rect=&block3->hitBox }, JB_AssetUpdate_rect);
	JB_appendGameObject(block3);

	JB_GameObject* block4 = calloc(1, sizeof *block4);
	block4->hitBox.w = 80;
	block4->hitBox.h = 40;
	block4->hitBox.x = 300;
	block4->hitBox.y = Game.windowSize.h - 150;
	block4->assets = JB_new_Image("assets/sprites/ground.png");
	JB_updateAsset(block4->assets, (JB_Asset) { .rect=&block4->hitBox }, JB_AssetUpdate_rect);
	JB_appendGameObject(block4);

	// TODO: Spieler an Bildschirmgröße anpassen
	static JB_GameObject player = {};
	player.hitBox.w = 50;
	player.hitBox.h = 100;
	player.hitBox.x = ( Game.windowSize.w - player.hitBox.w ) / 2;
	player.hitBox.y = ( Game.windowSize.h - player.hitBox.h ) / 2;
	player.assets = JB_new_Image("assets/sprites/player.png");
	static SDL_Rect size = {};
	size.x = player.hitBox.x;
	size.y = player.hitBox.y;
	size.w = 50;
	size.h = 100;
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
			Game.data.round.player->motion.y -= 15;
			Game.data.round.grounded = false;
			break;
		case SDLK_ESCAPE:
			JB_changeModeToMenu();
			break;
		default:
			return;
	}
}
