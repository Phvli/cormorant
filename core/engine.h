#ifndef _CORE_ENGINE_H
#define _CORE_ENGINE_H

#include "engines/core.h"
#include "engines/input.h"
#include "engines/video.h"
#include "engines/sound.h"

#include "../game/game_logic.h"

namespace core
{
    class Cormorant:
        virtual public EngineCore,
        virtual public InputEngine,
        virtual public VideoEngine,
        virtual public SoundEngine,
        virtual public GameLogic
    {
    public:
        virtual void
        initialize(void);

        virtual void
        update(void);

        virtual void
        render(void);

        Cormorant();
        ~Cormorant();
    };
    
    extern Cormorant engine;
}

#endif
