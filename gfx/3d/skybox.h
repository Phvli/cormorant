#ifndef _GFX_3D_SKYBOX_H
#define _GFX_3D_SKYBOX_H

#include "mesh.h"

namespace gfx
{
    class Skybox
    {
        public:
            gfx::Mesh
                *mesh;

            Skybox();
            Skybox(const char *filename);
            ~Skybox();

            bool
            load(const char *filename);

            void
            render(void);
    };
}

#endif
