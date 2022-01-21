#pragma once

#include <allegro5/allegro_primitives.h>

#include "vec.hpp"

struct View
{
	vec2d world_pos;
	vec2i screen_pos;
	vec2d scale;
	vec2i size;
};

vec2d worldToScreen(const vec2d& p, const View& v);
vec2d screenToWorld(const vec2d& p, const View& v);

void drawLine(const View& v, const vec2d& v1, const vec2d& v2, const ALLEGRO_COLOR& cl, double line_width);
void drawTriangle(const View& v, const vec2d& v1, const vec2d& v2, const vec2d v3, const ALLEGRO_COLOR& cl, double line_width);
void drawFilledTriangle(const View& v, const vec2d& v1, const vec2d& v2, const vec2d v3, const ALLEGRO_COLOR& cl);
void drawRectangle(const View& v, const vec2d& tl, const vec2d& br, const ALLEGRO_COLOR& cl, double line_width);
void drawFilledRectangle(const View& v, const vec2d& tl, const vec2d& br, const ALLEGRO_COLOR& cl);
void drawCircle(const View& v, const vec2d& c, const double& r, const ALLEGRO_COLOR& cl, double line_width);
void drawFilledCircle(const View& v, const vec2d& c, const double& r, const ALLEGRO_COLOR& cl);

void drawBitmap(const View& v, ALLEGRO_BITMAP* bmp, const vec2d& tl, int flags);
void drawBitmapRegion(const View& v, ALLEGRO_BITMAP* bmp, const vec2d s_tl, const vec2d& s_dim, const vec2d& d_tl, int flags);
void drawScaledBitmap(const View& v, ALLEGRO_BITMAP* bmp, const vec2d& tl, const vec2d& scale, int flags);
void drawTintedBitmapRegion(const View& v, ALLEGRO_BITMAP* bmp, const vec2d s_tl, const vec2d& s_dim, const vec2d& d_tl, const ALLEGRO_COLOR& cl, int flags);

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