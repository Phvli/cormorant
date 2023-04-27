#ifdef USE_OPENGL
#   define GLEW_STATIC
#   define GL3_PROTOTYPES 1
#   include <GL/glew.h>
#endif

#include "material.h"
#include "../../core/util/string.h"
#include "../../core/util/cache.h"
#include "../../core/engine.h"
#include "formats/obj.h"

using namespace gfx;

static core::Cache<Material> global_cache;

Material::Material() :
    name(name_mutable)
{
    this->name_mutable          = NULL;
    
    this->ambient_color         = math::Vec3(0.10f, 0.10f, 0.10f);
    this->diffuse_color         = math::Vec3(0.50f, 0.50f, 0.50f);
    this->specular_color        = math::Vec3(0.25f, 0.25f, 0.25f);
            
    this->specular_exponent     = 50.0f;
    this->refractive_index      = 1.0f;
    this->alpha                 = 1.0f;
            
    this->color_map             = NULL;
    this->bump_map              = NULL;
    this->normal_map            = NULL;
    this->specular_map          = NULL;
    this->decal_map             = NULL;
    this->ambient_occlusion_map = NULL;
            
    this->transparency          = false;
    this->reflections           = false;
    this->refractions           = false;

    this->shader                = NULL;
}

Material::Material(const Material &material) :
    name(name_mutable)
{
    this->ambient_color         = material.ambient_color;
    this->diffuse_color         = material.diffuse_color;
    this->specular_color        = material.specular_color;
            
    this->specular_exponent     = material.specular_exponent;
    this->refractive_index      = material.refractive_index;
    this->alpha                 = material.alpha;
            
    this->color_map             = material.color_map;
    this->bump_map              = material.bump_map;
    this->normal_map            = material.normal_map;
    this->specular_map          = material.specular_map;
    this->decal_map             = material.decal_map;
    this->ambient_occlusion_map = material.ambient_occlusion_map;
            
    this->transparency          = material.transparency;
    this->reflections           = material.reflections;
    this->refractions           = material.refractions;

    this->shader                = material.shader;
    this->name_mutable = core::str::dup(material.name);
}

Material::~Material()
{
    delete[] this->name_mutable;
}

void
Material::use(void)
{
    this->shader->use();
    
    glUniform3f(this->shader->unif.diffuse_color,
        this->diffuse_color.x,
        this->diffuse_color.y,
        this->diffuse_color.z);

    glUniform3f(this->shader->unif.specular_color,
        this->specular_color.x,
        this->specular_color.y,
        this->specular_color.z);

    glUniform3f(this->shader->unif.ambient_color,
        this->ambient_color.x,
        this->ambient_color.y,
        this->ambient_color.z);

    glUniform1f(this->shader->unif.specular_exponent,
        this->specular_exponent);

    glUniform1f(this->shader->unif.refractive_index,
        this->refractive_index);

    glUniform1f(this->shader->unif.alpha,
        this->alpha);


    // Textures
    if (this->color_map != NULL)
    {
        glActiveTexture(GL_TEXTURE0 + 0);
        glUniform1i(this->shader->unif.color_map, 0);
        this->color_map->bind();
    }

    if (this->bump_map != NULL)
    {
        glActiveTexture(GL_TEXTURE0 + 1);
        glUniform1i(this->shader->unif.bump_map, 1);
        this->bump_map->bind();
    }

    if (this->normal_map != NULL)
    {
        glActiveTexture(GL_TEXTURE0 + 2);
        glUniform1i(this->shader->unif.normal_map, 2);
        this->normal_map->bind();
    }

    if (this->specular_map != NULL)
    {
        glActiveTexture(GL_TEXTURE0 + 3);
        glUniform1i(this->shader->unif.specular_map, 3);
        this->specular_map->bind();
    }

    if (this->decal_map != NULL)
    {
        glActiveTexture(GL_TEXTURE0 + 4);
        glUniform1i(this->shader->unif.decal_map, 4);
        this->decal_map->bind();
    }

    if (this->ambient_occlusion_map != NULL)
    {
        glActiveTexture(GL_TEXTURE0 + 5);
        glUniform1i(this->shader->unif.ambient_occlusion_map, 5);
        this->ambient_occlusion_map->bind();
    }
}

int
Material::load(const char *filename)
{
    return io::load_MTL(filename);
}

void
Material::decompose(void)
{
    this->shader = NULL;
}

bool
Material::compose(Flags flags)
{
    if (this->shader == NULL)
    {
        char *fragment = core::str::empty();
        
        if (this->color_map != NULL
            || this->bump_map != NULL
            || this->normal_map != NULL
            || this->specular_map != NULL
            || this->decal_map != NULL
            || this->ambient_occlusion_map != NULL)
        {
            if (this->color_map != NULL)
            {
                this->transparency |= this->color_map->translucent;
            }

            core::str::set(fragment, core::str::cat(fragment, (this->transparency)
                ? " textured_alpha"
                : " textured"
            ));
        }
        else
        {
            core::str::set(fragment, core::str::cat(fragment, " diffuse"));
        }
        core::str::set(fragment, core::str::cat(fragment, " specular"));

        if (flags & Material::UNDERWATER)
        {
            // (!) TODO: maybe adjust base properties a bit for underwater versions?
            core::str::set(fragment, core::str::cat(fragment, " caustics"));
            core::str::set(fragment, core::str::cat(fragment, " fog"));
            core::str::set(fragment, core::str::cat(fragment, " underwater"));
        }
        else
        {
            core::str::set(fragment, core::str::cat(fragment, " fog"));
        }

        if (flags & Material::FISH)
        {
            this->shader = gfx::Program::get("fish", core::str::ltrim(fragment));
        }
        else if (flags & Material::KELP)
        {
            this->shader = gfx::Program::get("kelp", core::str::ltrim(fragment));
        }
        else
        {
            this->shader = gfx::Program::get("static", core::str::ltrim(fragment));
        }

        delete[] fragment;

        if (this->shader == NULL)
        {
            return false;
        }
    }

    if (this->shader->unif.color_map != -1)
    {
        glUniform1i(this->shader->unif.color_map, 0);
    }

    if (this->shader->unif.normal_map != -1)
    {
        glUniform1i(this->shader->unif.normal_map, 1);
    }

    if (this->shader->unif.bump_map != -1)
    {
        glUniform1i(this->shader->unif.bump_map, 2);
    }

    if (this->shader->unif.specular_map != -1)
    {
        glUniform1i(this->shader->unif.specular_map, 3);
    }

    if (this->shader->unif.decal_map != -1)
    {
        glUniform1i(this->shader->unif.decal_map, 4);
    }

    if (this->shader->unif.ambient_occlusion_map != -1)
    {
        glUniform1i(this->shader->unif.ambient_occlusion_map, 5);
    }

    return true;
}

Material *
Material::add(const char *name)
{
    if (global_cache.contains(name))
    {
        return global_cache[name];
    }

    Material *material = new Material();
    core::str::set(material->name_mutable, core::str::dup(name));

    return global_cache.store(name, material);
}

Material *
Material::get(const char *name, Flags flags)
{
    char *s = (flags)
        ? core::str::format("%s::%x", name, flags)
        : core::str::dup(name);

    Material *material = global_cache[s];
    if (flags && material == NULL)
    {
        // Search for base material if requested variant hasn't been cached
        material = global_cache[name];
        if (material != NULL)
        {
            // Create new variant
            material = new Material(*material);
            material->decompose();
            material->compose(flags);
            material = global_cache.store(s, material);
        }
    }
    
    delete[] s;
    return   material;
}

Material *
Material::get(const Material *material, Flags flags)
{
    return Material::get(material->name, flags);
}

void
Material::flush_cache(void)
{
    global_cache.flush();
}
