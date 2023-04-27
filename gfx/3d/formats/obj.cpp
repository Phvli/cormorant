#include "obj.h"

#define MODEL_PATH "video/models/"

#include "../../../core/util/string.h"
#include "../../../core/util/file.h"
#include "../../../core/util/config.h"
#include "../../../math/vec2.h"
#include "../../../math/vec3.h"
#include "../../../core/engine.h"

using namespace gfx;
using namespace core::str;

class OBJ_File:
    public core::File
{
public:
    // Property name (v, usemtl, map_Kd, ...)
    char * const &property;
    
    // Parameters as a single string
    char * const &value;
    
    // Float vector containing up to 3 float parameters
    const math::Vec3 &vector;

    OBJ_File(const char *filename) :
        core::File(filename),
        property(property_mutable),
        value(value_mutable),
        vector(vector_mutable)
    {
        this->property_mutable = NULL;
    }
    
    ~OBJ_File()
    {
        delete[] this->property_mutable;
    }
    
    bool
    operator==(const char *s)
    {
        return (cmp(this->property, s) == 0);
    }
    
    bool
    get_line(void)
    // Get the next meaningful line or return false
    {
        for (;;)
        {
            if (this->eof())
            {
                core::str::empty(this->property_mutable);
                return false;
            }
            
            char *s = this->read();
            set(this->property_mutable, ltrim(s));
            this->value_mutable = find(this->property, ' ');

            // Skip comments and lines with no parameters
            if (*this->property != '#' && this->value != NULL)
            {
                break;
            }
        }

        // NULL-terminate property name
        *this->value_mutable = '\0';
        
        // Move value pointer to the beginning of the actual data
        this->value_mutable  = ltrim(this->value + 1);
        
        if (is_numeric(*this->value))
        {
            // Update the float vector for convenience
            sscanf(this->value, "%f %f %f",
                &this->vector_mutable.x,
                &this->vector_mutable.y,
                &this->vector_mutable.z
            );
        }

        return true;
    }

protected:
    char *property_mutable;
    char *value_mutable;
    math::Vec3 vector_mutable;
};

Model *
io::load_OBJ(const char *filename)
{
    typedef
        std::vector<math::Vec3>
        VecBuf;

    typedef
        std::vector<int>
        IndBuf;
    
    struct
    {
        VecBuf pos;
        IndBuf i;
    } vertex, normal, uv;
    
    Model *model        = new Model();
    Mesh  *mesh         = new Mesh();
    char  *path         = cat(MODEL_PATH, filename);
    char  *name         = empty();
    char  *mtllib       = empty();
    int    index_offset = 0;
    
    IndBuf *face[3] = { &vertex.i, &uv.i, &normal.i };
#   define VERTEX_BUFFER_SIZE 5000

    vertex.pos.reserve(VERTEX_BUFFER_SIZE);
    vertex.i.reserve(VERTEX_BUFFER_SIZE);

    uv.pos.reserve(VERTEX_BUFFER_SIZE);
    uv.i.reserve(VERTEX_BUFFER_SIZE);

    normal.pos.reserve(VERTEX_BUFFER_SIZE);
    normal.i.reserve(VERTEX_BUFFER_SIZE);

    core::engine.log("Loading model %s", path);
    OBJ_File obj(path);

    if (!obj.exists())
    {
        core::engine.log("(!) %s not found", path);
    }
    
    set(path, get_directory(path));
    
    for (;;)
    {
        bool last_line = !obj.get_line();
        bool form_mesh = false;
        
        // Begin next mesh or break at the end of file
        if (obj == "o" || last_line)
        {
            set(mesh->name, name);
            name = dup(obj.value);

            form_mesh = true;
        }
        
        // Load materials
        else if (obj == "mtllib")
        {
            // Load from relative path
            set(mtllib, cat(path, obj.value));
            set(mtllib, normalize_path(mtllib));
            // (!) FIXME: TODO: cache mtl files
            Material::load(mtllib);
        }

        // Switch material (+ begin next mesh)
        else if (obj == "usemtl")
        {
            char *s = cat(mtllib, obj.value);
            mesh->material = Material::get(s);
            delete[] s;
            
            form_mesh = true;
        }
        
        // Vertex indices
        else if (obj == "f")
        {
            // int i = 0;
            // char *data, *start = dup(obj.value);
            // for (char *s = data = start;; ++s)
            // {
            //     char c = *s;
            //     if (c == '/' || c == ' ' || c == '\0')
            //     {
            //         *s = '\0';
            //         int index = 0;
            //         sscanf(start, "%i", &index);
            //         face[i % 3]->push_back(index - 1);

            //         if (c == '\0')
            //         {
            //             break;
            //         }

            //         start = s + 1;
            //         i++;
            //     }
            // }
            // delete[] data;
            
            // TODO: Triangulate
            core::Config *token = split(replace(obj.value, ' ', '/'), '/', true);
            if (token->count() == 9)
            {
                for (int i = 0; i < 9; ++i)
                {
                    face[i % 3]->push_back((*token)[i].integer() - 1);
                }
            }
            delete token;
        }
        
        // Vertex attributes
        else if (obj == "v")
        {
            vertex.pos.push_back(obj.vector);
        }
        else if (obj == "vt")
        {
            uv.pos.push_back(obj.vector);
        }
        else if (obj == "vn")
        {
            normal.pos.push_back(obj.vector);
        }
        
        if (form_mesh)
        {
            // Add and index all vertices
            for (unsigned int index = index_offset; index < vertex.i.size();
                ++index)
            {
                int n;
                Mesh::Vertex v;
                v.pos = vertex.pos[vertex.i[index]];
                
                v.normal = ((n = normal.i[index]) >= 0)
                    ? normal.pos[n]
                    : math::Vec3::up(); // (!) TODO: compute normal if omitted

                v.uv = ((n = uv.i[index]) >= 0)
                    ? math::Vec2(uv.pos[n])
                    : math::Vec2(0.0f, 0.0f);

                // Find existing vertex to point to
                Mesh::Vertices::iterator i;
i = mesh->vertices.end();
                // for (i = mesh->vertices.begin();
                //     i != mesh->vertices.end(); ++i)
                // {
                //     if (v.pos    == i->pos
                //     &&  v.uv     == i->uv
                //     &&  v.normal == i->normal)
                //     {
                //         mesh->indices.push_back(i - mesh->vertices.begin());
                //         break;
                //     }
                // }

                // Add new vertex if none was found
                if (i == mesh->vertices.end())
                {
                    mesh->indices.push_back(mesh->vertices.size());
                    mesh->add(v);
                }
            }
            
            if (mesh->indices.size() > 0)
            {
                mesh->compose();
                model->add(mesh);
            }
            else
            {
                delete mesh;
            }
            
            if (last_line)
            {
                break;
            }

            Material *material = mesh->material;
            mesh               = new Mesh();
            mesh->material     = material;
            index_offset       = vertex.i.size();
        }
    }
    core::engine.log("%s ready", filename);
    
    delete[] path;
    delete[] name;
    delete[] mtllib;

    return model;
}

int
io::load_MTL(const char *filename)
{
    int materials_loaded = 0;
    
    // Dummy material to hold potential illegal values
    gfx::Material *material;
    
    core::engine.log("Loading mtllib %s", filename);

    OBJ_File mtl(filename);
    if (!mtl.exists())
    {
        core::engine.log("(!) %s not found", filename);
    }
    
    char *path = core::str::get_directory(filename);
    
    while (mtl.get_line())
    {

        if (mtl == "newmtl")
        { // New material
            if (materials_loaded)
            {
                material->compose();
            }
            char *s  = cat(filename, mtl.value);
            material = Material::add(s);
            materials_loaded++;
            delete[] s;
        }
        
        // Transparency and reflections
        else if (mtl == "d")
        { // Dissolution
            material->transparency = true;
            material->alpha        = mtl.vector.x;
        }
        else if (mtl == "Tr")
        { // Transparency
            material->transparency = true;
            material->alpha        = 1.0f - mtl.vector.x;
        }
        else if (mtl == "Ni")
        { // Refractive index
            material->refractions      = true;
            material->refractive_index = mtl.vector.x;
        }
        else if (mtl == "illum")
        { // Illumination model (used to turn reflections on and off)
            switch ((int)mtl.vector.x)
            {
                case 3:
                case 8:
                case 9:
                    material->reflections = true;
                    break;
                
                default:
                    material->reflections = false;
                    break;
            }
        }
        
        // Colors
        else if (mtl == "Ka")
        { // Ambient color
            material->ambient_color = mtl.vector;
        }
        else if (mtl == "Kd")
        { // Diffuse color
            material->diffuse_color = mtl.vector;
        }
        else if (mtl == "Ks")
        { // Specular color
            material->specular_color = mtl.vector;
        }
        else if (mtl == "Ns")
        { // Specular exponent
            material->specular_exponent = mtl.vector.x;
            continue;
        }
        
        // Textures
        else if (mtl == "map_Kd")
        { // Diffuse texture
            char *s = cat(path, mtl.value);
            material->color_map = Texture::get(s, Texture::AUTO, 0);
            delete[] s;
            continue;
        }
        else if (mtl == "bump" || mtl == "map_Bump")
        { // Bumpmap texture
            char *s = cat(path, mtl.value);
            material->bump_map = Texture::get(s, Texture::AUTO, 1);
            delete[] s;
            continue;
        }
        else if (mtl == "norm" || mtl == "map_Norm")
        { // Normal map
            char *s = cat(path, mtl.value);
            material->normal_map = Texture::get(s, Texture::AUTO, 2);
            delete[] s;
            continue;
        }
        else if (mtl == "map_Ks")
        { // Specular texture
            char *s = cat(path, mtl.value);
            material->specular_map = Texture::get(s, Texture::AUTO, 3);
            delete[] s;
            continue;
        }
        else if (mtl == "decal" || mtl == "map_Decal")
        { // Stencil texture
            char *s = cat(path, mtl.value);
            material->decal_map = Texture::get(s, Texture::AUTO, 4);
            delete[] s;
            continue;
        }
        else if (mtl == "ao" || mtl == "map_Ao")
        { // Ambient occlusion texture
            char *s = cat(path, mtl.value);
            material->ambient_occlusion_map = Texture::get(s, Texture::AUTO, 5);
            delete[] s;
            continue;
        }
    }
    
    delete[] path;

    if (materials_loaded)
    {
        material->compose();
    }
    core::engine.log(
        (materials_loaded == 1)
            ? "%i material found"
            : "%i materials found",
        materials_loaded);

    return materials_loaded;
}
