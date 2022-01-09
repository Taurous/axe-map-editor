#include "tile_selector.hpp"

TileSelector::TileSelector(Map &map, View &view) : m(map), v(view)
{
	
}

void TileSelector::draw(vec2f pos, vec2f max_dim, float inset, float padding)
{
    vec2f tilemap_size, tilemap_scaled;

	tilemap_size = m.tilemap->tile_size * m.tilemap->size;

	// Scale horizontally first
	tilemap_scaled.x = max_dim.x - (padding * 2) - (inset * 2);
	tilemap_scaled.y = tilemap_size.y * (tilemap_scaled.x / tilemap_size.x);

	// If scaled tilemap height is greater than supplied max vertical dimension, scale down again to fit.
	if (tilemap_scaled.y > max_dim.y)
	{
		tilemap_scaled.y = max_dim.y - (padding * 2) - (inset * 2);
		tilemap_scaled.x = tilemap_size.x * (tilemap_scaled.y / tilemap_size.y);

		// Center horizontally
		pos.x += (max_dim.x / 2) - (tilemap_scaled.x / 2) - padding - inset;
	}

	al_draw_filled_rectangle(pos.x + inset, pos.y + inset, pos.x + tilemap_scaled.x + (padding * 2) + inset, pos.y + tilemap_scaled.y + (padding * 2) + inset, al_map_rgb(0, 0, 0));
	al_draw_scaled_bitmap(m.getTilemapBitmap(), 0, 0, tilemap_size.x, tilemap_size.y, pos.x + padding + inset, pos.y + padding + inset, tilemap_scaled.x, tilemap_scaled.y, 0);
}