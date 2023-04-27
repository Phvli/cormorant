#include "terrain.h"

#define TERRAIN_DIR "../data/locations/"

#include "../core/engine.h"
#include "../core/util/string.h"
#include "../core/util/file.h"
#include "../math/vec4.h"
#include "../math/util.h"
#include "../gfx/sprite.h"
#include "../gfx/3d/scenery/all.h"

#include <cmath>
#include <cstdlib>   // rand
#include <algorithm> // min, max

using namespace game;

Terrain game::terrain;

Terrain::Terrain():
    w(w_mutable),
    h(h_mutable)
{
    this->init();
}

Terrain::Terrain(const char *filename):
    w(w_mutable),
    h(h_mutable)
{
    this->init();
    this->load(filename);
}

void
Terrain::init(void)
{
    this->data           = NULL;
    this->chunks.cache   = NULL;
    this->name           = NULL;
    this->author         = NULL;
    this->music          = NULL;
    this->w_mutable      = 0;
    this->h_mutable      = 0;
    this->cached_minimap = NULL;
    
    for (int i = 0; i < 4; ++i)
    {
        this->texture[i].name  = NULL;
        this->texture[i].image = NULL;
    }
    this->material.color_map    = NULL;
    this->material.normal_map   = NULL;
    this->material.bump_map     = NULL;
    this->material.specular_map = NULL;
}

Terrain::~Terrain()
{
    this->reset();
}

void
Terrain::reset(void)
{
    if (this->chunks.cache != NULL)
    {
        for (int i =
            (1 + this->chunks.east - this->chunks.west) *
            (1 + this->chunks.south - this->chunks.north) - 1; i >= 0; --i)
        {
            delete this->chunks.cache[i];
        }
        delete[] this->chunks.cache;
    }

    delete[] this->data;
    delete[] this->name;
    delete[] this->author;
    delete[] this->music;
    
    for (int i = 0; i < 4; ++i)
    {
        delete[] this->texture[i].name;
        delete   this->texture[i].image;
        delete   this->texture[i].texture;
    }

    delete this->cached_minimap;
    
    for (int i = 0; i < PROP_TYPE_COUNT; ++i)
    {
        for (PropModelContainer::iterator m = this->prop_models[i].begin();
            m != this->prop_models[i].end(); ++m)
        {
            delete *m;
        }

        this->prop_models[i].clear();
    }
    
    this->init();
}

const gfx::Sprite *
Terrain::minimap(int w, int h)
{
    if (this->cached_minimap == NULL
    || (this->cached_minimap->w != w && this->cached_minimap->h != h))
    {
        delete this->cached_minimap;

        this->cached_minimap = new gfx::Sprite(w, h);
        
        float
            x_scale = (float)this->w * TerrainNode::SIZE / w,
            y_scale = 0.03f * (float)((this->w + this->h) / 2) / (Terrain::MAX_HEIGHT + Terrain::MAX_DEPTH),
            z_scale = (float)this->h * TerrainNode::SIZE / h;

        for (int x = 0; x < w; ++x)
        {
            TerrainNode above = this->at(x * x_scale, 0.0f);

            for (int z = 0; z < h; ++z)
            {
                TerrainNode node = this->at(x * x_scale, z * z_scale);
                gfx::Color color;
                
                if (node.height <= 0.0f)
                {
                    // Water
                    color = gfx::RGB_color(
                        math::rnd(0x48, 0x50),
                        math::rnd(0x58, 0x68),
                        math::rnd(0x70, 0x98)
                    );
                }
                else
                {
                    // Sample ground texture
                    color = gfx::blend::ratio(0x00000000,
                            this->texture[0].image->get_random(),
                            node.texture[0])

                        + gfx::blend::ratio(0x00000000,
                            this->texture[1].image->get_random(),
                            node.texture[1])

                        + gfx::blend::ratio(0x00000000,
                            this->texture[2].image->get_random(),
                            node.texture[2])

                        + gfx::blend::ratio(0x00000000,
                            this->texture[3].image->get_random(),
                            node.texture[3]);
                    
                    // Cliff shadows
                    float height_difference = y_scale * (node.height - above.height);
                    if (height_difference > 0.0f)
                    {
                        color = gfx::blend::ratio(color, 0x000000,
                            math::max(0.0f, height_difference * .8f));
                    }
                }

                this->cached_minimap->putpixel(x, z, color);
                
                above = node;
            }
        }
    }
    
    // Single-color border to make clamped textures look less bad
    this->cached_minimap->drawrect(
        0, 0, this->cached_minimap->w - 1, this->cached_minimap->h - 1,
        gfx::RGB_color(
            math::rnd(0x48, 0x50),
            math::rnd(0x58, 0x68),
            math::rnd(0x70, 0x98)));
    
    return this->cached_minimap;
}

// void
// Terrain::resize(int w, int h)
// {
//     if (w == this->w && h == this->h)
//     {
//         return;
//     }

//     delete[] this->data;

//     this->data      = new TerrainNode[w * h];
//     this->w_mutable = w;
//     this->h_mutable = h;

//     this->reset();
// }

// void
// Terrain::reset(void)
// {
//     TerrainNode *end = this->data + this->w * this->h;
//     for (TerrainNode *c = this->data; c < end; ++c)
//     {
//         c->init(this, c - this->data);
//         c->height = 0;
//     };
// }

// void
// Terrain::raise(float x, float y, float radius, float amount)
// {
//     int
//     x_start = (int)x - (int)radius,
//     y_start = (int)y - (int)radius,
//     x_end   = (int)x + (int)radius,
//     y_end   = (int)y + (int)radius;

//     if (x_start < 0) x_start = 0;
//     if (y_start < 0) y_start = 0;
//     if (x_end >= this->w) x_end = this->w - 1;
//     if (y_end >= this->h) y_end = this->h - 1;

//     for (int y_dst = y_start; y_dst <= y_end; ++y_dst)
//     {
//         TerrainNode *c = this->data + y_dst * this->w + x_start;
//         for (int x_dst = x_start; x_dst <= x_end; ++x_dst)
//         {
//             float
//             x2 = x - x_dst,
//             y2 = y - y_dst,
//             r  = std::sqrt(x2 * x2 + y2 * y2);

//             if (r <= radius)
//             {
//                 float f = amount * std::cos(r * 1.5707965f / radius);
//                 c->height += f;
//             }

//             c++;
//         }
//     }
// }

// void
// Terrain::erode(float x, float y, float radius, float amount)
// {
//     int
//     x_start = (int)x - (int)radius,
//     y_start = (int)y - (int)radius,
//     x_end   = (int)x + (int)radius,
//     y_end   = (int)y + (int)radius;

//     if (x_start < 0) x_start = 0;
//     if (y_start < 0) y_start = 0;
//     if (x_end >= this->w) x_end = this->w - 1;
//     if (y_end >= this->h) y_end = this->h - 1;

//     for (int y_dst = y_start; y_dst <= y_end; ++y_dst)
//     {
//         TerrainNode *c = this->data + y_dst * this->w + x_start;
//         for (int x_dst = x_start; x_dst <= x_end; ++x_dst)
//         {
//             float
//             x2 = x - x_dst,
//             y2 = y - y_dst,
//             r  = std::sqrt(x2 * x2 + y2 * y2);

//             if (r <= radius)
//             {
//                 float f = amount * std::cos(r * 1.5707965f / radius) - 0.5f;
//                 c->height += f * (float)rand() / RAND_MAX;
//             }

//             c++;
//         }
//     }
// }


// void
// Terrain::smoothen(float x, float y, float radius, float amount)
// {
//     int
//     x_start = (int)x - (int)radius,
//     y_start = (int)y - (int)radius,
//     x_end   = (int)x + (int)radius,
//     y_end   = (int)y + (int)radius;

//     if (x_start < 0) x_start = 0;
//     if (y_start < 0) y_start = 0;
//     if (x_end >= this->w) x_end = this->w - 1;
//     if (y_end >= this->h) y_end = this->h - 1;

//     float target = this->get(x, y)->height;

//     for (int y_dst = y_start; y_dst <= y_end; ++y_dst)
//     {
//         TerrainNode *c = this->data + y_dst * this->w + x_start;
//         for (int x_dst = x_start; x_dst <= x_end; ++x_dst)
//         {
//             float
//             x2 = x - x_dst,
//             y2 = y - y_dst,
//             r  = std::sqrt(x2 * x2 + y2 * y2);

//             if (r <= radius)
//             {
//                 float f = amount * std::cos(r * 1.5707965f / radius);
//                 if (f > 1.0f) f = 1.0f; else if (f < 0.0f) f = 0.0f;

//                 c->height = c->height * (1.0f - f) + target * f;
//             }

//             c++;
//         }
//     }
// }

// void
// Terrain::save(const char *filename)
// {
//     char *path = core::str::format("../data/world/maps/%s.map", filename);
//     core::File *file = new core::File(path);
//     delete[] path;

//     file->write_magic("rMAP1.0");
//     file->write_uint16(this->w);
//     file->write_uint16(this->h);
//     file->write_string(this->name);
//     file->write_string(this->author);
//     file->write_string(this->music);

//     TerrainNode *end = this->data + this->w * this->h;
//     for (TerrainNode *c = this->data; c < end; ++c)
//     {
//         file->write_float(c->height);
//         file->write_uint16(c->texture);
//         file->write_uint16(c->object);
//     }

//     delete file;
// }

// void
// Terrain::load(const char *filename)
// {
//     char *path = core::str::format("../data/world/maps/%s.map", filename);
//     core::File *file = new core::File(path);
//     delete[] path;

//     if (!file->exists())
//     {
//         throw 666;
//     }

//     file->test_magic("rMAP1.0");
//     this->resize(file->read_uint16(), file->read_uint16());

//     core::str::set(this->name, file->read_string());
//     core::str::set(this->author, file->read_string());
//     core::str::set(this->music, file->read_string());

//     TerrainNode *end = this->data + this->w * this->h;
//     for (TerrainNode *c = this->data; c < end; ++c)
//     {
//         c->height  = file->read_float();
//         c->texture = file->read_uint16();
//         c->object  = file->read_uint16();
//     }

//     delete file;
// }

TerrainNode
Terrain::at(float x, float z)
{
    int
        int_x = (int)(x /= (float)TerrainNode::SIZE),
        int_z = (int)(z /= (float)TerrainNode::SIZE);

    int_x = std::max(0, std::min(this->w - 2, int_x));
    int_z = std::max(0, std::min(this->h - 2, int_z));
    
    float
        r_x  = 1.0f - (x -= int_x),
        r_z  = 1.0f - (z -= int_z),
        f_nw = r_x * r_z,
        f_ne = x   * r_z,
        f_sw = r_x * z,
        f_se = x   * z;
    
    TerrainNode
        *nw = this->data + int_z * this->w + int_x,
        *ne = nw + 1,
        *sw = nw + this->w,
        *se = ne + this->w,
        result;
    
    result.height = math::interpolate::cosine(
        math::interpolate::cosine(nw->height, ne->height, x),
        math::interpolate::cosine(sw->height, se->height, x),
        z
    );

    // Interpolate vegetation coefficient linearly
    result.vegetation =
        f_nw * nw->vegetation + f_ne * ne->vegetation +
        f_sw * sw->vegetation + f_se * se->vegetation;
    
    // Interpolate texture weights linearly
    for (int i = 0; i < TerrainNode::TEXTURES; ++i)
    {
        result.texture[i] =
            f_nw * nw->texture[i] + f_ne * ne->texture[i] +
            f_sw * sw->texture[i] + f_se * se->texture[i];
    }
    
    return result;
}

void
Terrain::load(const char *filename)
{
    core::engine.log("Loading terrain %s", filename);
    this->reset();
    
    char
        *ext = core::str::get_extension(filename),
        *dir = core::str::cat(TERRAIN_DIR, filename);

    dir[core::str::len(dir) - core::str::len(ext) - 1] = '\0';
    
    char
        *heightmap_path  = core::str::format("%s/terrain.%s", dir, ext),
        *vegetation_path = core::str::format("%s/vegetation.%s", dir, ext);

    gfx::Sprite
        *heightmap     = new gfx::Sprite(heightmap_path),
        *vegetation    = new gfx::Sprite(vegetation_path);
    
    if (heightmap->data == NULL)
    {
        core::engine.log("(!) Failed to load %s", heightmap_path);
    }
    delete[] heightmap_path;

    if (vegetation->data == NULL)
    {
        core::engine.log("(!) Failed to load %s", vegetation_path);
        vegetation->resize(1, 1);
        vegetation->clear(0x80808080);
    }
    delete[] vegetation_path;
    
    if (heightmap->data != NULL)
    {
        this->w_mutable = heightmap->w;
        this->h_mutable = heightmap->h;
        this->data      = new TerrainNode[this->w * this->h];
        
        TerrainNode *dst = this->data;
        for (int z = 0; z < heightmap->h; ++z)
        {
            int veg_z = (5 * z * vegetation->h / heightmap->h)
                % vegetation->h;
            for (int x = 0; x < heightmap->w; ++x)
            {
                int veg_x = (5 * x * vegetation->w / heightmap->w)
                    % vegetation->w;

                dst->init(this, x, z);

                dst->height
                    = (gfx::blend::bw(heightmap->getpixel(x, z)) & 0xff)
                    - (float)Terrain::SEA_LEVEL;
                
                // abyss at world's edge
                dst->height = math::transition::ease_out(
                    -10.0f, dst->height,
                    .01f * math::min(
                        heightmap->w / 2 - 10 - abs(x - heightmap->w / 2),
                        heightmap->h / 2 - 10 - abs(z - heightmap->h / 2)
                    )
                );
                
                dst->vegetation = (gfx::blend::bw(
                    vegetation->getpixel(veg_x, veg_z)) & 0xff) / 255.0f;
                
                // Saturate vegetation => more distinct areas
                dst->vegetation = .5f + math::clamp(
                    3.0f * (dst->vegetation - .5f),
                    -.5f, .5f);
                
                // Form texture weights
                float city_weight;
                float mountain_weight;

                dst->height    /= (float)(0xff - Terrain::SEA_LEVEL);
                mountain_weight = 20.0f * (dst->height - 0.2f);
                city_weight     = 25.0f - dst->vegetation * 150.0f - dst->height * 500.0f;

                dst->height    *= Terrain::MAX_HEIGHT + 0.25f * (dst->height > 0);


                math::Vec4 weight(

                    // urban areas
                    std::max(0.0f, city_weight),

                    // dry land
                    1.0f - dst->vegetation,
                    
                    // forests
                    1.5f * pow(5.0f, dst->vegetation),
                    
                    // highlands
                    std::max(0.0f, mountain_weight)
                );
                
                weight.normalize();

                dst->texture[0] = weight.x;
                dst->texture[1] = weight.y;
                dst->texture[2] = weight.z;
                dst->texture[3] = weight.w;

                dst++;
            }
        }

        if (this->material.shader == NULL)
        {
            this->material.shader
                = gfx::Program::get("terrain", "terrain fog");

            static const char *tileset[] = {
                // Surface terrain textures
                "video/textures/city.jpg",
                "video/textures/field_3.jpg",
                "video/textures/forest.jpg",
                "video/textures/grey_stone4-512x512.jpg"
            };

            for (int i = 0; i < 4; ++i)
            {
                core::engine.log("Loading %s", tileset[i]);
                this->texture[i].name
                    = core::str::cat(DATA_DIRECTORY, tileset[i]);
                
                core::str::set(this->texture[i].name,
                    core::str::normalize_path(this->texture[i].name));

                this->texture[i].image
                    = new gfx::Sprite(this->texture[i].name);
                    
                if (this->texture[i].image->data == NULL)
                {
                    core::engine.log(
                        "(!) Failed to load %s", this->texture[i].name);
                    
                    this->texture[i].image->resize(1, 1);
                    this->texture[i].image->clear(0xff808080);
                }
                this->texture[i].texture
                    = gfx::Texture::load(this->texture[i].image, 0, i);
            }
            this->material.color_map    = this->texture[0].texture;
            this->material.normal_map   = this->texture[1].texture;
            this->material.bump_map     = this->texture[2].texture;
            this->material.specular_map = this->texture[3].texture;

            this->material.compose();
        }

        screen.scene->build_ground_plane();
        
        core::engine.log("%.1f square kilometers loaded", sqrt(this->w * this->h) * TerrainNode::SIZE / 1000.0f);
    }
    
    delete heightmap;
    delete vegetation;
    
    this->preload_props();
}

void
Terrain::prefetch(const math::Vec3 &pos, int radius)
{
    for (int grid_z = -radius; grid_z <= radius; ++grid_z)
    {
        float z = pos.z + grid_z * TerrainChunk::SIZE;
        for (int grid_x = -radius; grid_x <= radius; ++grid_x)
        {
            float x = pos.x + grid_x * TerrainChunk::SIZE;
            this->get_chunk(x, z);
        }
    }
}

void
Terrain::preload_props(void)
{
    using namespace gfx::scenery;
    
    int seed = rand();
    
    core::engine.log("Generating scenery prop models");
    for (int i = 0; i < 15; ++i)
    {
        PropType    type;
        char        style = 'a' + math::rnd(5);
        math::Vec3  size;
        int         wings;
        gfx::Model *roof;
        
        for (int t = 0; t < 3; ++t)
        {
            srand(seed);
            switch (t)
            {
                case 0:
                    type  = HOUSE;
                    wings = math::rnd(3);
                    size  = math::Vec3(
                        math::rnd(5, 15),
                        math::rnd(1, 3) + Terrain::BUILDING_STEM,
                        math::rnd(3, 10));
                    roof  = (math::probability(.4f))
                        ? building::roof::curved(style)
                        : building::roof::gable(style);

                    break;


                case 1:
                    type  = TOWNHOUSE;
                    wings = math::rnd(4);
                    size  = math::Vec3(
                        math::rnd(10, 40),
                        math::rnd(4,  10) + Terrain::BUILDING_STEM,
                        math::rnd(8,  20));
                    roof  = (math::probability(.6f))
                        ? building::roof::pyramid(style)
                        : building::roof::angled(style);

                    break;


                case 2:
                    type  = HIGHRISE;
                    wings = math::rnd(6);
                    size  = math::Vec3(
                        math::rnd(10, 30),
                        math::rnd(40, 60) + Terrain::BUILDING_STEM,
                        math::rnd(10, 30));
                    roof  = (math::probability(.4f))
                        ? building::roof::dome(style, 3.0f, 1)
                        : building::roof::pyramid(style, 5.0f);

                    break;
            }
            
            // low detail
            this->prop_models[type].push_back(
                building::house(size, wings / 2,
                    building::roof::flat(style), style, 0.0f));

            // medium detail
            srand(seed);
            this->prop_models[type].push_back(
                building::house(size, wings,
                    building::roof::pyramid(style), style, 0.0f));
            
            // best detail
            srand(seed);
            this->prop_models[type].push_back(
                building::house(size, wings, roof, style));
            
            seed++;
        }
    }
    
    srand(time(NULL));

    // this->prop_models[TREE].push_back(tree::stump(5.0f));
    // this->prop_models[TREE].push_back(tree::stump(5.0f));
    
    // for (int i = 0; i < 5; ++i)
    // {
    //     this->prop_models[TREE].push_back(tree::dead());
    //     this->prop_models[SHRUB].push_back(tree::shrub());
    // }

    unsigned int total = 0;
    for (int i = 0; i < PROP_TYPE_COUNT;
        total += this->prop_models[i++].size() / TerrainChunk::LOD_LEVELS);
    core::engine.log("%i prop models generated.", total);
}

bool
Terrain::get_prop(gfx::Model **dst, PropType type)
{
    if (this->prop_models[type].empty())
    {
        return false;
    }
    
    int src = rand() % this->prop_models[type].size();
    src = (int)(src / TerrainChunk::LOD_LEVELS) * TerrainChunk::LOD_LEVELS;
    
    for (int i = 0; i < TerrainChunk::LOD_LEVELS; ++i)
    {
        dst[i] = this->prop_models[type][src + i];
    }
    return true;
}

TerrainChunk &
Terrain::get_chunk(int x, int z)
{
    x /= TerrainChunk::SIZE;
    z /= TerrainChunk::SIZE;
    
    if (this->chunks.cache == NULL)
    {
        // Initialize new chunk cache
        this->chunks.cache    = new TerrainChunk*[1];
        this->chunks.north    = this->chunks.south = z;
        this->chunks.east     = this->chunks.west  = x;
        this->chunks.cache[0] = NULL;
    }
    else if (x < this->chunks.west || x > this->chunks.east
        || z < this->chunks.north || z > this->chunks.south)
    {
        int
            west  = std::min(x, this->chunks.west),
            east  = std::max(x, this->chunks.east),
            north = std::min(z, this->chunks.north),
            south = std::max(z, this->chunks.south);

        int
            src_w    = 1 + this->chunks.east - this->chunks.west,
            src_h    = 1 + this->chunks.south - this->chunks.north,
            dst_size = (1 + east - west) * (1 + south - north);

        // Resize
        TerrainChunk **resized = new TerrainChunk*[dst_size];
        
        // Copy references to existing chunks
        TerrainChunk **dst = resized;
        for (int z_dst = north; z_dst <= south; ++z_dst)
        {
            int z_src = z_dst - this->chunks.north;
            for (int x_dst = west; x_dst <= east; ++x_dst)
            {
                int x_src = x_dst - this->chunks.west;
                if (x_src >= 0 && x_src < src_w && z_src >= 0 && z_src < src_h)
                {
                    *dst = this->chunks.cache[x_src + z_src * src_w];
                }
                else
                {
                    *dst = NULL;
                }
                
                dst++;
            }
        }
        
        delete[] this->chunks.cache;
        this->chunks.west  = west;
        this->chunks.east  = east;
        this->chunks.north = north;
        this->chunks.south = south;
        this->chunks.cache = resized;
    }
    
    // Get correct chunk cache reference
    int i = (x - this->chunks.west)
        + (z - this->chunks.north)
        * (1 + this->chunks.east - this->chunks.west);

    TerrainChunk **chunk = &this->chunks.cache[i];

    // Create new chunk if not cached yet
    if (*chunk == NULL)
    {
        *chunk = new TerrainChunk(
            x * TerrainChunk::SIZE,
            z * TerrainChunk::SIZE
        );
        (*chunk)->mesh->material   = &this->material;
    }

    return **chunk;
}

bool
Terrain::is_cached(int x, int z)
{
    x /= TerrainChunk::SIZE;
    z /= TerrainChunk::SIZE;

    if (this->chunks.cache == NULL
        || x < this->chunks.west  || x > this->chunks.east
        || z < this->chunks.north || z > this->chunks.south)
    {
        return false;
    }

    int i = (x - this->chunks.west)
        + (z - this->chunks.north)
        * (1 + this->chunks.east - this->chunks.west);

    return (this->chunks.cache[i] != NULL);
}

void
Terrain::render(void)
{
    math::Vec3 camera(screen.scene->matrix.camera.inverse());
    
    float offset_x = fmod(camera.x, TerrainChunk::SIZE) / TerrainChunk::SIZE;
    float offset_z = fmod(camera.z, TerrainChunk::SIZE) / TerrainChunk::SIZE;
    
    math::Mat4 mv = screen.scene->matrix.camera
        * screen.scene->matrix.projection;
    
    int visible_chunks = core::engine.config
        ["video"]["detail"]["view_range"]
        .integer(6000) / TerrainChunk::SIZE;

    for (int grid_z = -visible_chunks; grid_z <= visible_chunks; ++grid_z)
    {
        float z   = camera.z + grid_z * TerrainChunk::SIZE;
        float r_z = grid_z - offset_z;
        for (int grid_x = -visible_chunks; grid_x <= visible_chunks; ++grid_x)
        {
            float x = camera.x + grid_x * TerrainChunk::SIZE;
            
            // Cull chunks behind camera
            if ((math::Vec3(x, 0.0f, z) * mv).z < -TerrainChunk::SIZE)
            {
                continue;
            }
            
            float r_x  = grid_x - offset_x;
            float dist = sqrt(r_x * r_x + r_z * r_z);
            
            if (dist < visible_chunks)
            {
                this->get_chunk(x, z).render(
                    dist * TerrainChunk::SIZE,
                    1.1f - dist / (float)visible_chunks);
            }
        }
    }
}
