#ifndef _GAME_GAMELOGIC_H
#define _GAME_GAMELOGIC_H

#include "../core/engines/core.h"

namespace core
{
    class GameLogic:
        virtual public EngineCore,
        public EngineInterface
    {
    public:
        GameLogic();
        ~GameLogic();
        
        virtual const char *
        get_module_name(void) { return "game"; };

        virtual void
        initialize_module(void);

        virtual void
        update_tick(void);

        virtual void
        update_frame(void);
    };
}

#endif
