#include "core.h"

#include "../util/file.h"
#include "../util/string.h"
#include "../../version.h"

#include <cstdio>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace core;

#define MAX_MODULE_COUNT 128
#define CURRENT_TICKS (clock() * 1000 / CLOCKS_PER_SEC)

static struct
    {
        EngineInterface
            *active[MAX_MODULE_COUNT],
            *registered[MAX_MODULE_COUNT];
    }
    modules;

EngineCore::EngineCore():
    ticks(mutable_ticks)
{
    this->log_header("Startup");
    
    modules.active[0]     = NULL;
    modules.registered[0] = NULL;
    
    this->start_ticks   = CURRENT_TICKS;
    this->mutable_ticks = 0;
    
    srand(time(NULL));
    this->config.load(DATA_DIRECTORY "settings.cfg");
}

EngineCore::~EngineCore()
{
    this->log("Shutdown complete.");
}

void
EngineCore::start(void)
{
    const char *module_name;
    try {
        for (EngineInterface **e = modules.registered; *e != NULL; ++e)
        {
            module_name = (*e)->get_module_name();
            this->log("Initializing %s subsystem", module_name);
            (*e)->initialize_module();
        }
    }
    catch (int i)
    {
        this->log("(!) %s subsystem threw %i, aborting", module_name, i);
        throw i;
    }

    this->log("Initializing " APP_NAME " engine");
    this->initialize();

    this->log_header(APP_TITLE);
    this->log("Started");

    this->start_ticks   = CURRENT_TICKS;
    this->mutable_ticks = 0;
}

bool
EngineCore::run(void)
{
    try {
        unsigned long
            target_ticks = CURRENT_TICKS - this->start_ticks,
            old_ticks    = this->ticks,
            elapsed      = target_ticks - old_ticks,
            tick_step    = 1000 / this->ticks_per_second;

        for (; this->ticks < target_ticks; this->mutable_ticks += tick_step)
        {
            for (EngineInterface **e = modules.active; *e != NULL; ++e)
            {
                (*e)->update_tick();
            }

            this->update();
        }

        long wait_time = (1000 / this->frames_per_second) - elapsed;
        
        if (wait_time > 0)
        {

#ifdef _WIN32
            Sleep(wait_time);
#else
            usleep(wait_time * 1000);
#endif
            for (EngineInterface **e = modules.active; *e != NULL; ++e)
            {
                (*e)->update_frame();
            }

            this->render();
        }
    }
    catch (int i)
    {
        this->log("Caught %i", i);
        return false;
    }

    return true;
}

void
EngineCore::register_module(EngineInterface *module, bool activate)
{
    EngineInterface **e;
    for (e = modules.registered; *e != NULL; ++e)
    {
        // Prevent accidental double-registration
        if (*e == module)
        {
            return;
        }
    }
    e[0] = module;
    e[1] = NULL;

    if (activate)
    {
        this->activate_module(module);
    }
}

void
EngineCore::activate_module(EngineInterface *module)
{
    if (module)
    {
        EngineInterface **e;
        for (e = modules.active; *e != NULL; ++e)
        {
            // Prevent accidental double-activation
            if (*e == module)
            {
                return;
            }
        }

        e[0] = module;
        e[1] = NULL;
        this->log("Subsystem %s activated", module->get_module_name());
    }
}

void
EngineCore::deactivate_module(EngineInterface *module)
{
    EngineInterface **e;
    for (e = modules.active; *e != NULL && *e != module; ++e);
    if (*e && module)
    {
        this->log("Subsystem %s suspended", module->get_module_name());
    }
    for (; (e[0] = e[1]) != NULL; ++e);
}

EngineInterface *
EngineCore::get_module(const char *module_name)
{
    for (EngineInterface **e = modules.registered; *e != NULL; ++e)
    {
        if (str::cmp(module_name, (*e)->get_module_name()) == 0)
        {
            return *e;
        }
    }
    
    return NULL;
}
