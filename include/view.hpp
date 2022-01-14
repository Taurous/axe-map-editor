#pragma once

#include <allegro5/allegro_primitives.h>

#include "vec.hpp"

class View
{
public:
	View() : world_pos({ 0, 0 }), screen_pos({ 0, 0 }), scale({ 1, 1 }), size({ 0, 0 }) { }
	~View() {}

	vec2f world_pos;
	vec2f screen_pos;
	vec2f scale;
	vec2f size;

private:
};

vec2f worldToScreen(const vec2f& p, const View& v);
vec2f screenToWorld(const vec2f& p, const View& v);

void drawLine(const View& v, const vec2f& v1, const vec2f& v2, const ALLEGRO_COLOR& cl, float line_width);
void drawTriangle(const View& v, const vec2f& v1, const vec2f& v2, const vec2f v3, const ALLEGRO_COLOR& cl, float line_width);
void drawFilledTriangle(const View& v, const vec2f& v1, const vec2f& v2, const vec2f v3, const ALLEGRO_COLOR& cl);
void drawRectangle(const View& v, const vec2f& tl, const vec2f& br, const ALLEGRO_COLOR& cl, float line_width);
void drawFilledRectangle(const View& v, const vec2f& tl, const vec2f& br, const ALLEGRO_COLOR& cl);
void drawCircle(const View& v, const vec2f& c, const float& r, const ALLEGRO_COLOR& cl, float line_width);
void drawFilledCircle(const View& v, const vec2f& c, const float& r, const ALLEGRO_COLOR& cl);

void drawBitmap(const View& v, ALLEGRO_BITMAP* bmp, const vec2f& tl, int flags);
void drawBitmapRegion(const View& v, ALLEGRO_BITMAP* bmp, const vec2f s_tl, const vec2f& s_dim, const vec2f& d_tl, int flags);
void drawScaledBitmap(const View& v, ALLEGRO_BITMAP* bmp, const vec2f& tl, const vec2f& scale, int flags);
void drawTintedBitmapRegion(const View& v, ALLEGRO_BITMAP* bmp, const vec2f s_tl, const vec2f& s_dim, const vec2f& d_tl, const ALLEGRO_COLOR& cl, int flags);

/*
* Bitmap Drawing
* 
al_draw_tinted_bitmap
al_draw_bitmap_region
al_draw_tinted_bitmap_region
al_draw_rotated_bitmap
al_draw_tinted_rotated_bitmap
al_draw_scaled_rotated_bitmap
al_draw_tinted_scaled_rotated_bitmap
al_draw_tinted_scaled_rotated_bitmap_region
al_draw_tinted_scaled_bitmap
*/