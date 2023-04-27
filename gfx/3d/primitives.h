#ifndef _GFX_3D_PRIMITIVES_H
#define _GFX_3D_PRIMITIVES_H

/*
    Primitive shapes to construct 3D models with.
    
    Created shapes measure from -1.0 to 1.0 in each dimension,
    except for planar shapes which lay flat on the XZ plane.
    
    All functions create a new gfx::Mesh, which must either
    be deallocated by the caller or used in pointer form in
    gfx::Mesh::add, which deallocates reserved memory afterwards.
    
    Phvli 2017-10-25
*/

#include "mesh.h"

namespace gfx
{
    namespace primitive
    {
        Mesh *
        quad();
        // Unindexed quad of 4 vertices to be rendered as GL_TRIANGLE_STRIP
        
        Mesh *
        triangle();
        // Unit triangle laying on the XZ plane
        
        Mesh *
        plane();
        // Unit square laying on the XZ plane
        
        Mesh *
        circle(int corners = 16);
        // Circle with a radius of 1.0, laying on the XZ plane
        
        Mesh *
        ring(int corners = 16, float thickness = 0.1f);
        // Rign with an outer radius of 1.0, laying on the XZ plane
        
        Mesh *
        cube(bool uv_mapped = true);
        // Unit cube
        
        Mesh *
        cone(int corners = 16, bool hollow = false);
        // Unit cone, 2.0 in both height and diameter, peak pointing up,
        // bottom covered with a circle unless hollow
        
        Mesh *
        cylinder(int corners = 16, bool hollow = false);
        // Unit cylinder, 2.0 in both height and diameter,
        // ends covered with circles unless hollow
        
        Mesh *
        tapered_cylinder(int corners = 16, float taper = 0.5f, bool hollow = false);
        // Unit cylinder, 2.0 in both height and bottom diameter,
        // upper end diameter multiplied by taper coefficient,
        // ends covered with circles unless hollow
        
        Mesh *
        tube(int corners = 16, float thickness = 0.1f);
        // Unit-sized square toroid with both inner and outer surfaces
        
        Mesh *
        torus(int corners = 16, float thickness = 0.1f);
        // Unit-sized torus, with both inner and outer surfaces
        
        Mesh *
        sphere(int parallels, int meridians);
        inline Mesh *sphere(int subdiv = 16) { return sphere(subdiv, subdiv); }
        // Unit-sized UV-sphere
        
        Mesh *
        half_sphere(int parallels, int meridians, bool hollow = false);
        inline Mesh *half_sphere(int subdiv = 16, bool hollow = false) { return half_sphere(subdiv, subdiv, hollow); }
        // Unit-sized upper half of an UV-sphere
        
        Mesh *
        icosphere(int subdiv = 1);
        // Unit-sized optionally subdivided icosahedron (20 faces minimum)
        
        Mesh *
        grid(int cells = 16, bool double_sided = false);
        // Grid mesh of unit sized cells laying on the XZ plane
    }
}

#endif
