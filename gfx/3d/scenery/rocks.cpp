#include "rocks.h"

#include <cmath>

#include "../mesh.h"
#include "../material.h"
#include "../primitives.h"
#include "../program.h"
#include "../../../math/util.h"

using namespace gfx;

static Model *
_finalize(Mesh *mesh)
{
    mesh->clip();
    mesh->build_normals();
    mesh->compress();
    mesh->compose();

    mesh->material                 = Material::add("scenery/rock");
    mesh->material->color_map      = Texture::get("video/textures/scenery/rock.jpg");
    mesh->material->ambient_color  = math::Vec3(0.12f, 0.12f, 0.12f);
    mesh->material->diffuse_color  = math::Vec3(0.40f, 0.40f, 0.40f);
    mesh->material->specular_color = math::Vec3(0.07f, 0.07f, 0.07f);
    mesh->material->compose();
    
    Model *model = new Model();
    model->add(mesh);
    model->compose();
    
    return model;
}

static void
_rotate(Mesh *mesh)
{
    mesh->rotX(math::vary(math::PI));
    mesh->rotY(math::vary(math::PI));
    mesh->rotZ(math::vary(math::PI));
}

Model *
scenery::rock::formation(float smoothness, const math::Vec3 &size)
{
    smoothness = math::clamp(smoothness, 0.0f, 1.0f);

    Mesh *mesh = new Mesh();
    math::Vec3 shift(0.0f);
    
    float a = math::rnd(math::DOUBLE_PI);
    
    for (float s = 1.0f; s > 0.3f; s -= math::vary(0.2f, 0.3f))
    {
        Mesh *stone = gfx::primitive::icosphere(
            math::min(2, 3.5f * smoothness * s));

        _rotate(stone);
        stone->scale(math::Vec3(s));
        stone->translate(shift);
        mesh->add(stone);
        
        a += math::vary(math::PI * 0.7f);

        shift.x += cos(a) * s;
        shift.z += sin(a) * s;
        shift.y += math::vary(.2f);
        
        if (math::probability(.3f / s))
        {
            break;
        }
    }

    mesh->roughen((1.0f - smoothness) * .5f);
    mesh->scale(size, false);

    return _finalize(mesh);
}

Model *
scenery::rock::boulder(float smoothness, const math::Vec3 &size)
{
    Mesh *mesh = gfx::primitive::icosphere(3);
    _rotate(mesh);

    (*mesh)
    .translate(0.0f, size.y * .9f, 0.0f)
    .roughen(math::clamp((1.0f - smoothness) * .5f, 0.0f, 0.5f))
    .scale(size, false);

    return _finalize(mesh);
}

Model *
scenery::rock::column(const math::Vec3 &size)
{
    Mesh *mesh = new Mesh();
    math::Vec3 shift(0.0f);
    for (float s = 1.0f; shift.y < size.y * 2; shift += math::Vec3(
        math::vary(.2f) * size.x,
        s * 1.5f,
        math::vary(.2f) * size.z)
    )
    {
        Mesh *stone = gfx::primitive::icosphere(1);
        _rotate(stone);
        stone->translate(0.0f, 0.7f, 0.0f);
        stone->clip();
        stone->translate(0.0f, -0.7f, 0.0f);
        stone->scale(s * size.x, s * 1.5f, s * size.z);
        stone->roughen(.1f);

        stone->translate(shift);
        mesh->add(stone);

        s = math::max(0.3f, s * math::vary(.85f, .1f));
    }
    
    // Hat
    if (math::probability(.5f))
    {
        Mesh *stone = gfx::primitive::icosphere(1);
        _rotate(stone);
        stone->scale(1.5f * size.x, 0.2f, 1.5f * size.z);
        stone->rotX(math::vary(.5f));
        stone->roughen(.1f);
        
        // Dropped on the ground
        if (math::probability(.4f))
        {
            shift = math::Vec3(size.x * 1.5f, size.x, 0.0f);
            stone->rotZ(0.7f);
        }
        
        stone->translate(shift);
        mesh->add(stone);
    }

    return _finalize(mesh);
}
