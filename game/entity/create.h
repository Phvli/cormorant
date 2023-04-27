#ifndef _GAME_ENTITY_CREATE_H
#define _GAME_ENTITY_CREATE_H

#include "../entity.h"
#include "../../math/vec3.h"


namespace game
{
    namespace create
    {
        Entity *
        explosion(const math::Vec3 &pos, float strength = 1.0f);

        Entity *
        fire(const math::Vec3 &pos, unsigned long int lifetime = 60000);

        Entity *
        smoke(const math::Vec3 &pos, unsigned long int lifetime = 10000);

        Entity *
        trail(const math::Vec3 &pos, unsigned long int lifetime = 5000);
    }
}

#endif
