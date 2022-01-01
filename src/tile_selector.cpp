#include "tile_selector.hpp"

void drawTileSelector(Map& m, View& v, vec2f pos, vec2f size, float padding, float bkg_padding)
{
	vec2f tsize, tscaled;

	tsize.x = al_get_bitmap_width(m.getTilemapBitmap());
	tsize.y = al_get_bitmap_height(m.getTilemapBitmap());

    pos.x += padding;
    pos.y += padding;

	tscaled.x = size.x-(padding*2);
	tscaled.y = tsize.y * (tscaled.x / tsize.x);

	al_draw_filled_rectangle(pos.x - bkg_padding, pos.y - bkg_padding, pos.x + tscaled.x + bkg_padding, pos.y + tscaled.y + bkg_padding, al_map_rgb(0, 0, 0));
	al_draw_scaled_bitmap(m.getTilemapBitmap(), 0, 0, tsize.x, tsize.y, pos.x, pos.y, tscaled.x, tscaled.y, 0);
}