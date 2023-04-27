#ifndef _GAME_ENTITY_COMPONENT_H
#define _GAME_ENTITY_COMPONENT_H

#include "../../core/util/config.h"
#include "../../math/vec3.h"
#include "../../math/mat4.h"
#include "../../gfx/3d/model.h"

namespace game
{
    typedef
        void *
        Message;

    class Entity;

    class EntityComponent;
    class EntityComponent
    {
        friend class Entity;

        public:
            virtual ~EntityComponent() {}

            virtual void
            update(void) = 0;

            virtual void
            receive(Message *message) {}

            virtual void
            bind(core::Config *conf) {}
        
            virtual EntityComponent *
            clone(void) = 0;
        
        protected:
            Entity *entity;
    };

    class ActivationComponent;
    class ActivationComponent: public EntityComponent
    {
        public:
            int threshold;

            ActivationComponent(): threshold(0) {}
            
            virtual void
            update(void) {}

            virtual void
            bind(core::Config *conf);
            
            virtual ActivationComponent *
            clone(void);
    };

    class AIComponent;
    class AIComponent: public EntityComponent
    {
        public:
            virtual void
            update(void) {}

            virtual void
            bind(core::Config *conf);
            
            virtual AIComponent *
            clone(void);
    };

    class GraphicsComponent;
    class GraphicsComponent: public EntityComponent
    {
        public:
            gfx::Model
                *model;
            
            bool
                visible;

            GraphicsComponent(): visible(false) {}

            virtual void
            update(void) {}

            virtual void
            render(void) {}

            virtual void
            bind(core::Config *conf);
            
            virtual GraphicsComponent *
            clone(void);
    };

    class PhysicsComponent;
    class PhysicsComponent: public EntityComponent
    {
        public:
            math::Vec3
                velocity,
                accel,
                turn_rate;
            
            bool
                collision_detection;
                
            PhysicsComponent(): collision_detection(false) {}

            virtual void
            update(void) {}

            math::Vec3
            vector(void) const;
            // Returns movement vector

            math::Vec3
            attitude(void) const;
            // Returns attitude vector

            float yaw(void) const;
            float pitch(void) const;
            float roll(void) const;
            // Returns attitude in one axis

            float heading(void) const;
            // returns Y rotation in degrees

            float altitude(void) const;
            // returns Y distance from ground

            bool
            intersects(const math::Vec3 &relative_origin, const math::Vec3 &direction) const;
            // Returns true if given line intersects with any mesh in this model

            float
            radius(void) const;
            // Returns largest vertex distance from center

            math::Vec3
            size(void) const;
            // Returns model size for all axes

            void
            get_bounds(math::Vec3 *negative, math::Vec3 *positive) const;
            // Stores bounding box coordinates into given vectors

            virtual void
            bind(core::Config *conf);
            
            virtual PhysicsComponent *
            clone(void);
    };
}

#include "../entity.h"

#endif
