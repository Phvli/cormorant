#include "buildings.h"

#include "../mesh.h"
#include "../material.h"
#include "../primitives.h"
#include "../program.h"

#include "../../../core/util/string.h"
#include "../../../math/util.h"
#include "../../../math/mat4.h"

#include <cmath>

#define FLOOR_HEIGHT  3.2f
#define TEXTURE_WIDTH 6.0f
#define MIN_WALL_SIZE 4.0f

using namespace gfx;

static Mesh *
_finalize(Mesh *mesh, const char *texture, char style)
{
    mesh->clip();
    mesh->build_normals();
    mesh->compress();
    mesh->compose();

    char *path = core::str::format(
        "video/textures/scenery/%s",
        texture);
    
    core::str::replace(path, '0', style);
    
    mesh->material                 = Material::add(path);
    mesh->material->color_map      = Texture::get(path);
    mesh->material->ambient_color  = math::Vec3(0.12f, 0.12f, 0.12f);
    mesh->material->diffuse_color  = math::Vec3(0.40f, 0.40f, 0.40f);
    mesh->material->specular_color = math::Vec3(0.07f, 0.07f, 0.07f);
    mesh->material->compose();
    
    delete path;
    
    return mesh;
}

static Model *
_chimney(const math::Vec3 &size)
{
    float y = FLOOR_HEIGHT * size.y / 4.0f;
    Mesh *mesh;
    
    // Antenna instead?
    if (math::probability(.4))
    {
        mesh = &(*primitive::cone(3))
            .scale(0.2f, 4.0f, 0.2f)
            .translate(0.0f, y * 2.0, 0.0f);
    }
    else
    {
        mesh = primitive::cylinder(5);
        
        // Make it double
        if (math::probability((size.x - 6.0f) / 20.0f))
        {
            mesh->translate(3.5f, 0.0f, 0.0f);
            mesh->add(primitive::cylinder(5));
        }
        
        mesh->scale(0.3f, y, 0.3f)
        .scale_uv(2.0, y)
        .translate(
            math::vary(size.x * .3f),
            y + 1.5f,
            -size.z * .3f);
    }

    
    Model *model = new Model();
    model->add(_finalize(mesh, "bricks.jpg", 0));
    
    return model;
}

Model *
scenery::building::simple(const math::Vec3 &size, char style)
{
    return scenery::building::house(size, 0,
        scenery::building::roof::flat(style), style, 0.0f);
}

Model *
scenery::building::house(const math::Vec3 &size, int wings, Model *roof, char style, float chimney_probability)
{
    float
        x   = size.x / 2.0f,
        y   = FLOOR_HEIGHT * (int)(size.y + .5f) / 2.0f,
        z   = size.z / 2.0f,
        uv_x = size.x / TEXTURE_WIDTH,
        uv_y = (int)(size.y + .5f) / 2.0f,
        uv_z = size.z / TEXTURE_WIDTH;
    
    // Snap wall texture scale to half texture widths
    uv_x = ((int)(uv_x * 2.0f)) / 2.0f;
    uv_z = ((int)(uv_z * 2.0f)) / 2.0f;

    Mesh *mesh = new Mesh();

    mesh->vertices.reserve(16);
    mesh->indices.reserve(24);
    math::Vec3 n;
    
    // left
    n = math::Vec3(-1.0f,  0.0f,  0.0f);
    (*mesh)
    .add(math::Vec3(-x, y,    -z), n, math::Vec2(0.0f, uv_y)) // 0
    .add(math::Vec3(-x, 0.0f, -z), n, math::Vec2(0.0f, 0.0f)) // 1
    .add(math::Vec3(-x, y,    +z), n, math::Vec2(uv_z, uv_y)) // 2
    .add(math::Vec3(-x, 0.0f, +z), n, math::Vec2(uv_z, 0.0f)) // 3
    .add(0, 1, 2)
    .add(2, 1, 3);

    // right
    n = math::Vec3(+1.0f,  0.0f,  0.0f);
    (*mesh)
    .add(math::Vec3(+x, y,    -z), n, math::Vec2(uv_z, uv_y)) // 4
    .add(math::Vec3(+x, 0.0f, -z), n, math::Vec2(uv_z, 0.0f)) // 5
    .add(math::Vec3(+x, y,    +z), n, math::Vec2(0.0f, uv_y)) // 6
    .add(math::Vec3(+x, 0.0f, +z), n, math::Vec2(0.0f, 0.0f)) // 7
    .add(4, 6, 5)
    .add(6, 7, 5);
    
    // front
    n = math::Vec3( 0.0f,  0.0f, +1.0f);
    (*mesh)
    .add(math::Vec3(-x, y,    +z), n, math::Vec2(0.0f, uv_y)) // 8
    .add(math::Vec3(+x, y,    +z), n, math::Vec2(uv_x, uv_y)) // 9
    .add(math::Vec3(-x, 0.0f, +z), n, math::Vec2(0.0f, 0.0f)) // 10
    .add(math::Vec3(+x, 0.0f, +z), n, math::Vec2(uv_x, 0.0f)) // 11
    .add(8,  10, 9)
    .add(11, 9,  10);

    // back
    n = math::Vec3( 0.0f,  0.0f, -1.0f);
    (*mesh)
    .add(math::Vec3(-x, y,    -z), n, math::Vec2(uv_x, uv_y)) // 12
    .add(math::Vec3(+x, y,    -z), n, math::Vec2(0.0f, uv_y)) // 13
    .add(math::Vec3(+x, 0.0f, -z), n, math::Vec2(0.0f, 0.0f)) // 14
    .add(math::Vec3(-x, 0.0f, -z), n, math::Vec2(uv_x, 0.0f)) // 15
    .add(12, 13, 14)
    .add(12, 14, 15);
    
    if (roof == NULL)
    {
        roof = scenery::building::roof::gable(style);
    }
    
    Model *model = new Model();
    model->add(_finalize(mesh, "wall_0.jpg", style));

    // Scale roof texture and translate it in place
    for (Model::Meshes::iterator r = roof->meshes.begin();
        r != roof->meshes.end(); ++r)
    {
        mesh = new Mesh();
        mesh->material = (*r)->material;
        mesh->vertices = (*r)->vertices;
        mesh->indices  = (*r)->indices;
        
        for (Mesh::Vertices::iterator v = mesh->vertices.begin();
            v != mesh->vertices.end(); ++v)
        {
            v->pos.x *= size.x / 2.0f;
            v->pos.y += y;
            v->pos.z *= size.z / 2.0f;

            if (r == roof->meshes.begin())
            {
                v->uv.x = v->pos.z / 2.0f;
                v->uv.y = v->pos.x / 2.0f;
            }
            else
            {
                v->uv.x = v->pos.z / 2.0f;
                v->uv.y = v->pos.y / 2.0f;
            }
        }
        
        mesh->compose();
        model->add(mesh);
    }
    
    // Chimney
    if (math::probability(chimney_probability))
    {
        model->add(_chimney(size));
    }
    
    // Wings
    if (wings > 1)
    {
        math::Vec3 wing_size(
            math::max(MIN_WALL_SIZE, size.x * .5f),
            math::rnd(1.0f, size.y),
            math::max(MIN_WALL_SIZE, size.z * .8f));

        Model *wing = scenery::building::house(wing_size, wings - 1,
            scenery::building::roof::flat(style), style, chimney_probability);
        
        model->add(wing, math::Mat4::identity()
            .translate((size.x - wing_size.x) / 2.0f - .1f, 0.0f, size.z * .5f)
            .rotY(math::QUARTER_PI * (int)math::vary(2.0f))
        );

        delete wing;
    }
    
    delete roof;

    model->compose();

    return model;
}

Model *
scenery::building::roof::flat(char style)
{
    Model *model = new Model();
    model->add(_finalize(primitive::plane(), "roof_0.jpg", style));

    model->compose();
    return model;
}

Model *
scenery::building::roof::gable(char style, float height, float brim)
{
    Model *model = new Model();
    math::Vec3 n;
    Mesh *mesh;
    
    brim = math::max(0.0f, 1.0f + brim);

    // Upper wall triangles
    mesh = new Mesh();
    mesh->vertices.reserve(6);
    mesh->indices.reserve(6);
    
    // left
    n = math::Vec3(0.0f, 0.0f, -1.0f);
    (*mesh)
    .add(math::Vec3(-1.0f, 0.0f,   -1.0f), n, math::Vec2(0.0f, 0.0f)) // 0
    .add(math::Vec3(-brim, height,  0.0f), n, math::Vec2(0.5f, 1.0f)) // 1
    .add(math::Vec3(-1.0f, 0.0f,   +1.0f), n, math::Vec2(1.0f, 0.0f)) // 2
    .add(0, 2, 1);

    // right
    (*mesh)
    .add(math::Vec3(+1.0f, 0.0f,   -1.0f), n, math::Vec2(1.0f, 0.0f)) // 3
    .add(math::Vec3(+brim, height,  0.0f), n, math::Vec2(0.5f, 1.0f)) // 4
    .add(math::Vec3(+1.0f, 0.0f,   +1.0f), n, math::Vec2(0.0f, 0.0f)) // 5
    .add(3, 4, 5);

    mesh->build_normals();
    model->add(_finalize(mesh, "facade_0.jpg", style));

    // Shingles
    mesh = new Mesh();
    mesh->vertices.reserve(8);
    mesh->indices.reserve(12);
    n = math::Vec3(0.0f);
    
    // back
    (*mesh)
    .add(math::Vec3(-1.0f, 0.0f,   -1.0f), n, math::Vec2(0.0f, 0.0f)) // 0
    .add(math::Vec3(-brim, height,  0.0f), n, math::Vec2(0.0f, 0.5f)) // 1
    .add(math::Vec3(+1.0f, 0.0f,   -1.0f), n, math::Vec2(1.0f, 0.0f)) // 2
    .add(math::Vec3(+brim, height,  0.0f), n, math::Vec2(1.0f, 0.5f)) // 3
    .add(0, 1, 3)
    .add(0, 3, 2);

    // front
    (*mesh)
    .add(math::Vec3(-1.0f, 0.0f,   +1.0f), n, math::Vec2(0.0f, 0.0f)) // 4
    .add(math::Vec3(-brim, height,  0.0f), n, math::Vec2(0.0f, 0.5f)) // 5
    .add(math::Vec3(+1.0f, 0.0f,   +1.0f), n, math::Vec2(1.0f, 0.0f)) // 6
    .add(math::Vec3(+brim, height,  0.0f), n, math::Vec2(1.0f, 0.5f)) // 7
    .add(4, 6, 7)
    .add(4, 7, 5);

    mesh->build_normals();
    model->add(_finalize(mesh, "roof_0.jpg", style));

    model->compose();
    return model;
}


Model *
scenery::building::roof::angled(char style, float height, float brim)
{
    Model *model = new Model();
    Mesh *mesh;
    math::Vec3 n;
    
    brim += 1.0f;

    // Upper wall triangles
    mesh = new Mesh();
    mesh->vertices.reserve(10);
    mesh->indices.reserve(12);
    
    // left
    n = math::Vec3(0.0f, 0.0f, -1.0f);
    (*mesh)
    .add(math::Vec3(-1.0f, 0.0f,   -1.0f), n, math::Vec2(0.0f, 0.0f)) // 0
    .add(math::Vec3(-1.0f, height,  brim), n, math::Vec2(0.0f, 1.0f)) // 1
    .add(math::Vec3(-1.0f, 0.0f,   +1.0f), n, math::Vec2(1.0f, 0.0f)) // 2
    .add(0, 2, 1);

    // back
    n = math::Vec3(0.0f, +1.0f, 0.0f);
    (*mesh)
    .add(math::Vec3(-1.0f, 0.0f,   +1.0f), n, math::Vec2(0.0f, 0.0f)) // 3
    .add(math::Vec3(+1.0f, 0.0f,   +1.0f), n, math::Vec2(1.0f, 0.0f)) // 4
    .add(math::Vec3(+1.0f, height,  brim), n, math::Vec2(1.0f, 1.0f)) // 5
    .add(math::Vec3(-1.0f, height,  brim), n, math::Vec2(0.0f, 1.0f)) // 6
    .add(3, 4, 5)
    .add(3, 5, 6);

    // right
    n = math::Vec3(0.0f, 0.0f, +1.0f);
    (*mesh)
    .add(math::Vec3(+1.0f, 0.0f,   -1.0f), n, math::Vec2(0.0f, 0.0f)) // 7
    .add(math::Vec3(+1.0f, height,  brim), n, math::Vec2(1.0f, 1.0f)) // 8
    .add(math::Vec3(+1.0f, 0.0f,   +1.0f), n, math::Vec2(1.0f, 0.0f)) // 9
    .add(8, 9, 7);

    mesh->build_normals();
    model->add(_finalize(mesh, "facade_0.jpg", style));

    // Shingles
    mesh = new Mesh();
    mesh->vertices.reserve(4);
    mesh->indices.reserve(6);
    n = math::Vec3(0.0f);
    
    (*mesh)
    .add(math::Vec3(-1.0f, 0.0f,   -1.0f), n, math::Vec2(0.0f, 0.0f)) // 0
    .add(math::Vec3(-1.0f, height,  brim), n, math::Vec2(0.0f, 0.5f)) // 1
    .add(math::Vec3(+1.0f, 0.0f,   -1.0f), n, math::Vec2(1.0f, 0.0f)) // 2
    .add(math::Vec3(+1.0f, height,  brim), n, math::Vec2(1.0f, 0.5f)) // 3
    .add(0, 1, 3)
    .add(0, 3, 2);

    mesh->build_normals();
    model->add(_finalize(mesh, "roof_0.jpg", style));

    model->compose();
    return model;
}

Model *
scenery::building::roof::pyramid(char style, float height)
{
    Model *model = new Model();
    Mesh  *mesh  = primitive::cone(4, true);

    float s = sqrt(2.0f);
    
    (*mesh)
    .rotY(math::QUARTER_PI)
    .translate(0.0f, 1.0f, 0.0f)
    .scale(s, height / 2.0f, s);
    
    model->add(_finalize(mesh, "roof_0.jpg", style));
    model->compose();

    return model;
}

Model *
scenery::building::roof::curved(char style, float height, int subdiv)
{
    subdiv *= 2;
    
    Model *model = new Model();
    
    model->add(_finalize(
        &(*primitive::circle(subdiv))
            .translate(0.0f, -2.0f, 0.0f)
            .flip()
            .add(primitive::circle(subdiv))
            .translate(0.0f, 1.0f, 0.0f)
            .rotZ(math::HALF_PI)
            .scale(1.0f, height, 1.0f),
        "facade_0.jpg", style));

    model->add(_finalize(
        &(*primitive::cylinder(subdiv, true))
            .rotZ(math::HALF_PI)
            .scale(1.0f, height, 1.0f),
        "roof_0.jpg", style));

    model->compose();
    return model;
}

Model *
scenery::building::roof::dome(char style, float height, int subdiv)
{
    Model *model = new Model();

    float s = sqrt(2.0f);
 
    model->add(_finalize(
        &(*primitive::cone(4, true))
            .rotY(math::QUARTER_PI)
            .translate(0.0f, 1.0f, 0.0f)
            .scale(s, height / 3.0f, s)
            .add(&(*primitive::icosphere(subdiv))
                .scale(0.9f, height, 0.9f)
            ),
        "roof_0.jpg", style));

    model->compose();
    return model;
}
