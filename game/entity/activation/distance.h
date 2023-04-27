#ifndef _GAME_ENTITY_ACTIVATION_DISTANCE_H
#define _GAME_ENTITY_ACTIVATION_DISTANCE_H

#include "../component.h"

namespace game
{
    class DistanceActivation:
        virtual public ActivationComponent
    {
        public:
            DistanceActivation();

            virtual void
            update(void);
            
            virtual DistanceActivation *
            clone(void) { return new DistanceActivation(*this); }
    };
}

#endif
