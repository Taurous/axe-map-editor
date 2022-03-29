#pragma once

#include "vec.hpp"
#include <list>

enum
{
    STR,    DEX,    CON,
    INT,    WIS,    CHA,
    MAX_STAT
};

struct Creature
{
    std::string name;
    uint8_t stats[MAX_STAT];
    uint8_t initiative;
};

inline int getStatModifier(uint8_t stat)
{
    return (static_cast<int>(stat) / 2) - 5;
}

std::string getStatAsString(int stat);

void generateRandomCreatureList(std::list<Creature>& creatures, int count, bool append = true);
void sortCreaturesByInitiative(std::list<Creature>& creatures);

bool renderInitiativeTracker(const vec2f& pos, const vec2f& size, std::list<Creature>& creatures);