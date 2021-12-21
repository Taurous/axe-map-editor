#include "play_state.hpp"

#include <allegro5/allegro_primitives.h>

#include "state_machine.hpp"
#include "input.hpp"

PlayState::PlayState(StateMachine& state_machine, InputHandler& input) : AbstractState(state_machine, input)
{
	map = std::unique_ptr<Map>(createMap(20, 20, std::move(std::unique_ptr<Tilemap>(loadTilemap("resources/tex/sheet.png", 32)))));

	_debug_font = al_load_font("C:/Windows/Fonts/arial.ttf", 18, 0);
}

PlayState::~PlayState()
{
	
}

void PlayState::pause() {}
void PlayState::resume() {}

void PlayState::handleEvents(const ALLEGRO_EVENT &ev)
{
	int tile_size = map->tilemap->tile_size;

	int x = m_input.getMouseX() / tile_size;
	int y = m_input.getMouseY() / tile_size;

	if (x > 0 && x < map->width - 1 && y > 0 && y < map->height - 1)
	{
		int tile = y * map->width + x;
		if (m_input.isMouseDown(MOUSE::LEFT))
		{
			if (isTileSolid(map, tile))
			{
				map->tile_array[tile].solid = false; // Set slected tile to floor
				map->tile_array[tile].walls.reset();

				std::vector<vec2i> tiles_to_check;
				tiles_to_check.push_back({  0, -1 });
				tiles_to_check.push_back({  1, -1 });
				tiles_to_check.push_back({  1,  0 });
				tiles_to_check.push_back({  1,  1 });
				tiles_to_check.push_back({  0,  1 });
				tiles_to_check.push_back({ -1,  1 });
				tiles_to_check.push_back({ -1,  0 });
				tiles_to_check.push_back({ -1, -1 });

				bool skip_next = false;
				int count = 0;
				for (auto &v : tiles_to_check)
				{
					if (skip_next) { skip_next = false; continue; }

					tile = (y + v.y) * map->width + (x + v.x);

					if (isTileSolid(map, tile))
					{
						map->tile_array[tile].walls.set(count, true); // If tile is not floor, add correct wall texture to existing wall textures.
					}
					else skip_next = true;

					++count;
				}
			}
		}
		else if (m_input.isMouseDown(MOUSE::RIGHT))
		{
			/*if (m_floor_array[tile])
			{
				m_floor_array[tile] = false; // Set slected tile to floor
				m_wall_array[tile].reset();

				vec2i topleft(x - 1, y - 1);
				vec2i bottomright(x + 1, y + 1);
			
				int tile_count = 0;
				for (int yy = topleft.y; yy <= bottomright.y; ++yy) // Look at every tile around selected tile
				{
					for (int xx = topleft.x; xx <= bottomright.x; ++xx)
					{
						if (xx == x && yy == y) continue;

						if (!m_floor_array[yy * m_map_width + xx])
						{
							m_wall_array[yy * m_map_width + xx].set(tile_count, false); // If tile is not floor, add correct wall texture to existing wall textures.
						}
						else
						{
							m_wall_array[tile].set(7 - (size_t)tile_count, true);
						}

						++tile_count;
					}
				}
			}*/
		}
	}
}

void PlayState::update(float delta_time)
{

}

void PlayState::draw()
{
	drawMap(map, 0, 0);

	int x = m_input.getMouseX() / map->tilemap->tile_size;
	int y = m_input.getMouseY() / map->tilemap->tile_size;

	size_t tile = y * map->width + x;

	if (tile >= 0 && tile < map->width * map->height)
	{
		al_draw_text(_debug_font, al_map_rgb(255, 0, 255), 16, 16, 0, map->tile_array[tile].walls.to_string().c_str());
		al_draw_textf(_debug_font, al_map_rgb(255, 0, 255), 16, 32, 0, "%i, %i", x, y);
	}

	/*if (x > 0 && x < m_map_width - 1 && y > 0 && y < m_map_height - 1)
	{
		int x1, x2, y1, y2;

		x1 = m_map_offset_x + (x * m_floor_tilemap->tile_size);
		x2 = x1 + m_floor_tilemap->tile_size + 1;
		y1 = m_map_offset_y + (y * m_floor_tilemap->tile_size);
		y2 = y1 + m_floor_tilemap->tile_size;

		//al_draw_rectangle(x1, y1, x2, y2, al_map_rgb(255, 0, 0), 2);

		al_draw_line(x1, y1, x1 + 4, y1, al_map_rgb(255, 0, 0), 1);
		al_draw_line(x1, y1-1, x1, y1 + 4, al_map_rgb(255, 0, 0), 1);

		al_draw_line(x1, y2, x1 + 4, y2, al_map_rgb(255, 0, 0), 1);
		al_draw_line(x1, y2, x1, y2 - 4, al_map_rgb(255, 0, 0), 1);

		al_draw_line(x2, y1, x2 - 4, y1, al_map_rgb(255, 0, 0), 1);
		al_draw_line(x2, y1, x2, y1 + 4, al_map_rgb(255, 0, 0), 1);

		al_draw_line(x2, y2, x2 - 4, y2, al_map_rgb(255, 0, 0), 1);
		al_draw_line(x2, y2, x2, y2 - 4, al_map_rgb(255, 0, 0), 1);
	}*/
}