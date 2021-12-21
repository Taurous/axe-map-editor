#include "map.hpp"

#include <algorithm>
#include <iostream> // for debugging
#include <iomanip>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include  "tinyxml2.h"

namespace xml = tinyxml2;

Map::Map() : editable(false)
{
	it_visible_begin = v_tiles.begin();
	tilemap = std::unique_ptr<Tilemap>(new Tilemap);
}

Map::~Map()
{
	destroyTilemap(tilemap.release());
}

void Map::draw(const View& v, bool grid)
{
	vec2f ts = tilemap->tile_size;
	ALLEGRO_BITMAP* target = tilemap->bmp;

	if (target)
	{
		al_hold_bitmap_drawing(true);
		for (std::vector<Tile>::iterator it = it_visible_begin; it != v_tiles.end(); ++it)
		{
			vec2f tilemap_pos((*it).id % (int)tilemap->size.x, (*it).id / (int)tilemap->size.x);
			vec2f map_pos = (*it).pos;
			map_pos *= ts;

			drawBitmapRegion(v, target, tilemap_pos * ts, ts, map_pos, 0);
		}
		al_hold_bitmap_drawing(false);
	}

	if (grid)
	{
		vec2i vis_tl, vis_br;

		getVisibleTileRect(v, vis_tl, vis_br);

		vec2f off((float)vis_tl.x * ts.x, (float)vis_tl.y * ts.y);
		off = worldToScreen(off, v);

		for (float x = off.x; x < v.size.x; x += ts.x * v.scale.x)al_draw_line(x, v.screen_pos.y, x, v.screen_pos.y + v.size.y, al_map_rgb(60, 60, 60), 1);
		for (float y = off.y; y < v.size.y; y += ts.y * v.scale.y)al_draw_line(v.screen_pos.x, y, v.screen_pos.x + v.size.x, y, al_map_rgb(60, 60, 60), 1);
	}
}

void Map::sortMapVisibilty(const View& v)
{
	/*
	* Map Visibilty Sort
	* Sort all visible tiles to end of vector
	* 'it_visible_begin' will point to first visible element
	*/

	auto pred = [this, &v](const Tile& t)
	{
		vec2i vis_tl, vis_br;

		getVisibleTileRect(v, vis_tl, vis_br);

		vec2f ft(t.pos);

		return !(ft.x >= vis_tl.x && ft.x <= vis_br.x && ft.y >= vis_tl.y && ft.y <= vis_br.y);
	};

	it_visible_begin = std::partition(v_tiles.begin(), v_tiles.end(), pred);
}

void Map::getVisibleTileRect(const View& v, vec2i& tl, vec2i& br)
{
	vec2f ts = tilemap->tile_size;
	tl.x = (int)floor((v.world_pos.x - (v.size.x / 2 / v.scale.x)) / ts.x);
	tl.y = (int)floor((v.world_pos.y - (v.size.y / 2 / v.scale.y)) / ts.y);

	br.x = (int)floor((v.world_pos.x + (v.size.x / 2 / v.scale.x)) / ts.x);
	br.y = (int)floor((v.world_pos.y + (v.size.y / 2 / v.scale.y)) / ts.y);
}

void Map::setTilemap(std::string path, vec2i tile_size)
{
	destroyTilemap(tilemap.release());
	tilemap.reset(loadTilemap(path, tile_size));
}

void Map::insertTile(Tile t)
{
	auto e = std::find_if(v_tiles.begin(), v_tiles.end(), [&t](Tile v_t) {return (v_t.pos == t.pos); });

	if (e == v_tiles.end())
	{
		// Tile does not exist, add to end of vector
		v_tiles.push_back(t);
	}
	else
	{
		//Tile exists, overwrite with new tile
		(*e) = t;
	}
}

void Map::removeTile(const vec2i& p)
{
	auto e = std::find_if(it_visible_begin, v_tiles.end(), [&p](Tile v_t) {return (v_t.pos == p); });

	if (e != v_tiles.end())
	{
		v_tiles.erase(e);
	}
}

vec2i Map::getTilePos(const View& v, const vec2f& screen_pos)
{
	vec2f p = screenToWorld(screen_pos, v);
	vec2i n;

	n.x = (int)floor(p.x / tilemap->tile_size.x);
	n.y = (int)floor(p.y / tilemap->tile_size.y);

	return n;
}

Tile Map::getTile(const vec2i& p)
{
	Tile t;
	t.pos = p;

	auto e = std::find_if(v_tiles.begin(), v_tiles.end(), [&t](Tile v_t) {return (v_t.pos == t.pos); });

	if (e == v_tiles.end())
	{
		// Tile does not exist, return default tile -1
		return Tile{ -1, {0, 0} };
	}
	else
	{
		//Tile exists
		return (*e);
	}
}

bool Map::save(std::string file, const View& v)
{
	/* XML STRUCTURE
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
	*/

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

	//Save Tilemap Data
	xml::XMLElement* tilemapElement = saveFile.NewElement("Tilemap");
	xml::XMLElement* tilemapPathElement = saveFile.NewElement("Path");
	tilemapPathElement->SetText(tilemap->path.c_str());
	xml::XMLElement* tilemapSizeElement = saveFile.NewElement("TileSize");
	tilemapSizeElement->SetAttribute("x", tilemap->tile_size.x);
	tilemapSizeElement->SetAttribute("y", tilemap->tile_size.y);
	tilemapElement->InsertEndChild(tilemapPathElement);
	tilemapElement->InsertEndChild(tilemapSizeElement);
	root->InsertEndChild(tilemapElement);

	//Dump Tiles into XML
	xml::XMLElement* tileTagElement = saveFile.NewElement("Tiles");
	tileTagElement->SetAttribute("count", v_tiles.size());
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
}

bool Map::load(std::string file, View &v, bool restore_view)
{
	/* XML STRUCTURE
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
	*/

	//Array of error just do I don't have to keep instatiating XMLError objects
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
	if (count == m.v_tiles.size())
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

		sortMapVisibilty(v);

		editable = true;
		return true;
	}
	else return failure("Tile data is corrupt!");
}

void Map::create(std::string tilemap_path, vec2i tile_size)
{
	v_tiles.clear();
	it_visible_begin = v_tiles.begin();
	setTilemap(tilemap_path, tile_size);
	if (tilemap) editable = true;
}
