#ifndef _GAME_ENTITY_GRAPHICS_RIGID_H
#define _GAME_ENTITY_GRAPHICS_RIGID_H

#include "../component.h"

namespace game
{
    class RigidGraphics:
        virtual public GraphicsComponent
    {
    public:
        RigidGraphics();
        virtual ~RigidGraphics();

        virtual void
        render(void);

        virtual RigidGraphics *
        clone(void) { return new RigidGraphics(*this); }
    };
}

#endif
