#include "map.hpp"

#include <iostream> // for debugging
#include <iomanip>
#include <fstream>
#include <math.h> // floor

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#define cchar_cast(x) reinterpret_cast<const char*>(&x)
#define char_cast(x) reinterpret_cast<char*>(&x)

constexpr uint8_t MAGIC[] = {'M', 'D', 'F'};
constexpr uint16_t version = 0x0100;

void printFile(std::string path);

bool createMap(Map& m, std::string path, int ts)
{
	if (m.bmp != nullptr)
	{
		std::cerr << "Map: " << path << " already loaded." << std::endl;
		return true;
	}

	m.bmp = nullptr;
	m.width = 0;
	m.height = 0;
	m.path = path;
	m.tile_size = ts;

	m.bmp = al_load_bitmap(m.path.c_str());

	if (!m.bmp)
	{
		std::cerr << "Failed to load image: " << m.path << std::endl;
		return false;
	}

	m.width = al_get_bitmap_width(m.bmp) / m.tile_size;
	m.height = al_get_bitmap_height(m.bmp) / m.tile_size;

	m.v_tiles.resize(m.width * m.height, false);

	m.needs_save = false;

	return true;
}

void clearMap(Map& m)
{
	if (m.bmp)
	{
		al_destroy_bitmap(m.bmp);
		m.bmp = nullptr;
	}

	m.width = 0;
	m.height = 0;
	m.path = "";
	m.tile_size = 0;
	m.needs_save = false;

	m.v_tiles.clear();
}

bool reloadMap(Map& m)
{
	if (m.bmp)
	{
		al_destroy_bitmap(m.bmp);
		m.bmp = nullptr;
	}

	return createMap(m, m.path, m.tile_size);
}

bool saveMap(Map& m, std::string file, const View& v)
{
	//TODO Make this little-endian
	/*	File Format
		Data to Save:
			path to map image
			width
			height
			tile_size
			tile vector

			view position
			view scale
	*/

	std::ofstream out(file, std::ofstream::out | std::ofstream::binary);

	if (out.is_open())
	{
		//Write Magic Bits to identify file
		out.write(cchar_cast(MAGIC), sizeof(char)*3);
		//Write version of file
		out.write(cchar_cast(version), sizeof(version));

		//Write View position and scale
		out.write(cchar_cast(v.world_pos), sizeof(v.world_pos));
		out.write(cchar_cast(v.scale), sizeof(v.scale));

		//Write size of path, then path
		size_t path_sz = m.path.size();
		out.write(char_cast(path_sz), sizeof(path_sz));
		out.write(m.path.c_str(), path_sz);

		//Write Map data
		out.write(cchar_cast(m.width), sizeof(m.width));
		out.write(cchar_cast(m.height), sizeof(m.height));
		out.write(cchar_cast(m.tile_size), sizeof(m.tile_size));
		std::copy(m.v_tiles.begin(), m.v_tiles.end(), std::ostreambuf_iterator<char>(out));
		//out.write(reinterpret_cast<const char *>(m.v_tiles[0]), m.v_tiles.size() * sizeof(bool));

		out.close();

		std::cout << "Filed Saved!\n";

		m.needs_save = false;
		return true;
	}

	return false;
}

bool loadMap(Map& m, std::string file, View &v, bool restore_view)
{
	View temp_view;
	Map temp_map;

	std::ifstream in(file, std::ifstream::binary);

	if (in.is_open())
	{
		uint8_t buf[3];
		uint16_t r_version = 0;

		in.read(char_cast(buf), sizeof(char) * 3);
		in.read(char_cast(r_version), sizeof(r_version));

		bool correct_file = true;

		for (int i = 0; i < 3; ++i)
		{
			if (buf[i] != MAGIC[i]) correct_file = false;
		}

		if (correct_file && version == r_version)
		{
			in.read(char_cast(temp_view.world_pos), sizeof(temp_view.world_pos));
			in.read(char_cast(temp_view.scale), sizeof(temp_view.scale));

			size_t path_sz = 0;
			in.read(char_cast(path_sz), sizeof(path_sz));

			char path_buf[path_sz];
			in.read(path_buf, sizeof(char) * path_sz);
			temp_map.path = std::string(path_buf);

			in.read(char_cast(temp_map.width), sizeof(temp_map.width));
			in.read(char_cast(temp_map.height), sizeof(temp_map.height));
			in.read(char_cast(temp_map.tile_size), sizeof(temp_map.tile_size));

			temp_map.v_tiles = std::vector<bool>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());

			temp_map.bmp = m.bmp;
			if (m.bmp == nullptr) temp_map.bmp = al_load_bitmap(temp_map.path.c_str());
			m = temp_map;
			v.scale = temp_view.scale;
			v.world_pos = temp_view.world_pos;

			std::cout << "Filed Loaded!\n";

			m.needs_save = false;
			return true;
		}

		in.close();
	}

	return false;
}

void drawMap(const Map& m, const View& v, bool draw_grid, bool show_hidden)
{
	vec2i vis_tl, vis_br;
	getVisibleTileRect(m, v, vis_tl, vis_br);

	al_hold_bitmap_drawing(true);
	char back_col = 18;

	for (int x = vis_tl.x; x <= vis_br.x; ++x)
	{
		for (int y = vis_tl.y; y <= vis_br.y; ++y)
		{
			if (m.v_tiles[y * m.width + x]) drawBitmapRegion(v, m.bmp, vec2d(x * m.tile_size, y * m.tile_size), vec2d(m.tile_size, m.tile_size), vec2d(x * m.tile_size, y * m.tile_size), 0);
			else if (show_hidden) drawTintedBitmapRegion(v, m.bmp, vec2d(x * m.tile_size, y * m.tile_size), vec2d(m.tile_size, m.tile_size), vec2d(x * m.tile_size, y * m.tile_size), al_map_rgba(100, 100, 100, 100), 0);
			else
			{
				vec2d top_left(x * m.tile_size, y * m.tile_size);
				vec2d bottom_right = top_left + vec2d(m.tile_size, m.tile_size);
				drawFilledRectangle(v, top_left, bottom_right, al_map_rgb(back_col, back_col, back_col));
			}
		}
	}

	al_hold_bitmap_drawing(false);

	if (draw_grid)
	{
		for (int x = vis_tl.x; x <= vis_br.x + 1; ++x)
		{
			drawLine(v, vec2d(x * m.tile_size, vis_tl.y * m.tile_size), vec2d(x * m.tile_size, vis_br.y * m.tile_size + m.tile_size), al_map_rgb(40, 40, 40), 1);
		}

		for (int y = vis_tl.y; y <= vis_br.y + 1; ++y)
		{
			drawLine(v, vec2d(vis_tl.x * m.tile_size, y * m.tile_size), vec2d(vis_br.x * m.tile_size + m.tile_size, y * m.tile_size), al_map_rgb(40, 40, 40), 1);
		}
	}
}

void hideTile(Map &m, const vec2i& p)
{
	setTile(m, p, false);
}
void showTile(Map &m, const vec2i& p)
{
	setTile(m, p, true);
}
void setTile(Map &m, const vec2i& p, bool show)
{
	if (p.x >= 0 && p.x < m.width && p.y >= 0 && p.y < m.height)
	{
		int index = p.y * m.width + p.x;

		m.v_tiles[index] = show;

		m.needs_save = true;
	}
}
bool isTileShown(const Map& m, const vec2i& p)
{
	if (p.x >= 0 && p.x < m.width && p.y >= 0 && p.y < m.height)
	{
		int index = p.y * m.width + p.x;

		return m.v_tiles[index];
	}

	return false;
}
void getVisibleTileRect(const Map& m, const View& v, vec2i& tl, vec2i& br)
{
	tl.x = std::max((int)floor((v.world_pos.x - (v.size.x / 2 / v.scale.x)) / m.tile_size), 0);
	tl.y = std::max((int)floor((v.world_pos.y - (v.size.y / 2 / v.scale.y)) / m.tile_size), 0);

	br.x = std::min((int)floor((v.world_pos.x + (v.size.x / 2 / v.scale.x)) / m.tile_size), m.width - 1);
	br.y = std::min((int)floor((v.world_pos.y + (v.size.y / 2 / v.scale.y)) / m.tile_size), m.height - 1);
}

vec2i getTilePos(const Map& m, const View& v, const vec2d& screen_pos)
{
	vec2d p = screenToWorld(screen_pos, v);
	vec2i n;

	n.x = (int)floor(p.x / m.tile_size);
	n.y = (int)floor(p.y / m.tile_size);

	return n;
}

void printFile(std::string path)
{
	std::ifstream in(path, std::ifstream::in | std::ifstream::binary);
	
	if (in.is_open())
	{
		std::cout << "Reading File" << std::endl;
		char *buffer;

		in.seekg(0, in.end);
		long size = in.tellg();
		in.seekg(0);

		buffer = new char[size];

		in.read(buffer, size);

		std::cout << "Size: "  << size << " bytes\n";

		for (int i = 0; i < size; i+= 16)
		{
			int s = i + 16;
			for (int j = i; j < s && j < size; ++j)
			{
				std::cout << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(buffer[j] & 0xFF) << " ";
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}
}