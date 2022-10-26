#include <stdio.h>

#include "../include/main.h"

/**
 * Rendert die FPS in die Ecke des Bildschirms
 */
void JB_renderFPS() {
	char s[12];
	sprintf(s, "%.1f FPS", Game.fps);
	JB_updateAsset(Game.assetsHardcoded.fps, (JB_Asset) { .string=s }, JB_AssetUpdate_string);
	JB_renderAssets(Game.assetsHardcoded.fps);
}

void JB_renderAssets(JB_Asset* assets) {
	while(assets != NULL && assets->texture != NULL) {
		if(assets->string && assets->font && assets->fontFitRect) {
			SDL_Rect r = *assets->rect;
			if(assets->fontFitRect) TTF_SizeText(assets->font, assets->string, &r.w, &r.h);
			SDL_RenderCopy(Game.renderer, assets->texture, assets->clip, &r);
		}
		else SDL_RenderCopy(Game.renderer, assets->texture, assets->clip, assets->rect);
		assets = assets->next;
	}
}