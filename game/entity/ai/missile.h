#ifndef _GAME_ENTITY_AI_MISSILE_H
#define _GAME_ENTITY_AI_MISSILE_H

#include "../component.h"

namespace game
{
    class MissileAI:
        virtual public AIComponent
    {
        public:
            MissileAI();

            virtual void
            update(void);
            
            virtual MissileAI *
            clone(void) { return new MissileAI(*this); }
    };
}

#endif
