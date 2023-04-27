#ifndef _GAME_COMPONENTS_PHYSICS_H
#define _GAME_COMPONENTS_PHYSICS_H

#include "../../entity.h"
#include "../../../math/vec3.h"

namespace game
{
    class DynamicPhysics:
        virtual public PhysicsComponent
    {
    public:
        DynamicPhysics();
        virtual ~DynamicPhysics();

        virtual void
        update(void);

        virtual DynamicPhysics *
        clone(void) { return new DynamicPhysics(*this); }
    };
}

#endif
