#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc50-cpp"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/things/game_objects.h"
#include "../include/game_logic.h"
#include "../include/main.h"

void JB_changeModeToRound() {
	if (Game.gameObjects){
		Game.modeType = JB_MODE_ROUND;
		return;
	}

	JB_GameObject* ground = calloc(1, sizeof *ground);
	ground->hitBox.w = 272;
	ground->hitBox.h = 128;
	ground->hitBox.x = Game.windowSize.w / 2 - ground->hitBox.w / 2;
	ground->hitBox.y = Game.windowSize.h - ground->hitBox.h;
	ground->assets = JB_new_Image("assets/sprites/ground.png");
	JB_updateAsset(ground->assets, (JB_Asset) { .rect=&ground->hitBox }, JB_AssetUpdate_rect);
	JB_appendGameObject(ground);

	JB_GameObject* ground1 = calloc(1, sizeof *ground1);
	ground1->hitBox.w = 272;
	ground1->hitBox.h = 128;
	ground1->hitBox.x = Game.windowSize.w / 2 - ground1->hitBox.w / 2 + 500;
	ground1->hitBox.y = Game.windowSize.h - ground1->hitBox.h;
	ground1->assets = JB_new_Image("assets/sprites/ground.png");
	JB_updateAsset(ground1->assets, (JB_Asset) { .rect=&ground1->hitBox }, JB_AssetUpdate_rect);
	JB_appendGameObject(ground1);

	JB_GameObject* ground2 = calloc(1, sizeof *ground2);
	ground2->hitBox.w = 272;
	ground2->hitBox.h = 128;
	ground2->hitBox.x = Game.windowSize.w / 2 - ground2->hitBox.w / 2 + 500 * 2;
	ground2->hitBox.y = Game.windowSize.h - ground2->hitBox.h;
	ground2->assets = JB_new_Image("assets/sprites/ground.png");
	JB_updateAsset(ground2->assets, (JB_Asset) { .rect=&ground2->hitBox }, JB_AssetUpdate_rect);
	JB_appendGameObject(ground2);
	JB_generateBlock();
	JB_generateBlock();
	JB_generateBlock();

	Game.data.round.counter = 0;

	static JB_GameObject player = {};
	player.hitBox.w = 120;
	player.hitBox.h = 110;
	player.hitBox.x = ( Game.windowSize.w - player.hitBox.w ) / 2;
	player.hitBox.y = ( Game.windowSize.h - player.hitBox.h ) / 2;
	player.assets = JB_new_Image("assets/sprites/player.png");
	player.assets->timePerClip = 1000;
	player.assets->maxClips = 5;
	player.assetsBox.x = player.hitBox.x - 7;
	player.assetsBox.y = player.hitBox.y - 5;
	player.assetsBox.w = player.hitBox.w + 30;
	player.assetsBox.h = player.hitBox.h + 5;
	player.assets->clipSize = (SDL_Rect) { 0, 0, 33, 28 };

	Game.data.round.player = &player;
	Game.modeType = JB_MODE_ROUND;
}

void JB_generateBlock() {
	int ranX = 300 + rand() % 600;
	int ranY = rand() % 200;
	bool spawnTop = rand() & 1;
	bool spawnExtra = rand() & 1;
	bool spawnBanana = rand() & 1;
	bool spawnVines = rand() & 1;

	while(ranY * ranY + ranX * ranX > 650 * 650) {
		ranX = rand() % 600 + 250;
		ranY = rand() % 200;
	}

	JB_GameObject* current = Game.gameObjects;
	while(current->next != NULL) current = current->next;

	JB_GameObject* ground = calloc(1, sizeof *ground);
	ground->hitBox.w = current->hitBox.w;
	ground->hitBox.h = current->hitBox.h;
	ground->hitBox.x = current->hitBox.x + ranX;
	ground->hitBox.y =
			!spawnTop && current->hitBox.y + ranY + ground->hitBox.h < Game.windowSize.h ?
			current->hitBox.y + ranY :
			current->hitBox.y - ranY;
	if (ground->hitBox.y < 400){
		ground->hitBox.y += 400;
	}
	ground->assets = JB_new_Image("assets/sprites/ground.png");
	ground->assetsBox = ground->hitBox;
	JB_updateAsset(ground->assets, (JB_Asset) { .rect=&ground->assetsBox }, JB_AssetUpdate_rect);

	if (spawnExtra){
		int ranExtraX = rand() % ( ground->hitBox.w - 80 );
		JB_GameObject* extraObj = calloc(1, sizeof *extraObj);
		extraObj->hitBox.w = 80;
		extraObj->hitBox.h = 64;
		extraObj->hitBox.x = ground->hitBox.x + ranExtraX;
		extraObj->hitBox.y = ground->hitBox.y - extraObj->hitBox.h;
		extraObj->next = ground;
		extraObj->assets = JB_new_Image("assets/sprites/extra.png");
		extraObj->assetsBox = extraObj->hitBox;
		extraObj->assetsBox.h += 20;
		extraObj->assetsBox.w += 20;
		extraObj->assetsBox.x -= 50;
		extraObj->assetsBox.y -= 15;
		JB_updateAsset(extraObj->assets, (JB_Asset) { .rect=&extraObj->assetsBox }, JB_AssetUpdate_rect);
		ground = extraObj;
	} else if (spawnBanana){
		int ranBananaX = rand() % ( ground->hitBox.w );
		JB_GameObject* newBanana = calloc(1, sizeof *newBanana);
		newBanana->hitBox.w = 38;
		newBanana->hitBox.h = 44;
		newBanana->hitBox.x = ground->hitBox.x + ranBananaX;
		newBanana->hitBox.y = ground->hitBox.y - newBanana->hitBox.h - 5;
		newBanana->assets = JB_new_Image("assets/sprites/banana.png");
		newBanana->assetsBox = newBanana->hitBox;
		JB_updateAsset(newBanana->assets, (JB_Asset) { .rect=&newBanana->assetsBox }, JB_AssetUpdate_rect);
		JB_GameObject* banana = Game.bananas;
		if (banana != NULL){
			while(banana->next) banana = banana->next;
			banana->next = newBanana;
		} else {
			Game.bananas = newBanana;
		}
	}

	if (spawnVines){
		JB_GameObject* mainGround = spawnExtra ? ground->next : ground;
		int ranVinesX = rand() % ( mainGround->hitBox.w - 160 );
		int ranVinesY = rand() % ( mainGround->hitBox.h - 160 );
		JB_GameObject* vinesObj = calloc(1, sizeof *vinesObj);
		vinesObj->hitBox.w = 160;
		vinesObj->hitBox.h = 160;
		vinesObj->hitBox.x = mainGround->hitBox.x + ranVinesX;
		vinesObj->hitBox.y = mainGround->hitBox.y + ranVinesY;
		vinesObj->next = ground;
		vinesObj->assets = JB_new_Image("assets/sprites/vines.png");
		vinesObj->assetsBox = vinesObj->hitBox;
		JB_updateAsset(vinesObj->assets, (JB_Asset) { .rect=&vinesObj->assetsBox }, JB_AssetUpdate_rect);
		ground = vinesObj;
	}

	current->next = ground;
	Game.data.round.counter++;
}

void JB_render_round() {
	char* s = calloc(12, sizeof *s);
	sprintf(s, "Points: %i", Game.data.round.counter);
	JB_updateAsset(Game.assetsHardcoded.pointCounter,
				   (JB_Asset) { .string=s },
				   JB_AssetUpdate_string);

	char* s2 = calloc(12, sizeof *s2);
	sprintf(s2, "Bananas: %i", Game.bananaScore);
	JB_updateAsset(Game.assetsHardcoded.bananaCounter,
				   (JB_Asset) { .string=s2 },
				   JB_AssetUpdate_string);

	JB_renderAssets(Game.assetsHardcoded.pointCounter);
	JB_renderAssets(Game.assetsHardcoded.bananaCounter);
	JB_renderAssets(Game.assets);

	/**
 	* Spieler rendern
 	*/
	{
		if (Game.data.round.showHitboxes){
			SDL_SetRenderDrawColor(Game.renderer, 0, 255, 0, 255);
			SDL_RenderDrawRect(Game.renderer, &Game.data.round.player->hitBox);
		}
		JB_updateAsset(Game.data.round.player->assets, (JB_Asset) { .rect=&Game.data.round.player->assetsBox },
					   JB_AssetUpdate_rect);
		JB_renderAssets(Game.data.round.player->assets);
	}
	/**
	 * GameObjects rendern
	 */
	JB_GameObject* currentObject = Game.gameObjects;
	while(currentObject) {
		JB_renderAssets(currentObject->assets);
		if (Game.data.round.showHitboxes){
			SDL_SetRenderDrawColor(Game.renderer, 0, 255, 0, 255);
			SDL_RenderDrawRect(Game.renderer, &currentObject->hitBox);
		}
		currentObject = currentObject->next;
	}
	/**
	 * Bananen rendern
	 */
	currentObject = Game.bananas;
	while(currentObject) {
		JB_renderAssets(currentObject->assets);
		if (Game.data.round.showHitboxes){
			SDL_SetRenderDrawColor(Game.renderer, 0, 255, 0, 255);
			SDL_RenderDrawRect(Game.renderer, &currentObject->hitBox);
		}
		currentObject = currentObject->next;
	}

}

void JB_handleEvents_round(SDL_Event* event) {
	if (!( event->type == SDL_KEYDOWN || event->type == SDL_KEYUP )) return;
	switch(event->key.keysym.sym) {
		case SDLK_a:
			Game.controls.aHeld = event->type == SDL_KEYDOWN;
			break;
		case SDLK_d:
			Game.controls.dHeld = event->type == SDL_KEYDOWN;
			break;
		case SDLK_SPACE:
			Game.controls.spaceHeld = event->type == SDL_KEYDOWN;
			break;
		case SDLK_ESCAPE:
			JB_changeModeToMenu(true);
			break;
		default:
			return;
	}
}

#pragma clang diagnostic pop