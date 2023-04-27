#ifndef _GFX_3D_SCENERY_BUILDINGS
#define _GFX_3D_SCENERY_BUILDINGS

#include "../mesh.h"
#include "../model.h"
#include "../../../math/vec3.h"

namespace gfx
{
    namespace scenery
    {
        namespace building
        {
            Model *
            simple(const math::Vec3 &size = math::Vec3(10.0f, 2, 6.0f), char style = 'a');
            // Builds a house-looking box with as little detail as possible.

            Model *
            house(const math::Vec3 &size = math::Vec3(10.0f, 2, 6.0f), int wings = 1, Model *roof = NULL, char style = 'a', float chimney_probability = .5f);
            // Builds a house with (int)size.y stories. A gable roof will be used if roof is NULL.

            namespace roof
            {
                Model *
                flat(char style = 'a');
                // Completely flat, shingled roof
                
                Model *
                gable(char style = 'a', float height = 1.5f, float brim = 0.0f);
                // Most common type, where two sloping sections join at the middle
                
                Model *
                angled(char style = 'a', float height = 1.0f, float brim = -0.1f);
                // Flat roof with an angle makes the back wall a bit higher
                // Optional brim reaches out and hangs over the front porch
                
                Model *
                pyramid(char style = 'a', float height = 1.0f);
                
                Model *
                curved(char style = 'a', float height = 2.0f, int subdiv = 12);
                
                Model *
                dome(char style = 'a', float height = 3.0f, int subdiv = 2);
            }
        }
    }
}

#endif
