#ifndef _GFX_3D_MATERIAL_H
#define _GFX_3D_MATERIAL_H

#include <GL/gl.h>

#include "program.h"
#include "texture.h"
#include "../../math/vec3.h"

namespace gfx
{
    class Material;
    class Material
    {
        public:
            typedef
                unsigned int
                Flags;
            
            static const Flags
                UNDERWATER = 0x0001,
                FISH       = 0x0002,
                KELP       = 0x0004,
                DEFAULT    = 0x0000;
            
            char * const &name;
            
            math::Vec3
                ambient_color,
                diffuse_color,
                specular_color;
            
            GLfloat
                specular_exponent,
                refractive_index,
                alpha;
            
            gfx::Texture
                *color_map,
                *bump_map,
                *normal_map,
                *specular_map,
                *decal_map,
                *ambient_occlusion_map;
            
            bool
                transparency,
                reflections,
                refractions;

            gfx::Program
                *shader;

            static Material *
            get(const char *name, Flags flags = 0);

            static Material *
            get(const Material *material, Flags flags = 0);

            static Material *
            add(const char *name);

            static int
            load(const char *filename);
            // Loads an MTL file, returns the number of new materials cached

            static void
            flush_cache(void);

            Material();
            Material(const Material &material);
            ~Material();
            
            void
            use(void);
            
            bool
            compose(Flags flags = 0);
            
            void
            decompose(void);
            
        protected:
            char *name_mutable;
    };
}

#endif
