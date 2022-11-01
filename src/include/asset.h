#pragma once
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


#define JB_AssetUpdate_rect        0b00000001
#define JB_AssetUpdate_string      0b00000010
#define JB_AssetUpdate_next        0b00000100
#define JB_AssetUpdate_colour      0b00001000
#define JB_AssetUpdate_font        0b00010000
#define JB_AssetUpdate_fontFitRect 0b00100000
#define JB_AssetUpdate_centred     0b01000000
#define JB_AssetUpdate_clip        0b10000000
#define JB_AssetUpdate_everything  0b11111111

/**
 * Kann einzeln stehen, um ohne HitBox gerendert zu werden, kann aber auch Teil eines GameObjects sein.
 */
typedef struct JB_Asset {
	/**
	 * Die Ausmaße der Textur<br/>
	 * Kann auch NULL sein, weshalb ein Pointer notwendig ist.
	 */
	SDL_Rect* rect;
	/**
	 *
	 */
	SDL_Rect* clip;
	/**
	 * die tatsächliche SDL-Textur
	 */
	SDL_Texture* texture;

	char* string;
	/**
	 * verlinkte Liste zum nächsten Element
	 */
	struct JB_Asset* next;
	SDL_Colour colour;
	TTF_Font* font;
	bool fontFitRect;
	/**
	 * wenn true wird asset mittig gerendert
	 */
	bool centered;
} JB_Asset;


TTF_Font* JB_loadFont(char* path, int size);
JB_Asset* JB_new_Text(char* string, SDL_Color colour, TTF_Font* font);
JB_Asset* JB_new_Image(char* path);
JB_Asset* JB_updateAsset(JB_Asset* asset, JB_Asset update, int updateFlags);
void JB_renderAssets(JB_Asset* assets);
void JB_initMutex();