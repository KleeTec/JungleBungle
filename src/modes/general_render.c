#include <stdio.h>

#include "../include/main.h"

/**
 * Rendert die FPS in die Ecke des Bildschirms
 */
void JB_renderFPS() {
	char* s = calloc(12, sizeof *s);
	sprintf(s, "%.1f FPS", Game.fps);
	JB_updateAsset(Game.assetsHardcoded.fps,
				   (JB_Asset) { .string=s },
				   JB_AssetUpdate_string);
	JB_renderAssets(Game.assetsHardcoded.fps);
}

void JB_renderAssets(JB_Asset* assets) {
	while(assets != NULL && assets->texture != NULL) {
		if(assets->string && assets->font && assets->fontFitRect) {
			SDL_Rect r = *assets->rect;
			int w = r.w, h = r.h;

			if (assets->fontFitRect) TTF_SizeText(assets->font, assets->string, &r.w, &r.h);
			if (assets->centered.x) r.x += (w - r.w) / 2;
			if (assets->centered.y) r.y += (h - r.h) / 2;

			SDL_RenderCopy(Game.renderer, assets->texture, assets->clip, &r);
		}
		else SDL_RenderCopy(Game.renderer, assets->texture, assets->clip, assets->rect);
		assets = assets->next;
	}
}


/**
 * Wenn über ein Objekt drüber gehalten wird, wird .hover=rue gesetzt.
 */
void JB_setHover(enum JB_ModeType modeType, SDL_Event* event) {
	JB_Button* object = Game.buttons[modeType];
	while(object != NULL) {
		if(event->motion.x >= object->rect.x &&
		   event->motion.y >= object->rect.y &&
		   event->motion.x <= object->rect.x + object->rect.w &&
		   event->motion.y <= object->rect.y + object->rect.h)
			object->hover = true;
		else object->hover = false;
		object = object->next;
	}
}