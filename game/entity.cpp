#include "entity.h"

#include "../core/engine.h"

#include <algorithm>

using namespace game;

static int static_uniq_id = 0;

Entity::Entity():
    id(id_mutable),
    x(pos.x),
    y(pos.y),
    z(pos.z)
{
    this->init(static_uniq_id++);
}

Entity::Entity(int id):
    id(id_mutable),
    x(pos.x),
    y(pos.y),
    z(pos.z)
{
    // (!) FIXME: TODO: prevent id collisions

    if (id >= static_uniq_id)
    {
        static_uniq_id = id + 1;
    }

    this->init(id);
}

Entity::Entity(const Entity &entity):
    id(id_mutable),
    x(pos.x),
    y(pos.y),
    z(pos.z)
{
    core::engine.log("Copying entity #%i (base at %x):", entity.id, &entity);
    this->init(static_uniq_id++);

    this->flags          = entity.flags;
    this->conf           = NULL;

    this->pos            = entity.pos;
    this->transformation = entity.transformation;
    this->creation_ticks = core::engine.ticks;

    if (entity.activation != NULL) this->attach(entity.activation->clone());
    if (entity.ai != NULL)         this->attach(entity.ai->clone());
    if (entity.physics != NULL)    this->attach(entity.physics->clone());
    if (entity.graphics != NULL)   this->attach(entity.graphics->clone());

    core::engine.log("Entity #%i copied", entity.id);
}
void
Entity::init(int id)
{
    core::engine.log("Creating entity #%i (base at %x)", id, this);
    this->id_mutable = id;
    this->flags      = Entity::ACTIVE;
    this->conf       = NULL;

    this->transformation = math::Mat4::identity();
    this->creation_ticks = core::engine.ticks;

    this->activation = NULL; this->attach(new ActivationComponent());
    this->ai         = NULL; this->attach(new AIComponent());
    this->graphics   = NULL; this->attach(new GraphicsComponent());
    this->physics    = NULL; this->attach(new PhysicsComponent());

    core::engine.log("<(OK)");
}

Entity::~Entity()
{
    core::engine.log("Deleting entity #%i (base at %x)", this->id, this);
    
    delete this->activation;
    delete this->ai;
    delete this->graphics;
    delete this->physics;

    delete this->conf;

    core::engine.log("<(OK)");
}

Entity *
Entity::attach(ActivationComponent *component)
{
    delete this->activation;
    this->activation = component;
    
    return component->entity = this;
}

Entity *
Entity::attach(AIComponent *component)
{
    delete this->ai;
    this->ai = component;
    
    return component->entity = this;
}

Entity *
Entity::attach(GraphicsComponent *component)
{
    delete this->graphics;
    this->graphics = component;
    
    return component->entity = this;
}

Entity *
Entity::attach(PhysicsComponent *component)
{
    delete this->physics;
    this->physics = component;
    
    return component->entity = this;
}

void
Entity::destroy(void)
{
    this->flags |= Entity::DELETED;
    core::engine.log("Requested entity #%i deletion", this->id);
}

void
Entity::bind(core::Config *config)
{
    this->conf = config;

    (*this->conf)["position"]["x"].bind(this->pos.x);
    (*this->conf)["position"]["y"].bind(this->pos.y);
    (*this->conf)["position"]["z"].bind(this->pos.z);

    (*this->conf)["rotation"]["w"].bind(this->rot.w);
    (*this->conf)["rotation"]["x"].bind(this->rot.x);
    (*this->conf)["rotation"]["y"].bind(this->rot.y);
    (*this->conf)["rotation"]["z"].bind(this->rot.z);

    (*this->conf)["heading"].bind(this->rot.y);
    (*this->conf)["depth"].bind(this->pos.y);

    this->activation->bind(config);
    this->ai->bind(config);
    this->graphics->bind(config);
    this->physics->bind(config);
}

void
Entity::update(void)
{
    this->activation->update();
    this->ai->update();
    this->graphics->update();
    this->physics->update();
}

void
Entity::add_tag(const char *tag)
{
    core::Config::Hash hash = core::Config::get_hash(tag);
    TagContainer::iterator t
        = std::find(this->tags.begin(), this->tags.end(), hash);

    if (t == this->tags.end())
    {
        this->tags.push_back(hash);
    }
}

void
Entity::remove_tag(const char *tag)
{
    core::Config::Hash hash = core::Config::get_hash(tag);
    TagContainer::iterator t
        = std::find(this->tags.begin(), this->tags.end(), hash);

    if (t != this->tags.end())
    {
        this->tags.erase(t);
    }
}

bool
Entity::has_tag(const char *tag)
const
{
    core::Config::Hash hash = core::Config::get_hash(tag);
    return (std::find(this->tags.begin(), this->tags.end(), hash)
        != this->tags.end());
}

void
Entity::invoce(game::Message *message)
{
    throw 666; // (!) FIXME: TODO: global message handling
}

void
Entity::receive(game::Message *message)
{
    this->activation->receive(message);
    this->ai->receive(message);
    this->graphics->receive(message);
    this->physics->receive(message);
}
