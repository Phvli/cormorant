#include "clouds.h"

#include "../mesh.h"
#include "../material.h"
#include "../primitives.h"
#include "../program.h"

#include "../../../math/util.h"

#include <cmath>

// #define CLOUDS_PER_AXIS 6
#define CLOUDS_PER_AXIS 4
// clouds in the cloud texture

using namespace gfx;

static Mesh *
_finalize(Mesh *mesh)
{
    mesh->build_normals();
    mesh->compose();

    const char *path = "video/textures/effects/cloud.png";
    // const char *path = "video/textures/effects/explosion.png";
    
    mesh->material                 = Material::add(path);
    mesh->material->color_map      = Texture::get(path);
    mesh->material->ambient_color  = math::Vec3(0.30f, 0.31f, 0.32f);
    mesh->material->diffuse_color  = math::Vec3(0.40f, 0.41f, 0.42f);
    mesh->material->specular_color = math::Vec3(0.10f, 0.10f, 0.10f);
    mesh->material->shader         = Program::get("sprite", "sprite");
    mesh->material->compose();
    
    return mesh;
}

Model *
scenery::cloud::cumulus(const math::Vec3 &size)
{
    Mesh *mesh = new Mesh();
    
    float uv = 1.0f / CLOUDS_PER_AXIS;
    for (int n = math::rnd(50, 200); n > 0; --n)
    {
        mesh->add(&(*primitive::plane())
            .rotZ(math::rnd(math::PI))
            .rotX(math::rnd(math::PI))
            .rotY(math::rnd(math::PI))
            .scale_uv(uv, uv)
            .translate_uv(
                uv * (int)math::rnd(CLOUDS_PER_AXIS),
                // uv * (int)math::rnd(CLOUDS_PER_AXIS)
                0.0f
            )
            .scale(size * math::rnd(0.05f, 0.20f))
            .translate(
                math::vary(size.x / 2.0f),
                math::vary(size.y / 2.0f),
                math::vary(size.z / 2.0f)
            )
        );
    }

    Model *model = new Model();
    model->add(_finalize(mesh));

    model->compose();

    return model;
}
