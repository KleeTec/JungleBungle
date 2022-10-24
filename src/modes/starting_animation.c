#include <math.h>

#include <SDL2/SDL.h>

#include "../include/game_logic.h"
#include "../include/main.h"


void JB_render_startingAnimation() {
	JB_renderAssets(JB_Game.assetsHardcoded.background);

	switch(JB_Game.mode.startAnimation.state) {
		case 0:
			JB_render_startingAnimation_StageOne();
			break;
		case 1:
			JB_render_startingAnimation_StageTwo();
			break;
	}
}

void JB_render_startingAnimation_StageOne() {
	double f = 0.8 * sqrt((double) JB_Game.data.startAnimation.counter / 1000) + 1;
	int w = (int) ( 1200 * f );
	int h = (int) ( 300 * f );
	SDL_Rect titleRect = { ( JB_Game.windowSize.width - w ) / 2, ( JB_Game.windowSize.height - h ) / 2, w, h };

	int alpha = (int) ( 255 * ( JB_Game.data.startAnimation.counter / ( MAX_FPS * 2 )));
	if(alpha < 255) SDL_SetTextureAlphaMod(JB_Game.assetsHardcoded.title->texture, alpha);

	JB_updateAsset(JB_Game.assetsHardcoded.title, (JB_Asset) { .rect = &titleRect }, JB_AssetUpdate_rect);
	JB_renderAssets(JB_Game.assetsHardcoded.title);

	JB_Game.data.startAnimation.counter++;
	if(JB_Game.data.startAnimation.counter >= MAX_FPS * 3) {
		JB_Game.data.startAnimation.counter = 0;
		JB_Game.mode.startAnimation.state = 1;
	}
}

void JB_render_startingAnimation_StageTwo() {
	double f = ((double) JB_Game.data.startAnimation.counter / 100 ) *
			   ((double) JB_Game.data.startAnimation.counter / 100 ) * 800 + 1;
	int y = ( JB_Game.windowSize.height - 402 ) / 2;
	SDL_Rect titleRect = { ( JB_Game.windowSize.width - 1607 ) / 2, y - (int) f, 1607, 402 };

	JB_updateAsset(JB_Game.assetsHardcoded.title, (JB_Asset) { .rect = &titleRect }, JB_AssetUpdate_rect);
	JB_renderAssets(JB_Game.assetsHardcoded.title);

	JB_Game.data.startAnimation.counter += 2;
	if(JB_Game.data.startAnimation.counter >= MAX_FPS * 2) JB_changeModeToMenu();
}

void JB_handleEvents_startingAnimation(SDL_Event* event) {
	switch(event->type) {
		case SDL_KEYDOWN:
			if(event->key.keysym.sym == SDLK_RETURN)
				JB_changeModeToMenu();
			break;
	}
}