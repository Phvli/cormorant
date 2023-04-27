#ifndef _GFX_3D_MESH_H
#define _GFX_3D_MESH_H

#include <GL/gl.h>
#include <vector>

#include "material.h"
#include "../../math/vec2.h"
#include "../../math/vec3.h"
#include "../../math/vec4.h"
#include "../../math/tri3.h"
#include "../../math/mat4.h"

namespace gfx
{
    class Mesh
    {
        public:
            char *name;
            
            typedef
                struct
                {
                    math::Vec3 pos;
                    math::Vec3 normal;
                    math::Vec2 uv;
                    math::Vec4 uv_weight;
                }
                Vertex;
            
            typedef
                std::vector<Vertex>
                Vertices;

            Vertices
                vertices;

            typedef
                std::vector<int>
                Indices;
                
            Indices
                indices;


            typedef
                struct
                {
                    math::Tri3 triangle;
                    math::Vec3 normal;
                }
                Face;

            typedef
                std::vector<Face>
                Faces;
            
            Faces
                faces;
            
            Material
                *material;
        
            struct
            {
                GLuint index;
                GLuint pos;
                GLuint normal;
                GLuint uv;
                GLuint uv_weight;
            }
            attr;

            Mesh();
            ~Mesh();
            
            Mesh &add(int index_0, int index_1, int index_2);

            Mesh &add(const Vertex &v);
            Mesh &add(const math::Vec3 &pos);
            Mesh &add(const math::Vec3 &pos, const math::Vec2 &uv);
            Mesh &add(const math::Vec3 &pos, const math::Vec3 &normal);
            Mesh &add(const math::Vec3 &pos, const math::Vec3 &normal, const math::Vec2 &uv);
            // Copy a single vertex
            
            Mesh &add(const Mesh &mesh);
            Mesh &add(const Mesh &mesh, float x, float y, float z, float scale = 1.0f);
            Mesh &add(const Mesh &mesh, const math::Vec3 &pos, float scale = 1.0f);
            Mesh &add(const Mesh &mesh, const math::Mat4 &transformation);
            // Copy the contents of another mesh with optional transformations
            
            Mesh &add(Mesh *mesh);
            Mesh &add(Mesh *mesh, float x, float y, float z, float scale = 1.0f);
            Mesh &add(Mesh *mesh, const math::Vec3 &pos, float scale = 1.0f);
            Mesh &add(Mesh *mesh, const math::Mat4 &transformation);
            // Transfer contents from another mesh and delete the original
            
            Mesh &transform(const math::Mat4 &transformation, bool transform_uv = false);
            // Multiply all vertex positions by given matrix
            
            // Convenience shortcuts for chained transformations:
            Mesh &translate(const math::Vec3 &v, bool transform_uv = false);
            Mesh &translate(float x, float y, float z, bool transform_uv = false);
            Mesh &scale(const math::Vec3 &v, bool transform_uv = false);
            Mesh &scale(float x, float y, float z, bool transform_uv = false);
            Mesh &scale(float s, bool transform_uv = false);
            Mesh &rotX(float angle, bool transform_uv = false);
            Mesh &rotY(float angle, bool transform_uv = false);
            Mesh &rotZ(float angle, bool transform_uv = false);
            Mesh &scale_uv(float x, float y);
            Mesh &translate_uv(float x, float y);
            Mesh &transform_uv(const math::Mat4 &transformation);

            Mesh &
            flip(bool normals = true, bool faces = true);
            // Flips normal vectors and/or face chirality (= handedness) around
            
            Mesh &
            build_normals(void);
            // Calculates correct surface normals
            
            Mesh &
            roughen(float max_deviation = 0.1f);
            // Translates all vertex positions at random (invalidates normals)
            
            Mesh &
            subdivide(int interations = 1);
            // Splits all edges in two
            
            Mesh &
            clip(void);
            // Cuts everything below XZ plane
            
            Mesh &
            clip(const math::Vec3 &point_on_plane, const math::Vec3 &plane_normal);
            // Cuts everything behind a plane on which given point lays
            
            Mesh &
            compress(void);
            // Removes duplicate vertices
            
            void
            compose(void);
            // Populate and send vertex attribute arrays to GPU
            
            void
            render(void) const;
            // Renders the mesh with backface culling
            // Assumes transformation and projection matrices already sent

            bool
            intersects(const math::Vec3 &relative_origin, const math::Vec3 &direction) const;
            // Returns true if given line intersects with this mesh

            float
            radius(void) const;
            // Returns largest vertex distance from origin

            math::Vec3
            size(void) const;
            // Returns model size for all axes

            math::Vec3
            center(void) const;
            // Returns center of mass (average vertex position)

            void
            get_bounds(math::Vec3 *negative, math::Vec3 *positive) const;
            // Stores bounding box coordinates into given vectors
    };
}

#include "primitives.h"

#endif
