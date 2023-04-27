#ifndef _GFX_3D_SCENERY_TREES
#define _GFX_3D_SCENERY_TREES

#include "../model.h"

namespace gfx
{
    namespace scenery
    {
        namespace tree
        {
            Model *
            deciduous(float height = 22.0f);
 
            Model *
            desert(float height = 18.0f);
 
            Model *
            fir(float height = 20.0f);
 
            Model *
            pine(float height = 25.0f);
 
            Model *
            palm(float height = 15.0f);
 
            Model *
            shrub(float height = 4.0f);
 
            Model *
            cactus(float height = 6.0f);
 
            Model *
            dead(float height = 20.0f);
 
            Model *
            stump(float height = 0.3f);
 
            Model *
            log(float length = 10.0f);
        }
    }
}

#endif
