#ifndef _GFX_3D_MODEL_H
#define _GFX_3D_MODEL_H

#include <GL/gl.h>
#include <vector>

#include "mesh.h"
#include "../../math/vec3.h"
#include "../../math/mat4.h"

namespace gfx
{
    class Model;
    class Model
    {
        public:
            typedef
                std::vector<Mesh *>
                Meshes;
            
            Meshes meshes;
            char * const &filename;

            static Model *
            load(const char *filename);

            void
            unload(void);
            // Safely deletes model

            static void
            flush_cache(void);

            Model();
            Model(Mesh *primitive);
            ~Model();

            void
            render(void) const;

            void
            compose(void);
            // Optimizes the model for rendering
            // by sorting its meshes by their material

            bool
            intersects(const math::Vec3 &relative_origin, const math::Vec3 &direction) const;
            // Returns true if given line intersects with any mesh in this model

            float
            radius(void) const;
            // Returns largest vertex distance from center

            math::Vec3
            size(void) const;
            // Returns model size for all axes

            math::Vec3
            height(void) const;
            // Returns model size in Y axis

            void
            get_bounds(math::Vec3 *negative, math::Vec3 *positive) const;
            // Stores bounding box coordinates into given vectors
            
            void
            add(Mesh *mesh);
            // Binds a mesh to this model

            void add(const Model *model);
            void add(const Model *model, float x, float y, float z, float rot_y = 0.0f);
            void add(const Model *model, const math::Vec3 &pos, float rot_y = 0.0f);
            void add(const Model *model, const math::Vec3 &pos, const math::Vec3 &rot);
            void add(const Model *model, const math::Mat4 &transformation);
            // Copy another model as a submodel

            Mesh *
            get(const char *name);
            // Returns a submesh by given name or NULL if doesn't exist

        protected:
            Meshes opaque_meshes, transparent_meshes;
            char *filename_mutable;
    };
}

#endif
