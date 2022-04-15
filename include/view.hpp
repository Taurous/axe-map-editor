#pragma once

#include <allegro5/allegro_primitives.h>

#include "vec.hpp"

namespace View
{
	struct ViewPort
	{
		vec2d world_pos;
		vec2i screen_pos;
		double scale;
		vec2i size;
	};

	vec2d worldToScreen(const vec2d& p, const ViewPort& v);
	vec2d screenToWorld(const vec2d& p, const ViewPort& v);

	void scaleRelativeToPoint(ViewPort& v, const vec2d& p, const double& scale);

	void drawLine(const ViewPort& v, const vec2d& v1, const vec2d& v2, const ALLEGRO_COLOR& cl, double line_width);
	void drawTriangle(const ViewPort& v, const vec2d& v1, const vec2d& v2, const vec2d v3, const ALLEGRO_COLOR& cl, double line_width);
	void drawFilledTriangle(const ViewPort& v, const vec2d& v1, const vec2d& v2, const vec2d v3, const ALLEGRO_COLOR& cl);
	void drawRectangle(const ViewPort& v, const vec2d& tl, const vec2d& br, const ALLEGRO_COLOR& cl, double line_width);
	void drawFilledRectangle(const ViewPort& v, const vec2d& tl, const vec2d& br, const ALLEGRO_COLOR& cl);
	void drawCircle(const ViewPort& v, const vec2d& c, const double& r, const ALLEGRO_COLOR& cl, double line_width);
	void drawFilledCircle(const ViewPort& v, const vec2d& c, const double& r, const ALLEGRO_COLOR& cl);

	void drawBitmap(const ViewPort& v, ALLEGRO_BITMAP* bmp, const vec2d& tl, int flags);
	void drawBitmapRegion(const ViewPort& v, ALLEGRO_BITMAP* bmp, const vec2d s_tl, const vec2d& s_dim, const vec2d& d_tl, int flags);
	void drawScaledBitmap(const ViewPort& v, ALLEGRO_BITMAP* bmp, const vec2d& tl, const vec2d& scale, int flags);
	void drawTintedBitmapRegion(const ViewPort& v, ALLEGRO_BITMAP* bmp, const vec2d s_tl, const vec2d& s_dim, const vec2d& d_tl, const ALLEGRO_COLOR& cl, int flags);
};