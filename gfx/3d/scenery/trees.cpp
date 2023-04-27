#include "trees.h"

#include <cmath>
#include "../../../core/util/string.h"
#include "../../../math/mat4.h"
#include "../../../math/util.h"

using namespace gfx;

static Mesh *
_finalize(Mesh *mesh, const char *texture)
{
    mesh->clip();
    mesh->build_normals();
    mesh->compress();
    mesh->compose();

    char *path = core::str::format(
        "video/textures/scenery/%s",
        texture);
    
    mesh->material                 = Material::add(path);
    mesh->material->color_map      = Texture::get(path);
    mesh->material->ambient_color  = math::Vec3(0.12f, 0.12f, 0.12f);
    mesh->material->diffuse_color  = math::Vec3(0.40f, 0.40f, 0.40f);
    mesh->material->specular_color = math::Vec3(0.07f, 0.07f, 0.07f);
    mesh->material->compose();
    
    return mesh;
}

static Mesh *
_branch(
    float length,           // Final lenght of the branch
    float thickness,        // Final branch thickness
    float waviness,         // Amount of horizontal distortion
    int branches,           // Number of branches sprouting from this one
    int sub_branches,       // Number of grandchild branches from each child
    float branching_point,  // Where branching happens (ratio of length)
    float branching_angle   // Maximum angle difference in radians
)
{
    Mesh *mesh = new Mesh();
    
    if (length < .01f || thickness < .003f)
    {
        return mesh;
    }
    
    int corners = 3 + length / 10.0f;
    
    if (branches)
    {
        length *= branching_point;
    }

    int subdiv  = 2 + length / 4.0f;
    
    float
        a      = 0.0f,
        t      = 0.0f,
        a_step = math::DOUBLE_PI / (float)corners,
        t_step = 1.0f / (float)corners;
    
    mesh->indices.reserve(4 * corners);
    mesh->vertices.reserve(corners * subdiv);

    for (int x = 0; x <= corners; ++x)
    {
        int i  = subdiv * x;
        int i2 = i - subdiv;
        
        math::Vec2 v = math::Vec2(cos(a), sin(a * (x != corners)));
        for (int y = 0; y < subdiv; ++y, ++i, ++i2)
        {
            float
                f = (float)y / (float)(subdiv - 1),
                r = thickness * (1.0f - f / 2.0f);

            mesh->add(
                math::Vec3(v.x * r, f * length, v.y * r),
                math::Vec3(v.x, 0.0f, v.y),
                math::Vec2(t, f));

            if (x * y > 0)
            {
                mesh->add(i2 - 1, i,  i - 1);
                mesh->add(i2 - 1, i2, i);
            }
        }
        a += a_step;
        t += t_step;
    }
    
    float r = waviness;
    for (int y = 1; y < subdiv - 1; ++y)
    {
        math::Vec3 diff = math::Vec3(
            math::vary(r),
            -1.0f,
            math::vary(r));

        for (Mesh::Vertices::iterator v = mesh->vertices.begin() + y;
            v < mesh->vertices.end(); v += subdiv)
        {
            v->pos += diff;
        }
    }
    
    if (branches)
    {
        float s = length * .95f;

        length = (length / branching_point) * (1.0f - branching_point);

        thickness /= 2.0f;
        waviness  *= 1.3f;
        
        for (; branches >= 0; branches--)
        {
            mesh->add(
                &(*_branch(
                    math::vary(length, length * .1f),
                    thickness + math::vary(.05f),
                    waviness,
                    math::rnd(sub_branches + .5f),
                    0,
                    branching_point,
                    branching_angle * .8f))
                .rotX(math::vary(branching_angle))
                .rotY(math::vary(math::PI))
                .translate(0.0f, s, 0.0f)
            );
        }
    }
    
    return mesh;
}

static Mesh *
_leaves(int count, float height, float radius, float aspect_ratio)
{
    Mesh *mesh = new Mesh();

    height *= 0.9f / (radius * aspect_ratio);

    mesh->add(&(*primitive::icosphere(0))
    .rotX(math::rnd(math::DOUBLE_PI))
    .rotZ(math::rnd(math::DOUBLE_PI))
    .translate(0.0f, height, 0.0f));

    while (count-- > 1)
    {
        mesh->add(&(*primitive::icosphere(0))
        .translate(
            math::vary(0.5f),
            math::vary(height * .8f, height * .2f),
            math::vary(0.5f)));
    }

    return &(*mesh)
        .scale(radius, radius * aspect_ratio, radius)
        .scale_uv(radius / 2.0f, radius / 2.0f);
}

Model *
scenery::tree::dead(float height)
{
    Model *model = new Model();
    model->add(_finalize(_branch(height, height * .1f, 1.5f,
        math::rnd(2, 3), 0, math::rnd(.3f, .6f), 1.2f),
        "bark.jpg"));
    model->compose();

    return model;
}

Model *
scenery::tree::shrub(float height)
{
    Model *model = new Model();
    model->add(_finalize(_leaves(
        3, height * .4f, height, 0.8f),
        "spruce.png"));
    model->compose();
    return model;
}

Model *
scenery::tree::deciduous(float height)
{
    Mesh *mesh = _branch(height, height * .08f, 1.0f,
        math::rnd(4), 2, math::rnd(.3f, .6f), .8f);
    
    Model *model = new Model();
    model->add(_finalize(mesh, "bark.jpg"));
    model->add(_finalize(_leaves(
        math::rnd(3), height, height * .4f, 1.2f),
        "spruce.png"));
    model->compose();
    return model;
}

Model *
scenery::tree::desert(float height)
{
    Mesh *mesh = _branch(height, height * .08f, 1.1f,
        math::rnd(2, 4), 2, .3f, 0.5f);
    
    Model *model = new Model();
    model->add(_finalize(mesh, "bark.jpg"));
    model->add(_finalize(_leaves(
        math::rnd(4), height, height * .6f, 0.2f),
        "spruce.png"));
    model->compose();
    return model;
}

Model *
scenery::tree::cactus(float height)
{
    Mesh *mesh = primitive::tapered_cylinder(7, 0.8f, false);
    mesh->scale(0.6f, height, 0.6f, false);
    mesh->scale_uv(1.0f, height);
    
    int n = math::rnd(5);
    for (int i = 0; i < n; ++i)
    {
        float y = math::rnd(.9f) * height;
        float h = math::rnd(.6f) * (height - y);
        mesh->add(&(*primitive::tapered_cylinder(5, .8f))
            .scale(0.6f, 1.0f, 0.6f)
            .rotX(-1.2f)
            .add(&(*primitive::tapered_cylinder(5, .8f))
                .scale(0.5f, h, 0.5f)
                .translate(0.0f, h + .2f, 0.8f)
            )
            .translate(0.0f, y, 1.0f)
            .rotY(math::vary(math::DOUBLE_PI * (float)i / n, .3f))
        );
    }
    


    Model *model = new Model();
    model->add(_finalize(mesh, "cactus.jpg"));
    model->compose();
    return model;
}

Model *
scenery::tree::stump(float height)
{
    Mesh *mesh = primitive::tapered_cylinder(5, 0.5f, false);

    (*mesh)
    .subdivide()
    .scale(0.7f, height, 0.7f, false)
    .roughen(.07f);

    Model *model = new Model();
    model->add(_finalize(mesh, "bark.jpg"));
    model->compose();
    return model;
}

Model *
scenery::tree::log(float length)
{
    Mesh *mesh = primitive::cylinder(6, false);

    (*mesh)
    .transform(
        math::Mat4::scaling(0.8f, length / 2.0f, 0.8f)
        .rotX(math::HALF_PI)
        .translateY(.3f))
    .subdivide()
    .roughen(.12f);

    Model *model = new Model();
    model->add(_finalize(mesh, "bark.jpg"));
    model->compose();
    return model;
}
