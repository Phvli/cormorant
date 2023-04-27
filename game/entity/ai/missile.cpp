#include "missile.h"

#include "../create.h"

#include "../../../core/engine.h"
#include "../../../math/util.h"
#include "../../../gfx/3d/scene.h"
#include "../../../game/terrain.h"
#include "../physics/dynamic.h"

using namespace game;

MissileAI::MissileAI()
{
    core::engine.play_sound("launch");
}

void
MissileAI::update(void)
{
    if (this->entity->flags & Entity::ACTIVE)
    {
        this->entity->physics->accel.z
            = 100.0f * (this->entity->physics->velocity.length() < 2.5f * math::convert::MACH_TO_MS);

        Entity *trail = create::trail(this->entity->pos
            + this->entity->physics->velocity / core::engine.ticks_per_second);
        trail->attach(new DynamicPhysics());
        trail->physics->collision_detection = false;
        trail->physics->velocity            = this->entity->physics->velocity;
        
        screen.scene->add(trail);
    }
    else
    {
        this->entity->physics->velocity.x *= .98f;
        this->entity->physics->velocity.z *= .98f;
        
        this->entity->physics->velocity.y -= 9.81f / core::engine.ticks_per_second;
    }

    // (!) TODO: use messages to detect collisions
    float alt = this->entity->physics->altitude();
    if (alt <= 0.0f)
    {
        // screen.scene->add(create::explosion(this->entity->pos));
        create::explosion(this->entity->pos - alt, 5.0f);
        this->entity->destroy();
    }
}
