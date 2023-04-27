#ifndef _GFX_3D_SCENERY_ROCKS
#define _GFX_3D_SCENERY_ROCKS

#include "../model.h"
#include "../../../math/vec3.h"

namespace gfx
{
    namespace scenery
    {
        namespace rock
        {
            Model *
            formation(float smoothness = 0.8f, const math::Vec3 &size = math::Vec3(1.5f, 1.0f, 1.5f));

            Model *
            boulder(float smoothness = 0.97f, const math::Vec3 &size = math::Vec3(1.0f, 1.0f, 1.0f));

            Model *
            column(const math::Vec3 &size = math::Vec3(0.8f, 2.0f, 0.8f));
        }
    }
}

#endif
