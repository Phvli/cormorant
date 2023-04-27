#include "dynamic.h"

#include "../../terrain.h"
#include "../../../core/engine.h"

#include <cmath>

using namespace game;

void
DynamicPhysics::update(void)
{
    // Thrust
    this->velocity
        += math::Vec3()
        * (math::Mat4::translation(this->accel)
            * this->entity->rot)
        / core::engine.ticks_per_second;

    // Movement
    this->entity->pos
        += this->velocity
        / core::engine.ticks_per_second;

    // Rotation: (yaw * pitch) * roll
    math::Quat rotation
        = (
            math::Quat::rotation_y(this->turn_rate.y)
            * math::Quat::rotation_x(this->turn_rate.x)
        )
        * math::Quat::rotation_z(this->turn_rate.z);

    // Turning; new attitude = rotation * old attitude
    this->entity->rot = (rotation * this->entity->rot).normalize();

    if (this->collision_detection)
    {
        // (!) TODO: send messages instead
        float terrain = this->altitude();
        if (terrain < .5f)
        {
            this->entity->y += 2.0f - terrain;
            this->entity->rot = math::Quat::slerp(this->entity->rot, math::Quat::facing(math::Vec3::up()), .5f);
            core::engine.play_sound("wilhelm_scream");
            this->accel *= .8f;
            this->velocity = math::Vec3(
                this->velocity.x * .5,
                1.0f + abs(this->velocity.y),
                this->velocity.z * .5);
        }
    }
}

DynamicPhysics::DynamicPhysics()
{
    this->collision_detection = true;
}

DynamicPhysics::~DynamicPhysics()
{
}
