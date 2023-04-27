/*
    A set of basic ARGB pixel-manipulation
    and sprite handling functions.

    This C++ version is decoupled from any actual GFX
    system (inherit to implement copying/rendering).

    * ANTI-ALIASING:
    Most functions have an a-variant that
    take floating point coordinates and
    produce anti-aliased results at the cost
    of some computational overhead:
    sprite->blit_a(source, 3.14f, 4.2f);

    * TRANSLUCENCY:
    Some functions have a variant with an additional
    float parameter that produces translucent
    translucent results. Opacity must range from
    0.0f (fully transparent) to 1.0f (fully opaque):
    sprite->blit(source, 3, 4, 0.75f);

    * All colors are 32-bit ARGB (with 8-bit
    components). Alpha channel is ignored
    unless alpha_blending is set.

    * Draw functions overwrite all channels,
    alpha included. To draw translucent shapes,
    use their t-variants instead.

    * Currently only Works on little-endian
    systems, like x86 and x86-64.


    Phvli 2016-10-06 - 2017-08-01
*/

#ifndef _GFX_SPRITE_H
#define _GFX_SPRITE_H

namespace gfx
{
    typedef
        unsigned long int
        Color;
    
    typedef
        unsigned char
        Component;

    class Sprite
    {
    public:
        static const int
            BYTES_PER_PIXEL = 4;

        typedef
            enum
            {
                IGNORE_ALPHA = 0, // alpha channel ignored (fastest)
                BINARY_ALPHA = 1, // pixels either fully transparent or opaque
                FULL_ALPHA   = 2  // full 8-bit alpha channel (slowest)
            }
            AlphaMode;

        typedef
            enum {
                /* basic blending: */
                FADE,     /* fade-in */
                ADD,      /* dodge (dst + src) */
                SUBTRACT, /* burn (dst - src) */
                MULTIPLY, /* (dst * src) */
                
                /* transitional effects: */
                RANDOM,   /* random pixels from src */
                DISSOLVE, /* like RANDOM but softer */
                DROP,     /* slide in from above */
                SLIDE,    /* slide in from left */
                CIRCLE    /* peephole centered at (x, y) */
            }
            BlendMode;

        // raw ARGB8888 pixel data and dimensions
        Color *data;
        const int &w, &h;
        
        // clipping area (defaults to full sprite)
        struct
        {
            bool enabled;
            /* Can be turned off to speed up drawing.
               Note that putpixel & getpixel must be
               clipped by the caller. */
            
            int x, y;
            int w, h;
        } clipping;
        
        // alpha blending mode
        AlphaMode alpha_blending;

        Sprite();
        Sprite(int w, int h);
        Sprite(const Sprite *copy);
        Sprite(const char *filename);
        
        ~Sprite();

        void  // Resize sprite (while losing pixel data)
        resize(int w, int h);

        void  // Fill sprite with given color
        clear(Color = 0xff000000);

        Sprite * // Return a scaled copy
        scale(float zoom) const;
        
        Sprite * // Return a scaled copy
        scale(float w, float h) const;
        
        Sprite * // Scale using nearest-neighbor interpolation (= pixelate)
        scale_nearest(float w, float h) const;
        
        Sprite * // Scale using bilinear interpolation
        scale_bilinear(float w, float h) const;
        // Used by default for shrinking images
        
        Sprite * // Scale using bicubic interpolation
        scale_bicubic(float w, float h) const;
        // Used by default for enlargening images

        void  // Overwrite all pixels' color values that match given <mask>
        colorize(Color color, Color mask = 0x00ffffff);
        // EXAMPLE: colorize(0x00000000, 0x00ff0000) mutes red

        void  // Make a single color transparent or semi-transparent
        make_transparent(Color c),
        make_transparent(Color c, float opacity);
        /* Modifies the alpha channel and sets the alpha_blending flag;
           sprites can have multiple transparent colors at no additional
           cost.)
        */

        void  // Multiply every pixel's alpha by <opacity>
        multiply_alpha(float opacity);

        void  // Copy a color channel into alpha channel
        channel_to_alpha(Color channel_mask);
        // EXAMPLE: channel_to_alpha(0x00ff0000) copies red into alpha

        void  // Fill the whole sprite with solid color
        fill(Color c),
        fill(Color c, float opacity);

        void  // Draw another sprite (partially or in full)
        blit(const Sprite *s, int x, int y),
        blit(const Sprite *s, int x, int y, float opacity),
        blit(const Sprite *s, int x, int y, int x_src, int y_src, int w, int h),
        blit(const Sprite *s, int x, int y, int x_src, int y_src, int w, int h, float opacity),
        blit_a(const Sprite *s, float x, float y),
        blit_a(const Sprite *s, float x, float y, float opacity);

        void  // Draw sprite and modify alpha accordingly
        alphablit(const Sprite *s, float x, float y, float opacity = 1.0f);

        void  // Zoom/rotate another sprite
        stretch(const Sprite *s, int x, int y, int w, int h),
        stretch(const Sprite *s, int x, int y, int w, int h, int x_src, int y_src, int w_src, int h_src),
        rotate(const Sprite *s, int x, int y, float angle, float zoom = 1.0f);
        // Scaling uses nearest-neighbor interpolation

        void  // Blend a sprite with current target
        blend(BlendMode blend_mode, const Sprite *s, int x, int y),
        blend(BlendMode blend_mode, const Sprite *s, int x, int y, float strength);

        Color // Get color value at a certain location
        getpixel(int x, int y) const,
        getpixel_a(float x, float y) const,
        get_average(void) const,
        get_random(void) const;
        
        void  // Draw a single pixel
        putpixel(int x, int y, Color c),
        putpixel(int x, int y, Color c, float opacity),
        putpixel_a(float x, float y, Color c);

        void  // Blend in a single pixel using given blend function
        blendpixel(Color (*blend_function)(Color, Color), int x, int y, Color c);
        // Use one of: gfx::blend::avg, gfx::blend::add, gfx::blend::burn

        void  // Draw circles
        drawcircle(int x, int y, int radius, Color c),
        drawcircle_a(float x, float y, float radius, Color c);

        void  // Draw lines
        drawline(int x0, int y0, int x1, int y1, Color c),
        drawline(int x0, int y0, int x1, int y1, Color c, float opacity),
        drawline_a(float x0, float y0, float x1, float y1, Color c),
        drawline_a(float x0, float y0, float x1, float y1, Color c, float opacity);

        void  // Draw rectangles
        drawrect(int x0, int y0, int x1, int y1, Color c),
        drawrect_a(float x0, float y0, float x1, float y1, Color c);

        void  // Draw filled rectangles
        fillcircle(int x, int y, int radius, Color c),
        fillcircle(int x, int y, int radius, Color c, float opacity),
        fillcircle_a(float x, float y, float radius, Color c);

        void  // Draw filled circles
        fillrect(int x0, int y0, int x1, int y1, Color c),
        fillrect(int x0, int y0, int x1, int y1, Color c, float opacity),
        fillrect_a(float x0, float y0, float x1, float y1, Color c);

        void  // Flood fill at given position
        floodfill(int x, int y, Color c);

        void // Centered, zoomable grid
        drawgrid(
            int center_x, int center_y,   // grid center coordinates
            float pan_x, float pan_y,     // panning offset
            int spacing, float zoom,      // grid line spacing and zoom level
            Color color,                  // line color
            float opacity = 1.0f,         // line opacity
            bool horizontal_lines = true, // draw horizontal grid lines?
            bool vertical_lines = true);  // draw vertical grid lines?

        void // Ring/pie divided into clockwise-running sectors
        drawring(
            int value,                // number of highlighteds sectors
            int sectors,              // total number of sectors
            int x, int y,             // center coordinates
            float radius,             // outer radius (= ring size)
            float thickness,          // ring thickness
            float gap,                // sector gap (0 = none ... 1 = full ring)
            Color less_than_value,    // color for the highlighted sectors
            Color greater_than_value, // color for the rest
            Color background,         // ring background color
            bool round_corners);      // round sector corners or not?
        
        bool
        save(const char *filename) const;

        bool
        load(const char *filename);

    protected:
        int mutable_w, mutable_h;
    };

    // Color blending functions:
    namespace blend
    {
        Color bw(Color c);
        Color avg(Color c1, Color c2);
        Color add(Color c1, Color c2);
        Color burn(Color c1, Color c2);
        Color ratio(Color c1, Color c2, float c2_weight);
    }

    // Color format conversions:
    Color RGB_color(Component red, Component green, Component blue);
    Color HSL_color(float hue, float saturation, float lightness);
    void  HSL_to_RGB(float hue, float saturation, float lightness, Color *color);
    void  RGB_to_HSL(Color color, float *hue, float *saturation, float *lightness);
    
    void
    HSL_to_RGB(
        float hue, float saturation, float lightness,
        Component *red,
        Component *green,
        Component *blue);

    void
    RGB_to_HSL(
        Component red,
        Component green,
        Component blue,
        float *hue, float *saturation, float *lightness);
}

#endif
