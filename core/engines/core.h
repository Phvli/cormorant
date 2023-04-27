#ifndef _CORE_ENGINES_CORE_H
#define _CORE_ENGINES_CORE_H

#include "logger.h"
#include "../message.h"
#include "../util/config.h"

namespace core
{
    class EngineInterface
    {
    public:
        virtual const char *
        get_module_name(void) { return "unnamed module"; };

        virtual void
        initialize_module(void) = 0;
    
        virtual void
        update_tick(void) {};
        // Fixed interval precise update
    
        virtual void
        update_frame(void) {};
        // Updated once per rendered frame
        // This is targeted at engine's frames_per_second setting
        // and the call interval may vary as frames are skipped
    };

    class EngineCore:
        virtual public Logger
    {
    public:
        const unsigned long &ticks;
        
        int
            frames_per_second, // Targeted framerate
            ticks_per_second;  // Physics update interval, generally at least the same as FPS
        
        Config  config;
        Message message;
        
        EngineCore();
        ~EngineCore();
        
        void
        start(void);
        
        bool
        run(void);

        virtual void
        initialize(void) {};

        virtual void
        update(void) {};

        virtual void
        render(void) {};


        EngineInterface *
        get_module(const char *module_name);

        void
        activate_module(EngineInterface *module),
        deactivate_module(EngineInterface *module);
        
        void
        activate_module(const char *module_name) { this->activate_module(this->get_module(module_name)); }

        void
        deactivate_module(const char *module_name) { this->deactivate_module(this->get_module(module_name)); }


    protected:
        void
        register_module(EngineInterface *e, bool activate = true);
        

    private:
        unsigned long
            mutable_ticks,
            start_ticks;
    };
}

#endif
