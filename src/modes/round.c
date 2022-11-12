#include <SDL2/SDL.h>
#include <stdlib.h>
#include <sys/time.h>
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

	// TODO: Spieler an Bildschirmgröße anpassen
	static JB_GameObject player = {};
	player.hitBox.w = 132;
	player.hitBox.h = 112;
	player.hitBox.x = ( Game.windowSize.w - player.hitBox.w ) / 2;
	player.hitBox.y = ( Game.windowSize.h - player.hitBox.h ) / 2;
	player.assets = JB_new_Image("assets/sprites/player.png");
	static SDL_Rect size = {};
	size.x = player.hitBox.x;
	size.y = player.hitBox.y;
	size.w = 132;
	size.h = 112;
	JB_updateAsset(player.assets, (JB_Asset) { .rect=&size },
				   JB_AssetUpdate_rect);
	Game.data.round.player = &player;

	Game.modeType = JB_MODE_ROUND;
}

void JB_generateBlock() {
	int ranX = 300 + rand() % 600;
	int ranY = rand() % 200;
	bool top = rand() & 1;
	bool extra = rand() & 1;
	bool vines = rand() & 1;

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
			!top && current->hitBox.y + ranY + ground->hitBox.h < Game.windowSize.h ?
			current->hitBox.y + ranY :
			current->hitBox.y - ranY;
	ground->assets = JB_new_Image("assets/sprites/ground.png");
	JB_updateAsset(ground->assets, (JB_Asset) { .rect=&ground->hitBox }, JB_AssetUpdate_rect);

	if (extra){
		int ranExtraX = rand() % ( ground->hitBox.w - 80 );
		JB_GameObject* extraObj = calloc(1, sizeof *extraObj);
		extraObj->hitBox.w = 80;
		extraObj->hitBox.h = 64;
		extraObj->hitBox.x = ground->hitBox.x + ranExtraX;
		extraObj->hitBox.y = ground->hitBox.y - extraObj->hitBox.h;
		extraObj->next = ground;
		extraObj->assets = JB_new_Image("assets/sprites/extra.png");
		JB_updateAsset(extraObj->assets, (JB_Asset) { .rect=&extraObj->hitBox }, JB_AssetUpdate_rect);
		ground = extraObj;
	}

	if (vines){
		JB_GameObject* mainGround = extra ? ground->next : ground;
		int ranVinesX = rand() % ( mainGround->hitBox.w - 160 );
		int ranVinesY = rand() % ( mainGround->hitBox.h - 160 );
		JB_GameObject* vinesObj = calloc(1, sizeof *vinesObj);
		vinesObj->hitBox.w = 160;
		vinesObj->hitBox.h = 160;
		vinesObj->hitBox.x = mainGround->hitBox.x + ranVinesX;
		vinesObj->hitBox.y = mainGround->hitBox.y + ranVinesY;
		vinesObj->next = ground;
		vinesObj->assets = JB_new_Image("assets/sprites/vines.png");
		JB_updateAsset(vinesObj->assets, (JB_Asset) { .rect=&vinesObj->hitBox }, JB_AssetUpdate_rect);
		ground = vinesObj;
	}

	current->next = ground;
}

void JB_render_round() {
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
