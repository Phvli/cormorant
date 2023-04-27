#ifndef _GFX_3D_FORMATS_OBJ_H
#define _GFX_3D_FORMATS_OBJ_H

#include "../model.h"

namespace gfx
{
    namespace io
    {
        Model *
        load_OBJ(const char *filename);
        
        int
        load_MTL(const char *filename);
    }
}

#endif
