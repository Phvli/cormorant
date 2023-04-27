#include "distance.h"

#include "../../../core/engine.h"
#include "../../../gfx/3d/scene.h"

using namespace game;

DistanceActivation::DistanceActivation()
{
}

void
DistanceActivation::update(void)
{
    if (this->threshold * this->threshold
        > this->entity->pos.dist_sq(screen.scene->player->pos))
    {
        this->entity->flags |= Entity::ACTIVE;
    }
    else
    {
        this->entity->flags &= ~Entity::ACTIVE;
    }
}
