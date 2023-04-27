/*
    Class to contain global scene related variables
    Holds camera, models and matrices.
    
    Default values:
    - Perspective projection enabled.
    - Clipping planes between 0.1 cm and 1 km
    - Field of view 60 degrees.

    Phvli 2017-08-07
*/

#ifndef _GFX_3D_SCENE_H
#define _GFX_3D_SCENE_H

#include "../../math/mat4.h"
#include "../../math/vec4.h"
#include "../../game/entity.h"
#include "mesh.h"
#include "skybox.h"

#include <vector>

namespace gfx
{
    class Scene
    {
        public:
            const int
                &frame,
                &w, &h;
            
            const float
                &zoom,
                &fov,
                &aspect_ratio,
                &clip_near,
                &clip_far;

            const bool
                &perspective;

            math::Vec4
                sunlight,
                fog;
                
            struct
            {
                math::Mat4
                    mv,
                    camera,
                    normal,
                    projection;
            } matrix;

            
            gfx::Mesh
                *ground_plane,
                *waves;
            
            gfx::Skybox
                *skybox;
            
            game::Entity
                *player,
                camera;
            
            typedef
                std::vector<game::Entity *>
                EntityContainer;
            
            EntityContainer
                entities;

            Scene();
            ~Scene();
            
            void
            resize(int w, int h) { if (w != this->w || h != this->h) { this->w_mutable = w; this->h_mutable = h; this->update_projection(); } }

            void
            set_perspective(bool enabled) { if (enabled != this->perspective) { this->perspective_mutable = enabled; this->update_projection(); } }

            void
            set_zoom(float zoom) { if (zoom != this->zoom) { this->zoom_mutable = zoom; this->update_projection(); } }

            void
            set_fov(float fov) { if (fov != this->fov) { this->fov_mutable = fov; this->update_projection(); } };

            void
            clip(float near, float far);

            void
            render(void);
            
            void
            update(void);
            
            void
            build_ground_plane(void);

            void
            render_ground_plane(void);

            game::Entity *
            add(game::Entity *entity);

            game::Entity *
            remove(game::Entity *entity, bool destroy = false);
        
        protected:
            int
                frame_mutable,
                w_mutable, h_mutable;

            float
                zoom_mutable,
                fov_mutable,
                aspect_ratio_mutable,
                clip_near_mutable,
                clip_far_mutable;
            
            bool
                perspective_mutable;

            void
            update_projection(void);
    };
}

#endif
