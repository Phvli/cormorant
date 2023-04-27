#include "spawner.h"

#include "../../../core/engine.h"
#include "../../../gfx/3d/scene.h"

using namespace game;

SpawnerAI::SpawnerAI()
{
    this->interval = 1000;
    this->next     = core::engine.ticks + this->interval;
}

SpawnerAI::SpawnerAI(int interval)
{
    this->interval = interval;
    this->next     = core::engine.ticks + this->interval;
}

void
SpawnerAI::update(void)
{
    if ((this->entity->flags & Entity::ACTIVE)
        && core::engine.ticks >= this->next)
    {
        this->next += this->interval;
        Entity *entity = new Entity(this->prototype);
        entity->pos += this->entity->pos;
        screen.scene->add(entity);
    }
}
