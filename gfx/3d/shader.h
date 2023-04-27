#ifndef _GFX_3D_SHADER_H
#define _GFX_3D_SHADER_H

#include <GL/gl.h>

namespace gfx
{
    class Shader;
    class Shader
    {
        public:
            const GLint  &id;
            const GLenum &type;
            const bool   &ready;
            char * const &filename;

            static Shader *
            get(const char *filename);
            
            static Shader *
            get_vertex(const char *filename);
            
            static Shader *
            get_fragment(const char *filename);
            
            Shader();
            ~Shader();
            
            const char *
            get_log(void);
            
            static void
            flush_cache(void);

        protected:
            GLint  id_mutable;
            GLenum type_mutable;
            bool ready_mutable;
            char *filename_mutable, *log;

            static Shader *
            load(const char *filename);

            void
            unload(void);
    };
}

#endif
