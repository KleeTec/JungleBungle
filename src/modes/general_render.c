#include <stdio.h>

#include "../include/main.h"

/**
 * Rendert die FPS in die Ecke des Bildschirms
 * @param game ==> Referenz zum Spiel
 */
void JB_renderFPS() {
	char s[12];
	sprintf(s, "%.1f FPS", JB_Game.fps);
	JB_updateAsset(JB_Game.assetsHardcoded.fps, (JB_Asset) { .string=s }, JB_AssetUpdate_string);
	JB_renderAssets(JB_Game.assetsHardcoded.fps);
}

void JB_renderAssets(JB_Asset* assets) {
	while(assets != NULL && assets->texture != NULL) {
		if(assets->rect) {
			SDL_Rect r = *assets->rect;
			if(assets->fontFitRect) SDL_QueryTexture(assets->texture, NULL, NULL, &r.w, &r.h);
			SDL_RenderCopy(JB_Game.renderer, assets->texture, NULL, &r);
		}
		else SDL_RenderCopy(JB_Game.renderer, assets->texture, NULL, assets->rect);
		assets = assets->next;
	}
}