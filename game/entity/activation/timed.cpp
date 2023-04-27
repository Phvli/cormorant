#include "timed.h"

#include "../../../core/engine.h"
#include "../../../gfx/3d/scene.h"

using namespace game;

TimedActivation::TimedActivation(int ticks)
{
    this->threshold = ticks;
}

void
TimedActivation::update(void)
{
    if (core::engine.ticks
        > this->entity->creation_ticks + this->threshold)
    {
        this->entity->flags |= Entity::ACTIVE;
    }
    else
    {
        this->entity->flags &= ~Entity::ACTIVE;
    }
}

TimedDeactivation::TimedDeactivation(int ticks)
{
    this->threshold = ticks;
}

void
TimedDeactivation::update(void)
{
    if (core::engine.ticks
        < this->entity->creation_ticks + this->threshold)
    {
        this->entity->flags |= Entity::ACTIVE;
    }
    else
    {
        this->entity->flags &= ~Entity::ACTIVE;
    }
}

TimedDestruction::TimedDestruction(int ticks)
{
    this->threshold = ticks;
}

void
TimedDestruction::update(void)
{
    if (core::engine.ticks
        < this->entity->creation_ticks + this->threshold)
    {
        this->entity->flags |= Entity::ACTIVE;
    }
    else
    {
        this->entity->destroy();
    }
}
