#include "component.h"

#include <cmath>

#include "../../core/engine.h"
#include "../../math/util.h"
#include "../terrain.h"

using namespace game;

ActivationComponent *
ActivationComponent::clone(void)
{
    return new ActivationComponent(*this);
}

AIComponent *
AIComponent::clone(void)
{
    return new AIComponent(*this);
}

GraphicsComponent *
GraphicsComponent::clone(void)
{
    return new GraphicsComponent(*this);
}

PhysicsComponent *
PhysicsComponent::clone(void)
{
    return new PhysicsComponent(*this);
}

void
ActivationComponent::bind(core::Config *conf)
{
}

void
AIComponent::bind(core::Config *conf)
{
}

void
GraphicsComponent::bind(core::Config *conf)
{
    (*conf)["visible"].bind(this->visible);
}

void
PhysicsComponent::bind(core::Config *conf)
{
    (*conf)["velocity"]["x"].bind(this->velocity.x);
    (*conf)["velocity"]["y"].bind(this->velocity.y);
    (*conf)["velocity"]["z"].bind(this->velocity.z);

    (*conf)["accel"]["x"].bind(this->accel.x);
    (*conf)["accel"]["y"].bind(this->accel.y);
    (*conf)["accel"]["z"].bind(this->accel.z);

    (*conf)["turn_rate"]["x"].bind(this->turn_rate.x);
    (*conf)["turn_rate"]["y"].bind(this->turn_rate.y);
    (*conf)["turn_rate"]["z"].bind(this->turn_rate.z);

    (*conf)["speed"].bind(this->velocity.z);

    (*conf)["collision_detection"].bind(this->collision_detection);
}

math::Vec3
PhysicsComponent::vector(void)
const
{
    return math::Vec3()
        * (
            math::Mat4::translation(
                this->velocity / core::engine.ticks_per_second
            )
            * this->entity->rot
        );
}

math::Vec3
PhysicsComponent::attitude(void)
const
{
    return math::Vec3::forward()
        * (math::Mat4::identity() * this->entity->rot);
}

bool
PhysicsComponent::intersects(const math::Vec3 &relative_origin, const math::Vec3 &direction)
const
{
    if (this->entity->graphics != NULL && this->entity->graphics->model != NULL)
    {
        // (!) FIXME: take current rotation into account
        math::Vec3 translated_origin    = relative_origin;
        math::Vec3 translated_direction = direction;

        return this->entity->graphics->model
            ->intersects(translated_origin, translated_direction);
    }
    
    return false;
}

float
PhysicsComponent::radius(void)
const
{
    if (this->entity->graphics != NULL && this->entity->graphics->model != NULL)
    {
        return this->entity->graphics->model->radius();
    }
    
    return 0.0f;
}

math::Vec3
PhysicsComponent::size(void)
const
{
    if (this->entity->graphics != NULL && this->entity->graphics->model != NULL)
    {
        // (!) FIXME: take current rotation into account
        return this->entity->graphics->model->size();
    }
    
    return math::Vec3(0.0f);
}

void
PhysicsComponent::get_bounds(math::Vec3 *negative, math::Vec3 *positive)
const
{
    if (this->entity->graphics != NULL && this->entity->graphics->model != NULL)
    {
        // (!) FIXME: take current rotation into account
        this->entity->graphics->model->get_bounds(negative, positive);
    }
    else
    {
        *negative = math::Vec3(0.0f);
        *positive = math::Vec3(0.0f);
    }
}

float
PhysicsComponent::yaw(void)
const
{
    math::Vec3 v = this->attitude();
    return atan2(v.z, v.x);
}

float
PhysicsComponent::pitch(void)
const
{
    return atan2(this->attitude().y, 1.0f);
}

float
PhysicsComponent::roll(void)
const
{
    return 0.0f; // (!) FIXME: TODO
}

float
PhysicsComponent::heading(void)
const
{
    return fmod(this->yaw() * math::convert::RAD_TO_DEG + 450.0f, 360.0f);
}

float
PhysicsComponent::altitude(void)
const
{
    return this->entity->y
        - math::max(
            0.0f,
            game::terrain.at(this->entity->x, this->entity->z).height
        );
}
