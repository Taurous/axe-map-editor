#include "map.hpp"

#include <algorithm>
#include <iostream> // for debugging
#include <iomanip>
#include <math.h> // floor

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include  <tinyxml2.h>

namespace xml = tinyxml2;

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
		std::cerr << "Failed to load map: " << m.path << std::endl;
		return false;
	}

	m.width = al_get_bitmap_width(m.bmp) / m.tile_size;
	m.height = al_get_bitmap_height(m.bmp) / m.tile_size;

	m.v_tiles.resize(m.width * m.height, true);

	std::cout << "createMap(m, " << path << ", " << ts << ") results:" << "\n\tWidth: " << m.width << "\n\tHeight: " << m.height
		<< "\n\tTile Size: " << m.tile_size << "\n\tv_tiles.size(): " << (unsigned int)m.v_tiles.size() << std::endl;

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

bool saveMap(const Map& m, std::string file, const View& v)
{
	/*
	 XML STRUCTURE
	<Map>
		<View>
			<Position x="float" y="float"/>
			<Scale x="float" y="float"/>
		</View>

		<Tiles total="int">
			<Tile textureid="int">
				<Position x="int" y="int"/>
			</Tile>
		</Tiles>

	</Map>
	

	xml::XMLDocument saveFile;
	xml::XMLNode* root = saveFile.NewElement("Map");
	saveFile.InsertFirstChild(root);

	//Save View Data
	xml::XMLElement* viewElement = saveFile.NewElement("View");
	xml::XMLElement* viewPositionElement = saveFile.NewElement("Position");
	viewPositionElement->SetAttribute("x", v.world_pos.x);
	viewPositionElement->SetAttribute("y", v.world_pos.y);
	xml::XMLElement* viewScaleElement = saveFile.NewElement("Scale");
	viewScaleElement->SetAttribute("x", v.scale.x);
	viewScaleElement->SetAttribute("y", v.scale.y);
	viewElement->InsertEndChild(viewPositionElement);
	viewElement->InsertEndChild(viewScaleElement);
	root->InsertEndChild(viewElement);

	//Dump Tiles into XML
	xml::XMLElement* tileTagElement = saveFile.NewElement("Tiles");
	tileTagElement->SetAttribute("count", (unsigned int)v_tiles.size());
	for (auto& v : v_tiles)
	{
		xml::XMLElement* tileElement = saveFile.NewElement("Tile");
		tileElement->SetAttribute("textureid", v.id);

		xml::XMLElement* posElement = saveFile.NewElement("Position");
		posElement->SetAttribute("x", v.pos.x);
		posElement->SetAttribute("y", v.pos.y);

		tileElement->InsertEndChild(posElement);
		tileTagElement->InsertEndChild(tileElement);
	}
	root->InsertEndChild(tileTagElement);

	xml::XMLError res = saveFile.SaveFile(file.c_str());
	if (res != xml::XML_SUCCESS)
	{
		std::cerr << "Failed to save map data!" << std::endl;
		return false;
	}

	return true;
	*/
	return false;
}

bool loadMap(Map& m, std::string file, View &v, bool restore_view)
{
	/*
	 XML STRUCTURE
	<Map>
		<View>
			<Position x="float" y="float"/>
			<Scale x="float" y="float"/>
		</View>
		<Tilemap>
			<Path>string</Path>
			<TileSize x="int" y="int"/>
		</Tilemap>
		<Tiles total="int">
			<Tile textureid="int">
				<Position x="int" y="int"/>
			</Tile>
		</Tiles>
	</Map>
	

	//Array of error just so I don't have to keep instatiating XMLError objects
	xml::XMLError res[4];
	std::fill_n(res, 4, xml::XML_SUCCESS);
	auto failure = [](const std::string& e)
	{
		std::cerr << e << std::endl;
		return false;
	};

	//Load XML File
	xml::XMLDocument loadFile;
	res[0] = loadFile.LoadFile(file.c_str());
	if (res[0] != xml::XML_SUCCESS) return failure(std::string("Failed to load: ") + file);

	//Get Root node
	xml::XMLNode* root = loadFile.FirstChild();
	if (root == nullptr) return failure("Map data is corrupt!");

	Map m; // Temporary map, copy into this after loading is completed successfully
	vec2i tile_size; // Same as above
	std::string tilemap_path; // Same as above

	//Get View Node
	xml::XMLElement* viewElement = root->FirstChildElement("View");
	if (viewElement != nullptr)
	{
		//Load View Position
		xml::XMLElement* viewPositionElement = viewElement->FirstChildElement("Position");
		if (viewPositionElement == nullptr) return failure("View position not found!");
		res[0] = viewPositionElement->QueryFloatAttribute("x", &v.world_pos.x);
		res[1] = viewPositionElement->QueryFloatAttribute("y", &v.world_pos.y);
		if (res[0] != xml::XML_SUCCESS || res[1] != xml::XML_SUCCESS) return failure("View data is corrupt!");

		//Load View Scale
		xml::XMLElement* viewScaleElement = viewElement->FirstChildElement("Scale");
		if (viewScaleElement == nullptr) return failure("View scale not found!");
		res[0] = viewScaleElement->QueryFloatAttribute("x", &v.scale.x);
		res[1] = viewScaleElement->QueryFloatAttribute("y", &v.scale.y);
		if (res[0] != xml::XML_SUCCESS || res[1] != xml::XML_SUCCESS) return failure("View data is corrupt!");
	}

	//Get Tilemap Node
	xml::XMLElement* tilemapElement = root->FirstChildElement("Tilemap");
	if (tilemapElement == nullptr) return failure("Tilemap data not found!");

	//Load Tilemap Path
	xml::XMLElement* tilemapPathElement = tilemapElement->FirstChildElement("Path");
	if (tilemapPathElement == nullptr) return failure("Tilemap path not found!");
	const char* string_exists = nullptr;
	string_exists = tilemapPathElement->GetText();
	if (string_exists != nullptr) tilemap_path = string_exists;

	//Load Tilemap Tile Size
	xml::XMLElement* tilemapSizeElement = tilemapElement->FirstChildElement("TileSize");
	if (tilemapSizeElement == nullptr) return failure("Tilemap size not found!");
	res[0] = tilemapSizeElement->QueryIntAttribute("x", &tile_size.x);
	res[1] = tilemapSizeElement->QueryIntAttribute("y", &tile_size.y);
	if (res[0] != xml::XML_SUCCESS || res[1] != xml::XML_SUCCESS) return failure("Tilemap data is corrupt!");

	//Load Tiles into vector
	int count = 0;
	xml::XMLElement* tilesElement = root->FirstChildElement("Tiles");
	if (tilesElement != nullptr)
	{
		res[0] = tilesElement->QueryIntAttribute("count", &count);

		xml::XMLElement* tileElement = tilesElement->FirstChildElement("Tile");
		while (tileElement != nullptr)
		{
			Tile t;

			//Load Texture ID
			res[0] = tileElement->QueryIntAttribute("textureid", &t.id);

			//Load Tile Position
			xml::XMLElement* tilePositionElement = tileElement->FirstChildElement("Position");
			res[1] = tilePositionElement->QueryIntAttribute("x", &t.pos.x);
			res[2] = tilePositionElement->QueryIntAttribute("y", &t.pos.y);

			if (res[0] != xml::XML_SUCCESS || res[1] != xml::XML_SUCCESS || res[2] != xml::XML_SUCCESS) std::cout << "Tile data corrupt, removing tile." << std::endl;
			else m.v_tiles.push_back(t);

			tileElement = tileElement->NextSiblingElement("Tile");
		}
	}

	//If total tiles read is equal to the number stored in the Tiles tag then load is probably corrent.
	if ((unsigned int)count == m.v_tiles.size())
	{
		if (!restore_view)
		{
			//Set view to defaults
			v.world_pos = { 0, 0 };
			v.scale = { 1, 1 };
		}

		// Create Tilemap/Map Object
		setTilemap(tilemap_path, tile_size);

		//Clear Current Map
		v_tiles.clear();
		it_visible_begin = v_tiles.begin();

		//Copy Temporary Map to this->v_tiles
		v_tiles.resize(m.v_tiles.size());
		std::copy(m.v_tiles.begin(), m.v_tiles.end(), v_tiles.begin());

		sortMapVisibily(v);

		editable = true;
		return true;
	}
	else return failure("Tile data is corrupt!");
	*/
	return false;
}

void drawMap(const Map& m, const View& v, bool draw_grid)
{
	vec2i vis_tl, vis_br;
	getVisibleTileRect(m, v, vis_tl, vis_br);

	al_hold_bitmap_drawing(true);

	for (int x = vis_tl.x; x <= vis_br.x; ++x)
	{
		for (int y = vis_tl.y; y <= vis_br.y; ++y)
		{
			if (m.v_tiles[y * m.width + x]) drawBitmapRegion(v, m.bmp, vec2f{x * m.tile_size, y * m.tile_size}, vec2f{m.tile_size, m.tile_size}, vec2f{x * m.tile_size, y * m.tile_size}, 0);
		}
	}

	al_hold_bitmap_drawing(false);

	if (draw_grid)
	{
		for (int x = vis_tl.x; x <= vis_br.x + 1; ++x)
		{
			drawLine(v, {x * m.tile_size, vis_tl.y * m.tile_size}, {x * m.tile_size, vis_br.y * m.tile_size + m.tile_size}, al_map_rgb(40, 40, 40), 1);
		}

		for (int y = vis_tl.y; y <= vis_br.y + 1; ++y)
		{
			drawLine(v, {vis_tl.x * m.tile_size, y * m.tile_size}, {vis_br.x * m.tile_size + m.tile_size, y * m.tile_size}, al_map_rgb(40, 40, 40), 1);
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

vec2i getTilePos(const Map& m, const View& v, const vec2f& screen_pos)
{
	vec2f p = screenToWorld(screen_pos, v);
	vec2i n;

	n.x = (int)floor(p.x / m.tile_size);
	n.y = (int)floor(p.y / m.tile_size);

	return n;
}


