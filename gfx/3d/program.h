#ifndef _GFX_3D_PROGRAM_H
#define _GFX_3D_PROGRAM_H

#include <GL/gl.h>
#include "shader.h"

namespace gfx
{
    class Program;
    class Program
    {
        public:
            Program();
            ~Program();

            static Program *
            get(const char *vertex, const char *fragment);
            // Builds a shader program with specified shaders linked together
            // Multiple shaders can be included, separated with a ' '
            
            static void
            flush_cache(void);

            const GLuint &id;
            const bool   &ready;

            Shader
                *vertex_shader,
                *fragment_shader;

            struct
            {
                GLint
                    i,        // vertex indices
                    v,        // vertex positions
                    n,        // vertex normals
                    t,        // texture coordinates
                    t_weight; // texture weights
            } attr;
            struct
            {
                GLint
                    ambient_color,
                    diffuse_color,
                    specular_color,
                    specular_exponent,
                    refractive_index,
                    alpha,

                    light_position,
                    ambient_fog,

                    projection,
                    modelview,
                    normal,

                    color_map,
                    normal_map,
                    bump_map,
                    specular_map,
                    decal_map,
                    ambient_occlusion_map,

                    camera_pos,
                    elapsed_time;
            } unif;
            
            void
            use(void);
            
            const char *
            get_log(void);

            void
            unload(void);
            
        protected:
            GLuint
                id_mutable;

            bool
                ready_mutable;
            
            char
                *log;

            void
            init(void);
    };
}

#endif
