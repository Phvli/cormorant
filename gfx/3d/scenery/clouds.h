#ifndef _GFX_3D_SCENERY_CLOUDS
#define _GFX_3D_SCENERY_CLOUDS

#include "../model.h"
#include "../../../math/vec3.h"

namespace gfx
{
    namespace scenery
    {
        namespace cloud
        {
            Model *
            cumulus(const math::Vec3 &size = math::Vec3(250.0f, 150.0f, 250.0f));

            Model *
            stratus(const math::Vec3 &size = math::Vec3(250.0f, 50.0f, 250.0f));

            Model *
            cirrus(const math::Vec3 &size = math::Vec3(500.0f, 25.0f, 500.0f));
        }
    }
}

#endif
