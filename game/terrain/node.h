#ifndef _GAME_TERRAIN_NODE_H
#define _GAME_TERRAIN_NODE_H

namespace game
{
    class Terrain;

    class TerrainNode
    {
    public:
        static const int
            SIZE     = 100; // grid cell size in square metres
        
        static const int
            TEXTURES = 4;
        
        float
            height,
            vegetation,
            texture[TEXTURES];

        TerrainNode();
        ~TerrainNode();

        void
        init(Terrain *parent, int x, int y);

        bool
        is_passable(void) const;

    private:
        Terrain *parent;
        int offset;
    };
}

#include "../terrain.h"

#endif
