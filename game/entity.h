#ifndef _GAME_ENTITY_H
#define _GAME_ENTITY_H

#include "entity/component.h"

#include "../core/util/config.h"
#include "../math/quat.h"
#include "../math/mat4.h"
#include "../math/vec3.h"

namespace game
{
    class Entity
    {
        friend class EntityComponent;
        
        public:
            const int &id;
            float &x, &y, &z;
            
            math::Vec3 pos;
            math::Quat rot;
            math::Mat4 transformation;
            
            typedef
                unsigned long int
                Flags;
                
            static const Flags
                ACTIVE      = (0x00000001 << 0),
                TRANSLUCENT = (0x00000001 << 1),
                DELETED     = (0x00000001 << 31),

                AUTO        = 0;
            
            Flags flags;
            
            unsigned long int
                creation_ticks;

            // (!) FIXME: TODO: somehow prevent changing these pointers directly
            ActivationComponent *activation;
            AIComponent         *ai;
            GraphicsComponent   *graphics;
            PhysicsComponent    *physics;
            
            Entity();
            Entity(const Entity &entity);
            Entity(int id);
            ~Entity();

            // Enable new functionality (all return this):
            Entity *attach(ActivationComponent *component);
            Entity *attach(AIComponent *component);
            Entity *attach(GraphicsComponent *component);
            Entity *attach(PhysicsComponent *component);
        
            void
            invoce(game::Message *message);
            // Broadcast a public message

            void
            receive(game::Message *message);
            // Send a message for this entity to handle

            void
            update(void);
            // Run per-frame tasks
        
            void add_tag(const char *tag);
            void remove_tag(const char *tag);
            bool has_tag(const char *tag) const;
        
            void
            destroy(void);
            // Mark object for deletion (only safe way to delete objects in the update function)
        
            void
            bind(core::Config *config);
            // Bind entity data to given config
        
        protected:
            core::Config *conf;
            
            typedef
                std::vector<core::Config::Hash>
                TagContainer;

            TagContainer
                tags;

        private:
            int id_mutable;
            
            void
            init(int id);
    };
}


#endif
