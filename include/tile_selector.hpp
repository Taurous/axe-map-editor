#pragma once

#include "view.hpp"
#include "map.hpp"

class TileSelector
{
public:
    TileSelector(Map &map, View &view);
    ~TileSelector() { }

    void draw(vec2f pos, vec2f max_dim, float inset, float padding);

private:
    Map &m;
    View &v; 
};