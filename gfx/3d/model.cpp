
#include "model.h"

#include <algorithm> // min, max, sort
#include <map>

#include "formats/obj.h"
#include "../../core/util/cache.h"
#include "../../core/util/string.h"
#include "../../core/util/file.h"   // normalize_path

using namespace gfx;

static core::Cache<Model> global_cache;

Model::Model() :
    filename(filename_mutable)
{
    this->filename_mutable = NULL;
}

Model::Model(Mesh *primitive) :
    filename(filename_mutable)
{
    this->filename_mutable = NULL;
    
    if (primitive != NULL)
    {
        this->meshes.push_back(primitive);
    }
}

Model::~Model()
{
    global_cache.drop(this, false);

    for (Model::Meshes::iterator mesh = this->meshes.begin();
        mesh != this->meshes.end(); ++mesh)
    {
        delete *mesh;
    }
    
    delete[] this->filename_mutable;
}

Mesh *
Model::get(const char *name)
{
    for (Model::Meshes::iterator mesh = this->meshes.begin();
        mesh != this->meshes.end(); ++mesh)
    {
        if ((*mesh)->name != NULL
            && core::str::cmp(name, (*mesh)->name) == 0)
        {
            return *mesh;
        }
    }
    
    return NULL;
}

void
Model::render(void)
const
{
    for (Model::Meshes::const_iterator mesh = this->opaque_meshes.begin();
        mesh != this->opaque_meshes.end(); ++mesh)
    {
        (*mesh)->render();
    }

    // glDisable(GL_DEPTH_TEST);
    // glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    for (Model::Meshes::const_iterator mesh = this->transparent_meshes.begin();
        mesh != this->transparent_meshes.end(); ++mesh)
    {
        (*mesh)->render();
    }
    glDisable(GL_BLEND);
    // glDepthMask(GL_TRUE);
    // glEnable(GL_DEPTH_TEST);
}

static bool
sort_by_material(const Mesh *a, const Mesh *b)
{
    return a->material < b->material;
}

void
Model::compose(void)
{
    std::sort(this->meshes.begin(), this->meshes.end(), sort_by_material);

    this->transparent_meshes.clear();
    this->opaque_meshes.clear();
    for (Model::Meshes::const_iterator mesh = this->meshes.begin();
        mesh != this->meshes.end(); ++mesh)
    {
        if ((*mesh)->material->transparency)
        {
            this->transparent_meshes.push_back(*mesh);
        }
        else
        {
            this->opaque_meshes.push_back(*mesh);
        }
    }
}

bool
Model::intersects(const math::Vec3 &relative_origin, const math::Vec3 &direction)
const
{
    for (Model::Meshes::const_iterator mesh = this->meshes.begin();
        mesh != this->meshes.end(); ++mesh)
    {
        if ((*mesh)->intersects(relative_origin, direction))
        {
            return true;
        }
    }
    
    return false;
}

float
Model::radius(void)
const
{
    float result = 0.0f;
    
    for (Model::Meshes::const_iterator mesh = this->meshes.begin();
        mesh != this->meshes.end(); ++mesh)
    {
        result = std::max(result, (*mesh)->radius());
    }

    return result;
}

math::Vec3
Model::size(void)
const
{
    math::Vec3 result(0.0f, 0.0f, 0.0f);

    for (Model::Meshes::const_iterator mesh = this->meshes.begin();
        mesh != this->meshes.end(); ++mesh)
    {
        math::Vec3 mesh_size = (*mesh)->size();
        
        result = math::Vec3(
            std::max(result.x, mesh_size.x),
            std::max(result.y, mesh_size.y),
            std::max(result.z, mesh_size.z)
        );
    }

    return result;
}

math::Vec3
Model::height(void)
const
{
    float
        min = +99999.0f,
        max = -99999.0f;

    for (Model::Meshes::const_iterator mesh = this->meshes.begin();
        mesh != this->meshes.end(); ++mesh)
    {
        for (Mesh::Vertices::const_iterator v = (*mesh)->vertices.begin();
            v != (*mesh)->vertices.end(); ++v)
        {
            min = std::min(min, v->pos.y);
            max = std::max(max, v->pos.y);
        }
    }

    return max - min;
}

void
Model::get_bounds(math::Vec3 *negative, math::Vec3 *positive)
const
{
    *negative = math::Vec3(-99999.0f, -99999.0f, -99999.0f);
    *positive = math::Vec3( 99999.0f,  99999.0f,  99999.0f);

    for (Model::Meshes::const_iterator mesh = this->meshes.begin();
        mesh != this->meshes.end(); ++mesh)
    {
        math::Vec3 mesh_neg, mesh_pos;
        (*mesh)->get_bounds(&mesh_neg, &mesh_pos);
        
        *negative = math::Vec3(
            std::min(negative->x, mesh_neg.x),
            std::min(negative->y, mesh_neg.y),
            std::min(negative->z, mesh_neg.z)
        );
        *positive = math::Vec3(
            std::max(positive->x, mesh_pos.x),
            std::max(positive->y, mesh_pos.y),
            std::max(positive->z, mesh_pos.z)
        );
    }
}

Model *
Model::load(const char *filename)
{
    char *s = core::str::normalize_path(filename);
    Model *model;

    if (global_cache.contains(s))
    {
        model = global_cache[s];
    }
    else
    {
        model = gfx::io::load_OBJ(s);
        
        if (model != NULL)
        {
            core::str::set(model->filename_mutable, core::str::dup(s));
            model->compose();
        }

        global_cache.store(
            s,
            model);
    }
    
    delete[] s;
    return model;
}

void
Model::unload(void)
{
    if (!global_cache.contains(this))
    {
        delete this;
    }
}

void
Model::add(Mesh *mesh)
{
    this->meshes.push_back(mesh);
}

void
Model::add(const Model *model)
{
    for (Model::Meshes::const_iterator mesh = model->meshes.begin();
        mesh != model->meshes.end(); ++mesh)
    {
        Mesh *copy = new Mesh();
        copy->material = (*mesh)->material;
        
        for (Mesh::Vertices::const_iterator v = (*mesh)->vertices.begin();
            v != (*mesh)->vertices.end(); ++v)
        {
            copy->add(*v);
        }

        for (Mesh::Indices::const_iterator i = (*mesh)->indices.begin();
            i != (*mesh)->indices.end(); ++i)
        {
            copy->indices.push_back(*i);
        }
        
        copy->compose();
        this->add(copy);
    }
}

void
Model::add(const Model *model, const math::Mat4 &transformation)
{
    for (Model::Meshes::const_iterator mesh = model->meshes.begin();
        mesh != model->meshes.end(); ++mesh)
    {
        Mesh *copy = new Mesh();
        copy->material = (*mesh)->material;
        
        for (Mesh::Vertices::const_iterator v = (*mesh)->vertices.begin();
            v != (*mesh)->vertices.end(); ++v)
        {
            Mesh::Vertex translated = *v;
            translated.pos *= transformation;
            
            copy->add(translated);
        }

        for (Mesh::Indices::const_iterator i = (*mesh)->indices.begin();
            i != (*mesh)->indices.end(); ++i)
        {
            copy->indices.push_back(*i);
        }
        
        copy->compose();
        this->add(copy);
    }
}

void
Model::add(const Model *model, float x, float y, float z, float rot_y)
{
    this->add(model,
        math::Mat4::identity()
        .rotY(rot_y)
        .translate(x, y, z));
}

void
Model::add(const Model *model, const math::Vec3 &pos, float rot_y)
{
    this->add(model,
        math::Mat4::identity()
        .rotY(rot_y)
        .translate(pos));
}

void
Model::add(const Model *model, const math::Vec3 &pos, const math::Vec3 &rot)
{
    this->add(model,
        math::Mat4::identity()
        .rotX(rot.x)
        .rotY(rot.y)
        .rotZ(rot.z)
        .translate(pos));
}

void
Model::flush_cache(void)
{
    global_cache.flush();
}
