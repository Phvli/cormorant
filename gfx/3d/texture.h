#ifndef _GFX_3D_TEXTURE_H
#define _GFX_3D_TEXTURE_H

#include <GL/gl.h>
#include "../sprite.h"

namespace gfx
{
    class Texture
    {
        public:
            typedef unsigned long int
                Flags;
            
            static const Flags
                NEAREST     = (0x00000001 << 0),
                NO_MIPMAP   = (0x00000001 << 1),

                FLIP_X      = (0x00000001 << 10),
                FLIP_Y      = (0x00000001 << 11),
                CLAMP_X     = (0x00000001 << 12),
                CLAMP_Y     = (0x00000001 << 13),
                CLAMP       = (CLAMP_X | CLAMP_Y),

                CUBEMAP     = (0x00000001 << 29),
                FRAMEBUFFER = (0x00000001 << 30),
                DEPTHBUFFER = (0x00000001 << 31),

                AUTO        = 0;
            
            Flags
                flags;
            
            const int  &w, &h;
            const bool &ready;
            const bool &translucent;
            
            GLenum
                type;

            GLuint
                id,
                framebuffer_id,
                depthbuffer_id;

            Texture();
            ~Texture();

            static Texture *
            get(const char *filename, Flags flags = AUTO, int index = 0);

            static Texture *
            load(const Sprite *sprite, Flags flags = AUTO, int index = 0) { return load(&sprite, flags, index); }
        
            static Texture *
            load(const Sprite **sprite, Flags flags = AUTO, int index = 0);
        
            static Texture *
            framebuffer(const char *name, int w = 512, int h = 512, bool alpha = false, unsigned int depth_bits = 0);
            // Returns a previously created framebuffer of the same name, or
            // creates a new framebuffer with an optional renderbuffer if depth_bits > 0.
            // If set, depth_bits should be either 16, 24 or 32 (24 is used as a fallback).

            void
            bind(void) const;
            // Binds this as the target of OpenGL texture operations (e.g. drawing)

            void
            unbind(void) const;
            // Breaks texture binding

            void
            target(bool render_to_texture = true);
            // Sets this framebuffer as the rendering target

            bool
            attach(const Sprite *sprite, Flags flags, GLenum target);
        
            static void
            flush_cache(void);
            
            void
            blit(void) const,
            blit(float x, float y, float opacity = 1.0f, float rotation = 0.0f) const,
            blit(float x, float y, float w, float h, float opacity = 1.0f, float rotation = 0.0f) const,
            blit(float x, float y, float w, float h, float x_src, float y_src, float w_src, float h_src, float opacity = 1.0f, float rotation = 0.0f) const;

        protected:
            int
                w_mutable,
                h_mutable;

            bool
                ready_mutable,
                translucent_mutable;
    };
}

#endif
