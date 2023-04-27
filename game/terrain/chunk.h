#ifndef _GAME_TERRAIN_CHUNK_H
#define _GAME_TERRAIN_CHUNK_H

#include "../../math/mat4.h"
#include "../../gfx/3d/mesh.h"
#include "../../gfx/3d/model.h"

#include <vector>

namespace game
{
    class Terrain;

    class TerrainChunk
    {
    public:
        static const int
            SIZE           = 500, // chunk size in metres
            LOD_LEVELS     = 3,
            VEGETATION_LOD = 8;

        class Prop
        {
        public:
            math::Mat4
                transformation,
                scaled;

            gfx::Model
                *model[LOD_LEVELS];

            Prop();
        };

        typedef
            std::vector<Prop *>
            Props;
        
        Props
            props;
        
        gfx::Mesh
            *mesh,
            *trees[VEGETATION_LOD];
        
        int tree_count[VEGETATION_LOD];
        
        TerrainChunk();
        TerrainChunk(int x, int z);
        ~TerrainChunk();
        
        void
        generate(int x, int z);
        
        void
        generate_forest(int x, int z);
        
        void
        render(float dist, float lod) const;

    private:
        void
        init();
    };
}

#include "../terrain.h"

#endif
