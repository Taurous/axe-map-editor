#include "view.hpp"

vec2d worldToScreen(const vec2d& p, const View& v)
{
	vec2d half_size = vec2d(v.size) / 2.0;
	vec2d t;

	t = p - v.world_pos;
	t *= v.scale;
	t += half_size;
	t += v.screen_pos;

	return t;
	//return (p - v.world_pos) * v.scale + (v.size / 2.f) + v.screen_pos;
}
vec2d screenToWorld(const vec2d& p, const View& v)
{
	vec2d half_size = vec2d(v.size) / 2.0;
	vec2d t;

	t = p - half_size - vec2d(v.screen_pos);
	t /= v.scale;
	t += v.world_pos;

	return t;
	//return (p - (v.size / 2.f) - v.screen_pos) / v.scale + v.world_pos;
}

//Primitive Drawing

void drawLine(const View& v, const vec2d& v1, const vec2d& v2, const ALLEGRO_COLOR& cl, double line_width)
{
	vec2d new_v1 = worldToScreen(v1, v);
	vec2d new_v2 = worldToScreen(v2, v);

	al_draw_line(new_v1.x, new_v1.y, new_v2.x, new_v2.y, cl, std::max(line_width * v.scale.x, 1.0));
}

void drawTriangle(const View& v, const vec2d& v1, const vec2d& v2, const vec2d v3, const ALLEGRO_COLOR& cl, double line_width)
{
	vec2d new_v1 = worldToScreen(v1, v);
	vec2d new_v2 = worldToScreen(v2, v);
	vec2d new_v3 = worldToScreen(v3, v);

	al_draw_triangle(new_v1.x, new_v1.y, new_v2.x, new_v2.y, new_v3.x, new_v3.y, cl, std::max(line_width * v.scale.x, 1.0));
}

void drawFilledTriangle(const View& v, const vec2d& v1, const vec2d& v2, const vec2d v3, const ALLEGRO_COLOR& cl)
{
	vec2d new_v1 = worldToScreen(v1, v);
	vec2d new_v2 = worldToScreen(v2, v);
	vec2d new_v3 = worldToScreen(v3, v);

	al_draw_filled_triangle(new_v1.x, new_v1.y, new_v2.x, new_v2.y, new_v3.x, new_v3.y, cl);
}

void drawRectangle(const View& v, const vec2d& tl, const vec2d& br, const ALLEGRO_COLOR& cl, double line_width)
{
	vec2d new_tl = worldToScreen(tl, v);
	vec2d new_br = worldToScreen(br, v);

	al_draw_rectangle(new_tl.x, new_tl.y, new_br.x, new_br.y, cl, std::max(line_width * v.scale.x, 1.0));
}

void drawFilledRectangle(const View& v, const vec2d& tl, const vec2d& br, const ALLEGRO_COLOR& cl)
{
	vec2d new_tl = worldToScreen(tl, v);
	vec2d new_br = worldToScreen(br, v);

	al_draw_filled_rectangle(new_tl.x, new_tl.y, new_br.x, new_br.y, cl);
}

void drawCircle(const View& v, const vec2d& c, const double& r, const ALLEGRO_COLOR& cl, double line_width)
{
	vec2d new_c = worldToScreen(c, v);

	al_draw_circle(new_c.x, new_c.y, r * v.scale.x, cl, std::max(line_width * v.scale.x, 1.0));
}

void drawFilledCircle(const View& v, const vec2d& c, const double& r, const ALLEGRO_COLOR& cl)
{
	vec2d new_c = worldToScreen(c, v);

	al_draw_filled_circle(new_c.x, new_c.y, r*v.scale.x, cl);
}

//Bitmap Drawing

void drawBitmap(const View& v, ALLEGRO_BITMAP* bmp, const vec2d& tl, int flags)
{
	double sw, sh;

	sw = al_get_bitmap_width(bmp);
	sh = al_get_bitmap_height(bmp);

	vec2d new_tl = worldToScreen(tl, v);

	al_draw_scaled_bitmap(bmp, 0, 0, sw, sh, new_tl.x, new_tl.y, sw * v.scale.x, sh * v.scale.y, 0);
}

void drawBitmapRegion(const View& v, ALLEGRO_BITMAP* bmp, const vec2d s_tl, const vec2d& s_dim, const vec2d& d_tl, int flags)
{
	drawTintedBitmapRegion(v, bmp, s_tl, s_dim, d_tl, al_map_rgb(255, 255, 255), flags);
}

void drawScaledBitmap(const View& v, ALLEGRO_BITMAP* bmp, const vec2d& tl, const vec2d& scale, int flags)
{
	double sw, sh;

	sw = al_get_bitmap_width(bmp);
	sh = al_get_bitmap_height(bmp);

	vec2d new_tl = worldToScreen(tl, v);

	al_draw_scaled_bitmap(bmp, 0, 0, sw, sh, new_tl.x, new_tl.y, sw * v.scale.x * scale.x, sh * v.scale.y * scale.y, 0);
}

void drawTintedBitmapRegion(const View& v, ALLEGRO_BITMAP* bmp, const vec2d s_tl, const vec2d& s_dim, const vec2d& d_tl, const ALLEGRO_COLOR& cl, int flags)
{
	vec2d new_tl = worldToScreen(d_tl, v);

	al_draw_tinted_scaled_rotated_bitmap_region(bmp, s_tl.x, s_tl.y, s_dim.x, s_dim.y,
		cl, 0, 0, new_tl.x, new_tl.y, v.scale.x, v.scale.y, 0, flags);
}