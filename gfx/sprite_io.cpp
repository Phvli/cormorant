#include <SDL2/SDL_image.h>

bool
Sprite::load(const char *filename)
{
    SDL_Surface *surface = IMG_Load(filename);
    if (surface == NULL)
    {
        return false;
    }
    
    SDL_LockSurface(surface);
    SDL_PixelFormat *format = surface->format;

    delete[] this->data;
    this->data = NULL;
    this->resize(surface->w, surface->h);

    Color *dst = this->data;

    // Full alpha when alpha channel was not read (JPG and the like)
    Color alpha_mask = 0xff000000 * (!format->Amask);

    for (int y = 0; y < this->h; ++y)
    {
        Uint8  *src = (Uint8 *)surface->pixels + y * surface->pitch;
        for (int x = this->w - 1; x >= 0; --x)
        {
            Uint32 temp = 0x00000000;
            switch (format->BytesPerPixel)
            {
                case 1:
                    temp = *src;
                    break;

                case 2:
                    temp = *(Uint16 *)src;
                    break;

                case 3:
                    temp = (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                        ? (src[0] << 16 | src[1] << 8 | src[2])
                        : (src[0]       | src[1] << 8 | src[2] << 16);
                    break;

                case 4:
                    temp = *(Uint32 *)src;
                    break;

                default:;
            }
            SDL_Color color;
            SDL_GetRGBA(temp, surface->format, &color.r, &color.g, &color.b, &color.a);

            *dst = alpha_mask
#           if SDL_BYTEORDER == SDL_BIG_ENDIAN
                | (color.a << 24)  // alpha
                | (color.b << 16)  // red
                | (color.g << 8 )  // green
                | (color.r << 0 ); // blue
#           else
                | (color.a << 24)  // alpha
                | (color.r << 16)  // red
                | (color.g << 8 )  // green
                | (color.b << 0 ); // blue
#           endif
            
            src += surface->format->BytesPerPixel;
            dst++;
        }
    }
    SDL_UnlockSurface(surface);
    SDL_FreeSurface(surface);

    return true;
}

bool
Sprite::save(const char *filename)
const
{
    SDL_Surface *surface = SDL_CreateRGBSurface(0, this->w, this->h,
        32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

    if (surface == NULL)
    {
        return false;
    }

    Color *src = this->data;
    for (int y = 0; y < this->h; ++y)
    {
        unsigned char *dst
            = (unsigned char *)surface->pixels + y * surface->pitch;
        
        for (int x = this->w - 1; x >= 0; --x)
        {
#           if SDL_BYTEORDER == SDL_BIG_ENDIAN
                dst[0] = 0xff;
                dst[1] = (*src      ) & 0xff;
                dst[2] = (*src >> 8 ) & 0xff;
                dst[3] = (*src >> 16) & 0xff;
#           else
                dst[0] = (*src >> 16) & 0xff;
                dst[1] = (*src >> 8 ) & 0xff;
                dst[2] = (*src      ) & 0xff;
                dst[3] = 0xff;
#           endif
            
            dst += surface->format->BytesPerPixel;
            src++;
        }
    }
    
    bool success = (SDL_SaveBMP(surface, filename) == 0);
    SDL_FreeSurface(surface);
    
    return success;
}
