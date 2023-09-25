#include "view.hpp"

namespace View
{
	vec2d worldToScreen(const vec2d& p, const ViewPort& v)
	{
		return ((p - v.world_pos) * v.scale) + (vec2d(v.size) / 2.0) + vec2d(v.screen_pos);
	}
	vec2d screenToWorld(const vec2d& p, const ViewPort& v)
	{
		return ((p - (vec2d(v.size) / 2.0) - vec2d(v.screen_pos)) / v.scale) + v.world_pos;
	}

	void scaleRelativeToPoint(ViewPort& v, const vec2d& p, const double& scale)
	{
		vec2d prev_pos = screenToWorld(p, v);
		v.scale += scale;
		v.world_pos += prev_pos - screenToWorld(p, v);
	}

	//Primitive Drawing

	void drawLine(const ViewPort& v, const vec2d& v1, const vec2d& v2, const ALLEGRO_COLOR& cl, double line_width)
	{
		vec2d new_v1 = worldToScreen(v1, v);
		vec2d new_v2 = worldToScreen(v2, v);

		//al_draw_line(new_v1.x, new_v1.y, new_v2.x, new_v2.y, cl, std::max(line_width * v.scale, 1.0)); Vary width with scale
		al_draw_line(new_v1.x, new_v1.y, new_v2.x, new_v2.y, cl, line_width);
	}

	void drawTriangle(const ViewPort& v, const vec2d& v1, const vec2d& v2, const vec2d v3, const ALLEGRO_COLOR& cl, double line_width)
	{
		vec2d new_v1 = worldToScreen(v1, v);
		vec2d new_v2 = worldToScreen(v2, v);
		vec2d new_v3 = worldToScreen(v3, v);

		al_draw_triangle(new_v1.x, new_v1.y, new_v2.x, new_v2.y, new_v3.x, new_v3.y, cl, std::max(line_width * v.scale, 1.0));
	}

	void drawFilledTriangle(const ViewPort& v, const vec2d& v1, const vec2d& v2, const vec2d v3, const ALLEGRO_COLOR& cl)
	{
		vec2d new_v1 = worldToScreen(v1, v);
		vec2d new_v2 = worldToScreen(v2, v);
		vec2d new_v3 = worldToScreen(v3, v);

		al_draw_filled_triangle(new_v1.x, new_v1.y, new_v2.x, new_v2.y, new_v3.x, new_v3.y, cl);
	}

	void drawRectangle(const ViewPort& v, const vec2d& tl, const vec2d& br, const ALLEGRO_COLOR& cl, double line_width)
	{
		vec2d new_tl = worldToScreen(tl, v);
		vec2d new_br = worldToScreen(br, v);

		al_draw_rectangle(new_tl.x, new_tl.y, new_br.x, new_br.y, cl, std::max(line_width * v.scale, 1.0));
	}

	void drawFilledRectangle(const ViewPort& v, const vec2d& tl, const vec2d& br, const ALLEGRO_COLOR& cl)
	{
		vec2d new_tl = worldToScreen(tl, v);
		vec2d new_br = worldToScreen(br, v);

		al_draw_filled_rectangle(new_tl.x, new_tl.y, new_br.x, new_br.y, cl);
	}

	void drawCircle(const ViewPort& v, const vec2d& c, const double& r, const ALLEGRO_COLOR& cl, double line_width)
	{
		vec2d new_c = worldToScreen(c, v);

		al_draw_circle(new_c.x, new_c.y, r * v.scale, cl, std::max(line_width * v.scale, 1.0));
	}

	void drawFilledCircle(const ViewPort& v, const vec2d& c, const double& r, const ALLEGRO_COLOR& cl)
	{
		vec2d new_c = worldToScreen(c, v);

		al_draw_filled_circle(new_c.x, new_c.y, r*v.scale, cl);
	}

	//Bitmap Drawing

	void drawBitmap(const ViewPort& v, ALLEGRO_BITMAP* bmp, const vec2d& tl, int flags)
	{
		double sw, sh;

		sw = al_get_bitmap_width(bmp);
		sh = al_get_bitmap_height(bmp);

		vec2d new_tl = worldToScreen(tl, v);

		al_draw_scaled_bitmap(bmp, 0, 0, sw, sh, new_tl.x, new_tl.y, sw * v.scale, sh * v.scale, flags);
	}

	void drawBitmapRegion(const ViewPort& v, ALLEGRO_BITMAP* bmp, const vec2d s_tl, const vec2d& s_dim, const vec2d& d_tl, int flags)
	{
		drawTintedBitmapRegion(v, bmp, s_tl, s_dim, d_tl, al_map_rgb(255, 255, 255), flags);
	}

	void drawScaledBitmap(const ViewPort& v, ALLEGRO_BITMAP* bmp, const vec2d& tl, const vec2d& scale, int flags)
	{
		double sw, sh;

		sw = al_get_bitmap_width(bmp);
		sh = al_get_bitmap_height(bmp);

		vec2d new_tl = worldToScreen(tl, v);

		al_draw_scaled_bitmap(bmp, 0, 0, sw, sh, new_tl.x, new_tl.y, sw * v.scale * scale.x, sh * v.scale * scale.y, flags);
	}

	void drawTintedBitmapRegion(const ViewPort& v, ALLEGRO_BITMAP* bmp, const vec2d s_tl, const vec2d& s_dim, const vec2d& d_tl, const ALLEGRO_COLOR& cl, int flags)
	{
		vec2d new_tl = worldToScreen(d_tl, v);

		al_draw_tinted_scaled_rotated_bitmap_region(bmp, s_tl.x, s_tl.y, s_dim.x, s_dim.y,
			cl, 0, 0, new_tl.x, new_tl.y, v.scale, v.scale, 0, flags);
	}
};