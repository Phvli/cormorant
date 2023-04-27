#include "create.h"

#include "../../core/engine.h"
#include "../../core/util/string.h"
#include "../../math/util.h"
#include "ai/spawner.h"
#include "physics/dynamic.h"
#include "activation/timed.h"
#include "graphics/sprite.h"
#include "../../game/terrain.h"

using namespace game;

Entity *
game::create::explosion(const math::Vec3 &pos, float strength)
{
    game::TerrainNode terrain = game::terrain.at(pos);
    float dist = (pos - screen.scene->camera.pos).length();
    
    if (terrain.height < 1.0f)
    {
        core::engine.play_sound_at("splash", dist, strength);
        for (int i = 0; i <= strength * 10; ++i)
        {
            // Smoke
            Entity *entity = new Entity();
            entity->attach(new SpriteGraphics("bubbles.png", 1,
                strength * 5.0f,
                math::vary(.003, .0005f) / strength,
                .002f)
            );
            entity->flags |= Entity::TRANSLUCENT;
            entity->pos    = pos;
            entity->pos.y  = -strength;

            entity->attach(new DynamicPhysics());
            entity->physics->collision_detection = false;
            entity->physics->velocity   = math::Vec3::random() * strength * 2.0f;
            entity->physics->velocity.y = (float)i * 1.5;
            entity->physics->accel.y    = -9.81f;

            screen.scene->add(entity);
        }

        return NULL;
    }
    
    if (terrain.vegetation > .9f)
    {
        create::fire(pos + math::Vec3(0.0f, 0.0f, 0.0f));
    }

    char *sfx = core::str::format("explosion_%i", (int)math::min((int)dist / 1000, 1));
    core::engine.play_sound_at(sfx, dist, strength * 40.0f);
    delete[] sfx;
    
    // Flash
    Entity *entity = new Entity();
    entity->attach(new SpriteGraphics("spark.png", 1, strength * 30.0f, .05f));
    entity->flags |= Entity::TRANSLUCENT;
    entity->pos    = pos;
    screen.scene->add(entity);

    entity = new Entity();
    entity->attach(new SpriteGraphics("flash.png", 1, strength * 10.0f, .02f));
    entity->flags |= Entity::TRANSLUCENT;
    entity->pos    = pos;
    screen.scene->add(entity);

    for (int i = 0; i <= strength; ++i)
    {
        // Smoke
        Entity *entity = new Entity();
        entity->attach(new SpriteGraphics("trail.png", 1,
            strength * 10.0f,
            math::vary(.012, .004f) / strength));
        entity->flags |= Entity::TRANSLUCENT;
        entity->pos    = pos;
        entity->y     -= strength * 2.0f;

        entity->attach(new DynamicPhysics());
        entity->physics->collision_detection = false;
        entity->physics->velocity    = math::Vec3::random() * strength * 2.0f;
        entity->physics->velocity.y *= 0.2f;
        entity->physics->accel.y     = math::vary(1.0f, 0.5f);

        screen.scene->add(entity);
    }

    for (int i = 0; i <= strength * 4 + 2; ++i)
    {
        // Fire
        Entity *entity = new Entity();
        entity->attach(new SpriteGraphics("explosion.png", 6,
            math::vary(5.0f, 2.0f) * strength,
            math::vary(.02, .005f) / strength + .002f));
        entity->flags |= Entity::TRANSLUCENT;
        entity->pos    = pos + math::Vec3::random() * strength;
        entity->pos.y += strength * 2.0f;

        entity->attach(new DynamicPhysics());
        entity->physics->collision_detection = false;
        entity->physics->velocity = math::Vec3::random() * strength;
        entity->physics->velocity.y += strength;
        entity->physics->velocity.y *= 2.5f;
        entity->physics->accel.y     = -9.81f;

        screen.scene->add(entity);
    }
    
    return NULL;
}

Entity *
game::create::fire(const math::Vec3 &pos, unsigned long int lifetime)
{
    Entity *entity = new Entity();
    entity->pos = pos;
    if (lifetime)
    {
        entity->attach(new TimedDestruction(lifetime));
    }

    SpawnerAI *spawner = new SpawnerAI(1000);
    spawner->prototype.attach(new SpriteGraphics("smoke.png", 1, 50.0f, .001f, .001f));
    spawner->prototype.flags |= Entity::TRANSLUCENT;
    spawner->prototype.attach(new DynamicPhysics());
    spawner->prototype.pos.y = -100;
    spawner->prototype.physics->collision_detection = false;
    spawner->prototype.physics->velocity.y = 20.0f;
    spawner->prototype.physics->accel.y    = 3.0f;
    entity->attach(spawner);
    screen.scene->add(entity);

    entity = new Entity();
    entity->pos = pos;
    if (lifetime)
    {
        entity->attach(new TimedDestruction(lifetime));
    }
    spawner = new SpawnerAI(500);
    spawner->prototype.attach(new SpriteGraphics("fire.png", 3, 40.0f, .015f, 0.0f));
    spawner->prototype.flags |= Entity::TRANSLUCENT;
    spawner->prototype.attach(new DynamicPhysics());
    spawner->prototype.physics->collision_detection = false;
    spawner->prototype.physics->accel.y = 20.0f;
    entity->attach(spawner);
    screen.scene->add(entity);

    return NULL;
}

Entity *
game::create::smoke(const math::Vec3 &pos, unsigned long int lifetime)
{
    Entity *entity = new Entity();
    entity->attach(new TimedDestruction(lifetime));
    entity->attach(new SpriteGraphics("smoke.png", 1));
    entity->flags |= Entity::TRANSLUCENT;
    entity->pos    = pos;

    entity->attach(new DynamicPhysics());
    entity->physics->collision_detection = false;
    entity->physics->accel.y = 0.5f;
    entity->physics->velocity.y = 2.0f;

    return entity;
}

Entity *
game::create::trail(const math::Vec3 &pos, unsigned long int lifetime)
{
    Entity *entity = new Entity();
    entity->attach(new SpriteGraphics("trail.png", 1, 1.0f,
        (float)core::engine.ticks_per_second / (float)lifetime + math::rnd(0.004f), .005f));
    entity->flags |= Entity::TRANSLUCENT;
    entity->pos    = pos;

    screen.scene->add(entity);
    entity = new Entity();
    entity->attach(new SpriteGraphics("spark.png", 1, 1.0f,
        0.1f, .005f));
    entity->flags |= Entity::TRANSLUCENT;
    entity->pos    = pos;
    return entity;
}
