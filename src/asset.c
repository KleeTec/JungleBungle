#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "include/asset.h"
#include "include/main.h"
#include "include/util.h"

/**
 * erstellt eine neue JB_Asset
 * @param renderer ==> JB_Game.renderer
 * @param string ==> ein Text
 * @param colour ==> Farbe des Textes
 * @param font ==> Schriftart des Textes
 * @return neue JB_Asset
 */
JB_Asset* JB_new_Text(char* string, SDL_Color colour, TTF_Font* font) {
	JB_Asset* asset = calloc(1, sizeof *asset);
	asset->string = string;
	asset->colour = colour;
	asset->font = font;
	if(font == NULL) font = JB_loadFont("assets/roboto.ttf", 24);
	SDL_Surface* surface = TTF_RenderText_Solid(font, string, colour);
	asset->texture = SDL_CreateTextureFromSurface(JB_Game.renderer, surface);
	SDL_FreeSurface(surface);
	return asset;
}

JB_Asset* JB_new_Image(char* path) {
	JB_Asset* asset = calloc(1, sizeof *asset);
	asset->texture = JB_loadImage(path);
	return asset;
}


/**
 * Lädt die Font je nach Speicherort
 * @param game ==> Referenz zum Spiel
 * @param path ==> relativer Pfad zur Font (z.B. assets/roboto.ttf)
 * @param size ==> Größe der Font
 * @return Referenz zur Font
 */
TTF_Font* JB_loadFont(char* path, int size) {
	TTF_Font* font = TTF_OpenFont(path, size);
	if(font == NULL) {
		font = TTF_OpenFont(appendChar("../", path), size);
	}
	return font;
}

/**
 * aktualisiert das Textur-Element.
 * @param asset ==> Referenz zum zu aktualisierenden Element
 * @param update ==> Struct an zu aktualisierenden Daten
 * @param updateFlags ==> welche Daten aus dem Struct aktualisiert werden sollen
 * @return sich selbst
 */
JB_Asset* JB_updateAsset(JB_Asset* asset, JB_Asset update, int updateFlags) {
	bool everything = updateFlags == JB_AssetUpdate_everything;
	if(everything || updateFlags & JB_AssetUpdate_string)
		asset->string = update.string;
	if(everything || updateFlags & JB_AssetUpdate_colour)
		asset->colour = update.colour;
	if(everything || updateFlags & JB_AssetUpdate_font)
		asset->font = update.font;
	if(everything || updateFlags & JB_AssetUpdate_fontFitRect)
		asset->fontFitRect = update.fontFitRect;
	if(everything || updateFlags & JB_AssetUpdate_next)
		asset->next = update.next;
	if(everything || updateFlags & JB_AssetUpdate_rect)
		asset->rect = update.rect;
	// Textur aktualisieren
	if(everything ||( asset->string && (
			updateFlags & JB_AssetUpdate_font ||
			updateFlags & JB_AssetUpdate_colour ||
			updateFlags & JB_AssetUpdate_string
	))){
		SDL_Surface* surface = TTF_RenderText_Solid(asset->font, asset->string, asset->colour);
		asset->texture = SDL_CreateTextureFromSurface(JB_Game.renderer, surface);
		SDL_FreeSurface(surface);
	}
	return asset;
}