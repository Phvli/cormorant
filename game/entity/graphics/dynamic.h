#ifndef _GAME_ENTITY_GRAPHICS_DYNAMIC_H
#define _GAME_ENTITY_GRAPHICS_DYNAMIC_H

#include "../component.h"
#include <vector>

namespace game
{
    class DynamicGraphics:
        virtual public GraphicsComponent
    {
        public:
            typedef
                struct
                {
                    int             id;
                    math::Mat4      transformation;
                    const gfx::Mesh *mesh;
                }
                DynamicMesh;

            DynamicMesh &operator[](int id) { return this->dynamic_meshes[id]; }
            DynamicMesh &operator[](const char *name_name);
            DynamicMesh &operator[](const gfx::Mesh *mesh);
        
            int make_dynamic(const char *mesh_name, int id = -1, bool dynamic = true);
            int make_dynamic(const gfx::Mesh *mesh, int id = -1, bool dynamic = true);
            // Turn dynamic per-mesh transformations for this entity on or off.
            // Model MUST be set before calling these functions.
            // If id is given, assign mesh to that id; otherwise picks the next free one.
            // Return mesh index or -1 if not found on this model
        
            DynamicGraphics();
            virtual ~DynamicGraphics();

            virtual void
            render(void);
            
            virtual DynamicGraphics *
            clone(void) { return new DynamicGraphics(*this); }

        protected:
            typedef
                std::vector<DynamicMesh>
                DynamicMeshes;

            typedef
                std::vector<DynamicMesh *>
                DynamicIndex;

            gfx::Model::Meshes
                static_meshes;
            
            DynamicMeshes
                dynamic_meshes;
            
            DynamicIndex
                dynamic_mesh_index;
            
            void
            sort(void);
    };
}

#endif
