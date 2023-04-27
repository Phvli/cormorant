#ifndef _GAME_ENTITY_ACTIVATION_TIMED_H
#define _GAME_ENTITY_ACTIVATION_TIMED_H

#include "../component.h"

namespace game
{
    class TimedActivation:
        virtual public ActivationComponent
    {
        public:
            TimedActivation() {}
            TimedActivation(int threshold);

            virtual void
            update(void);
            
            virtual TimedActivation *
            clone(void) { return new TimedActivation(*this); }
    };

    class TimedDeactivation:
        public TimedActivation
    {
        public:
            TimedDeactivation() {}
            TimedDeactivation(int threshold);

            virtual void
            update(void);

            virtual TimedDeactivation *
            clone(void) { return new TimedDeactivation(*this); }
    };

    class TimedDestruction:
        public TimedActivation
    {
        public:
            TimedDestruction() {}
            TimedDestruction(int threshold);

            virtual void
            update(void);

            virtual TimedDestruction *
            clone(void) { return new TimedDestruction(*this); }
    };
}

#endif
