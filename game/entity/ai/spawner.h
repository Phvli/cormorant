#ifndef _GAME_ENTITY_AI_SPAWNER_H
#define _GAME_ENTITY_AI_SPAWNER_H

#include "../component.h"

namespace game
{
    class SpawnerAI:
        virtual public AIComponent
    {
        public:
            Entity prototype;
            int interval;
            
            SpawnerAI();
            SpawnerAI(int interval);

            virtual void
            update(void);
        
            virtual SpawnerAI *
            clone(void) { return new SpawnerAI(*this); }

        protected:
            unsigned int next;
    };
}

#endif
