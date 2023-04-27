#ifndef _GAME_TERRAIN_H
#define _GAME_TERRAIN_H

#include "terrain/node.h"
#include "terrain/chunk.h"
#include "../gfx/3d/model.h"
#include "../gfx/3d/material.h"
#include "../gfx/3d/texture.h"
#include "../gfx/sprite.h"

#include <vector>

namespace game
{
    class Terrain
    {
    public:
        static const unsigned char
            SEA_LEVEL  = 0x06; // baseline in heightmaps
        
        static const int
            MAX_VISIBILITY = 75000, // Maximum visibility (m) in any weather condition (does not affect performance)
            MAX_HEIGHT     = 2000,
            MAX_DEPTH      = 25,
            BUILDING_STEM  = 5; // extra floors generated for buildings to account for possibly sloping terrain
        
        const int &w, &h;
        char *name;
        char *author;
        char *music;

        gfx::Material
            material;

        struct
        {
            char         *name;
            gfx::Sprite  *image;
            gfx::Texture *texture;
        } texture[4];

        // Terrain(int w, int h);
        Terrain(const char *filename);
        Terrain();
        ~Terrain();

        void
        load(const char *filename);

        // void
        // save(const char *filename);

        // void
        // resize(int w, int h);

        // void
        // reset(void);

        // Get interpolated (smooth) node values
        TerrainNode at(float x, float z);
        TerrainNode at(const math::Vec2 &v) { return this->at(v.x, v.y); }
        TerrainNode at(const math::Vec3 &v) { return this->at(v.x, v.z); }
        TerrainNode at(const math::Vec4 &v) { return this->at(v.x, v.z); }

        TerrainNode &
        get_node(int x, int z) { return this->data[z * this->w + x]; };
        // Return actual node data

        TerrainChunk &
        get_chunk(int x, int z);
        // Get 3D model for this area

        bool
        is_cached(int x, int z);
        // Test if a 3D model for this area exists yet

        TerrainNode &
        operator[](int i) { return this->data[i]; };

        // void
        // raise(float x, float z, float radius, float amount = 1.0f);

        // inline void
        // lower(float x, float z, float radius, float amount = 1.0f) { this->raise(x, z, radius, -amount); }

        // void
        // erode(float x, float z, float radius, float amount = 1.0);

        // void
        // smoothen(float x, float z, float radius, float amount = 1.0);

        void
        render(void);

        void
        prefetch(const math::Vec3 &pos, int radius = 0);
        // Silently preload one or more chunks at given position
        // Useful for making sure all assets are initialized beforehand

        const gfx::Sprite *
        minimap(int w, int h);
        
        typedef
            enum
            {
                ROCK,
                SHRUB,
                TREE,
                HOUSE,
                TOWNHOUSE,
                HIGHRISE,
                PROP_TYPE_COUNT
            }
            PropType;
        
        bool
        get_prop(gfx::Model **dst, PropType type);
        // Returns an array of models, from lowest to highest LOD level
        // Returns false if no models of that type are cached

    private:
        int
            w_mutable,
            h_mutable;
        
        TerrainNode
            *data;
        
        // Cached 3D meshes
        struct
        {
            // Chunk cache boundaries
            int
                west,
                east,
                north,
                south;
                
            // Grid of cached chunk meshes or NULLs
            TerrainChunk
                **cache;
        }
        chunks;
        
        gfx::Sprite
            *cached_minimap;

        void
        init(void);

        void
        reset(void);

        typedef
            std::vector<gfx::Model *>
            PropModelContainer;
        
        void
        preload_props(void);

        PropModelContainer prop_models[PROP_TYPE_COUNT];
    };
    
    extern Terrain terrain;
}


#endif
