#include "view.hpp"

vec2f worldToScreen(const vec2f& p, const View& v)
{
	vec2f half_size = v.size / 2.f;
	vec2f t;

	t = p - v.world_pos;
	t *= v.scale;
	t += half_size;
	t += v.screen_pos;

	return t;
	//return (p - v.world_pos) * v.scale + (v.size / 2.f) + v.screen_pos;
}
vec2f screenToWorld(const vec2f& p, const View& v)
{
	vec2f half_size = v.size / 2.f;
	vec2f t;

	t = p - half_size - v.screen_pos;
	t /= v.scale;
	t += v.world_pos;

	return t;
	//return (p - (v.size / 2.f) - v.screen_pos) / v.scale + v.world_pos;
}

//Primitive Drawing

void drawLine(const View& v, const vec2f& v1, const vec2f& v2, const ALLEGRO_COLOR& cl, float line_width)
{
	vec2f new_v1 = worldToScreen(v1, v);
	vec2f new_v2 = worldToScreen(v2, v);

	al_draw_line(new_v1.x, new_v1.y, new_v2.x, new_v2.y, cl, std::max(line_width * v.scale.x, 1.f));
}

void drawTriangle(const View& v, const vec2f& v1, const vec2f& v2, const vec2f v3, const ALLEGRO_COLOR& cl, float line_width)
{
	vec2f new_v1 = worldToScreen(v1, v);
	vec2f new_v2 = worldToScreen(v2, v);
	vec2f new_v3 = worldToScreen(v3, v);

	al_draw_triangle(new_v1.x, new_v1.y, new_v2.x, new_v2.y, new_v3.x, new_v3.y, cl, std::max(line_width * v.scale.x, 1.f));
}

void drawFilledTriangle(const View& v, const vec2f& v1, const vec2f& v2, const vec2f v3, const ALLEGRO_COLOR& cl)
{
	vec2f new_v1 = worldToScreen(v1, v);
	vec2f new_v2 = worldToScreen(v2, v);
	vec2f new_v3 = worldToScreen(v3, v);

	al_draw_filled_triangle(new_v1.x, new_v1.y, new_v2.x, new_v2.y, new_v3.x, new_v3.y, cl);
}

void drawRectangle(const View& v, const vec2f& tl, const vec2f& br, const ALLEGRO_COLOR& cl, float line_width)
{
	vec2f new_tl = worldToScreen(tl, v);
	vec2f new_br = worldToScreen(br, v);

	al_draw_rectangle(new_tl.x, new_tl.y, new_br.x, new_br.y, cl, std::max(line_width * v.scale.x, 1.f));
}

void drawFilledRectangle(const View& v, const vec2f& tl, const vec2f& br, const ALLEGRO_COLOR& cl)
{
	vec2f new_tl = worldToScreen(tl, v);
	vec2f new_br = worldToScreen(br, v);

	al_draw_filled_rectangle(new_tl.x, new_tl.y, new_br.x, new_br.y, cl);
}

void drawCircle(const View& v, const vec2f& c, const float& r, const ALLEGRO_COLOR& cl, float line_width)
{
	vec2f new_c = worldToScreen(c, v);

	al_draw_circle(new_c.x, new_c.y, r * v.scale.x, cl, std::max(line_width * v.scale.x, 1.f));
}

void drawFilledCircle(const View& v, const vec2f& c, const float& r, const ALLEGRO_COLOR& cl)
{
	vec2f new_c = worldToScreen(c, v);

	al_draw_filled_circle(new_c.x, new_c.y, r*v.scale.x, cl);
}

//Bitmap Drawing

void drawBitmap(const View& v, ALLEGRO_BITMAP* bmp, const vec2f& tl, int flags)
{
	float sw, sh;

	sw = al_get_bitmap_width(bmp);
	sh = al_get_bitmap_height(bmp);

	vec2f new_tl = worldToScreen(tl, v);

	al_draw_scaled_bitmap(bmp, 0, 0, sw, sh, new_tl.x, new_tl.y, sw * v.scale.x, sh * v.scale.y, 0);
}

void drawBitmapRegion(const View& v, ALLEGRO_BITMAP* bmp, const vec2f s_tl, const vec2f& s_dim, const vec2f& d_tl, int flags)
{
	drawTintedBitmapRegion(v, bmp, s_tl, s_dim, d_tl, al_map_rgb(255, 255, 255), flags);
}

void drawScaledBitmap(const View& v, ALLEGRO_BITMAP* bmp, const vec2f& tl, const vec2f& scale, int flags)
{
	float sw, sh;

	sw = al_get_bitmap_width(bmp);
	sh = al_get_bitmap_height(bmp);

	vec2f new_tl = worldToScreen(tl, v);

	al_draw_scaled_bitmap(bmp, 0, 0, sw, sh, new_tl.x, new_tl.y, sw * v.scale.x * scale.x, sh * v.scale.y * scale.y, 0);
}

void drawTintedBitmapRegion(const View& v, ALLEGRO_BITMAP* bmp, const vec2f s_tl, const vec2f& s_dim, const vec2f& d_tl, const ALLEGRO_COLOR& cl, int flags)
{
	vec2f new_tl = worldToScreen(d_tl, v);

	al_draw_tinted_scaled_rotated_bitmap_region(bmp, s_tl.x, s_tl.y, s_dim.x, s_dim.y,
		cl, 0, 0, new_tl.x, new_tl.y, v.scale.x, v.scale.y, 0, flags);
}