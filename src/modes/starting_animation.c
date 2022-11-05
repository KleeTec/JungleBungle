#include <math.h>

#include <SDL2/SDL.h>

#include "../include/game_logic.h"
#include "../include/main.h"


void JB_render_startingAnimation() {
	switch(Game.mode.startAnimation.state) {
		case 0:
			JB_render_startingAnimation_StageOne();
			break;
		case 1:
			JB_render_startingAnimation_StageTwo();
			break;
	}
}

void JB_render_startingAnimation_StageOne() {
	double f = 0.8 * sqrt((double) Game.data.startAnimation.counter / 1000) + 1;
	int w = (int) ( 1200 * ( f + 0.01 ));
	int h = (int) ( 300 * ( f + 0.01 ));
	SDL_Rect titleRect = { ( Game.windowSize.w - w ) / 2, ( Game.windowSize.h - h ) / 2, w, h };

	int alpha = (int) ( 255 * ( Game.data.startAnimation.counter / ( JB_MAX_FPS * 2 )));
	if(alpha < 255) SDL_SetTextureAlphaMod(Game.assetsHardcoded.title->texture, alpha);

	JB_updateAsset(Game.assetsHardcoded.title, (JB_Asset) { .rect = &titleRect }, JB_AssetUpdate_rect);
	JB_renderAssets(Game.assetsHardcoded.title);

	Game.data.startAnimation.counter++;
	if(Game.data.startAnimation.counter >= JB_MAX_FPS * 3) Game.mode.startAnimation.state = 1;
}

void JB_render_startingAnimation_StageTwo() {
	double f = 0.8 * sqrt((double) Game.data.startAnimation.counter / 1000) + 1;
	double f2 = Game.data.startAnimation.counter * ( 5.0 / 3.0 );

	int w = (int) ( 1200 * f );
	int h = (int) ( 300 * f );
	double y = (double) ( Game.windowSize.h - h ) / 2 - ( 300 - f2 );
	SDL_Rect titleRect = { ( Game.windowSize.w - w ) / 2, (int) y, w, h };

	JB_updateAsset(Game.assetsHardcoded.title, (JB_Asset) { .rect = &titleRect }, JB_AssetUpdate_rect);
	JB_renderAssets(Game.assetsHardcoded.title);

	Game.data.startAnimation.counter -= 5;
	if(f <= 1) JB_changeModeToMenu(false);
}

void JB_handleEvents_startingAnimation(SDL_Event* event) {
	switch(event->type) {
		case SDL_KEYDOWN:
			if(event->key.keysym.sym == SDLK_RETURN) {
				SDL_SetTextureAlphaMod(Game.assetsHardcoded.title->texture, 255);
				JB_changeModeToMenu(false);
			}
			break;
	}
}