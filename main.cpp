#include "main.h"
#include "core/engine.h"
#include "core/engines/input.h"

#include <cstdio>
int
main(int argc, char *argv[])
{
    try
    {
        for (
            core::engine.start();
            core::engine.run() && !input[core::Input::QUIT];
        );
            
    }
    catch (int i)
    {
        core::engine.log("(!) Caught %i, aborting...", i);
    }

    return 0;
}
