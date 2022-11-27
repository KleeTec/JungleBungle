#include <stdio.h>

#include "../include/main.h"
#include "../include/util.h"

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
	long currentTime = currentTimeMillis();
	while(assets != NULL && assets->texture != NULL) {
		// Ist die Clip-Größe auf 0 gesetzt, wird die Ressource voll gerendert, also der Pointer,
		// der SDL übergeben wird, auf NULL gesetzt
		SDL_Rect* p;
		if (assets->clipSize.w == 0 || assets->clipSize.h == 0) p = NULL;
		else p = &assets->clipSize;
		// Andernfalls wird die x-Koordinate des Clips auf die richtige Stelle gepackt, und der
		// Clipindex gegebenenfalls erhöht
		if (assets->timePerClip > 0 && currentTime - assets->clipStartTime >= assets->timePerClip){
			assets->clipIndex++;
			assets->clipSize.x += assets->clipSize.w * assets->clipIndex;
			if (assets->clipSize.x > assets->maxClips * assets->clipSize.w) assets->clipSize.x = 0;
			if (assets->clipIndex > assets->maxClips) assets->clipIndex = 0;
			assets->clipStartTime = currentTime;
		}

		if (assets->string && assets->font && assets->fontFitRect){
			SDL_Rect r = *assets->rect;
			int w = r.w, h = r.h;

			if (assets->fontFitRect) TTF_SizeText(assets->font, assets->string, &r.w, &r.h);
			if (assets->centered.x) r.x += ( w - r.w ) / 2;
			if (assets->centered.y) r.y += ( h - r.h ) / 2;


			SDL_RenderCopy(Game.renderer, assets->texture, p, &r);
		} else SDL_RenderCopy(Game.renderer, assets->texture, p, assets->rect);
		assets = assets->next;
	}
}


/**
 * Wenn über ein Objekt drüber gehalten wird, wird .hover=true gesetzt.
 */
void JB_setHover(enum JB_ModeType modeType, SDL_Event* event) {
	JB_Button* object = Game.buttons[modeType];
	while(object != NULL) {
		if (event->motion.x >= object->rect.x &&
			event->motion.y >= object->rect.y &&
			event->motion.x <= object->rect.x + object->rect.w &&
			event->motion.y <= object->rect.y + object->rect.h)
			object->hover = true;
		else object->hover = false;
		object = object->next;
	}
}