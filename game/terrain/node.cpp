#include "node.h"

using namespace game;

TerrainNode::TerrainNode()
{
}

TerrainNode::~TerrainNode()
{
}

void
TerrainNode::init(Terrain *parent, int x, int y)
{
    this->parent = parent;
    this->offset = y * parent->w + x;
}

bool
TerrainNode::is_passable(void)
const
{
    return true;
}
