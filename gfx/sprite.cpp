#include "sprite.h"

#include <cstdlib>
#include <cstring>
#include <cmath>

using namespace gfx;

/*
	This is a long file, but the intention is to keep basic gfx stuff
	in a single file. This is supposed to be fire-and-forget stuff.

    25    VIDEO           Video mode handling
    203   SPRITES SYSTEM  Sprite alloc/dealloc/conversions
    416   SPRITE DRAWING  Sprite blitting
    1380  SHAPE DRAWING   Shape drawing
          PIXEL I/O       Single pixel put and get variants
          COLORS          Color blending & format conversions

*/

/*
________________________________________________________________________________

	V I D E O
________________________________________________________________________________

*/

static float frac(float value)  { return       value - (int)value; }
static float rfrac(float value) { return 1.0 - value + (int)value; }
static void
_putpixel(Component *p, Color c, float opacity)
{
	float f = 1.0f - opacity;
	*p = *p * f + (Component)c * opacity; p++;
	*p = *p * f + (Component)(c >>  8) * opacity; p++;
	*p = *p * f + (Component)(c >> 16) * opacity;
}

Sprite::~Sprite()
{
	delete[] this->data;
}

Sprite::Sprite():
	w(mutable_w),
	h(mutable_h)
{
	this->mutable_w        = 0;
	this->mutable_h        = 0;
	this->data             = NULL;

	this->clipping.x       = 0;
	this->clipping.y       = 0;
	this->clipping.w       = 0;
	this->clipping.h       = 0;
	
	this->clipping.enabled = true;
	this->alpha_blending   = Sprite::IGNORE_ALPHA;
}

Sprite::Sprite(int w, int h):
	w(mutable_w),
	h(mutable_h)
{
	this->mutable_w        = w;
	this->mutable_h        = h;
	this->data             = new Color[w * h];

	this->clipping.x       = 0;
	this->clipping.y       = 0;
	this->clipping.w       = w;
	this->clipping.h       = h;
	
	this->clipping.enabled = true;
	this->alpha_blending   = Sprite::IGNORE_ALPHA;
}

Sprite::Sprite(const Sprite *copy):
	w(mutable_w),
	h(mutable_h)
{
	this->mutable_w        = copy->w;
	this->mutable_h        = copy->h;
	this->data             = new Color[copy->w * copy->h];

	this->clipping.x       = copy->clipping.x;
	this->clipping.y       = copy->clipping.y;
	this->clipping.w       = copy->clipping.w;
	this->clipping.h       = copy->clipping.h;
	
	this->clipping.enabled = copy->clipping.enabled;
	this->alpha_blending   = copy->alpha_blending;

	memcpy(this->data, copy->data, this->w * this->h * Sprite::BYTES_PER_PIXEL);
}

Sprite::Sprite(const char *filename):
	w(mutable_w),
	h(mutable_h)
{
	this->data = NULL;
	this->load(filename);
}

void
Sprite::resize(int w, int h)
{
	delete[] this->data;

	this->data       = new Color[w * h];
	this->mutable_w  = w;
	this->mutable_h  = h;

	this->clipping.x = 0;
	this->clipping.y = 0;
	this->clipping.w = w;
	this->clipping.h = h;
}

void
Sprite::clear(Color color)
{
	Color *data = this->data + this->w * this->h - 1;
	
	if (this->alpha_blending)
	{
		color &= 0x00ffffff;
		for (; data >= this->data; --data)
		{
			*data &= 0xff000000;
			*data |= color;
		}
	}
	else
	{
		for (; data >= this->data; --data)
		{
			*data = color;
		}
	}
}

static float
_interpolate_cubic(float *f4, float x)
{
	/* f[0] . . . f[1] . x . f[2] . . . f[3] */

	float d0 = f4[0] - f4[1];
	float d2 = f4[2] - f4[1];
	float d3 = f4[3] - f4[1];
	float a0 = f4[1];
	float a1 = -1.0f / 3.0f * d0 + d2 -1.0f / 6.0f * d3;
	float a2 = .5f * d0 + .5f * d2;
	float a3 = -1.0f / 6.0f * d0 - .5f * d2 + 1.0f / 6.0f * d3;
	
	return a0 + a1 * x + a2 * x * x + a3 * x * x * x;
}

static float
_interpolate_bicubic(float *f16, float x, float y)
{
	float f_row[4];
	f_row[0] = _interpolate_cubic(f16, x);
	f_row[1] = _interpolate_cubic(f16 + 4, x);
	f_row[2] = _interpolate_cubic(f16 + 8, x);
	f_row[3] = _interpolate_cubic(f16 + 12, x);
	return _interpolate_cubic(f_row, y);
}

Sprite *
Sprite::scale(float zoom)
const
{
	return this->scale(this->w * zoom, this->h * zoom);
}

Sprite *
Sprite::scale(float w, float h)
const
{
	return (w > this->w || h > this->h)
		? this->scale_bicubic(w, h)
		: this->scale_bilinear(w, h);
}

Sprite *
Sprite::scale_nearest(float w, float h)
const
{
	int result_w, result_h;
	
	result_w = w; if (w > result_w) result_w++;
	result_h = h; if (h > result_h) result_h++;
	
	if (result_w == 0 || result_h == 0)
		return NULL;

	Sprite *result = new Sprite(result_w, result_h);
	
	int x, y;
	
	Color *dst = result->data;
	for (y = 0; y < result_h; ++y)
	{
		for (x = 0; x < result_w; ++x)
		{
			*dst = *(this->data +
				(int)((x * this->w + .5f) / w) +
				(int)((y * this->h + .5f) / h) * this->w);
			dst++;
		}
	}
	
	result->alpha_blending = this->alpha_blending;
	return result;
}

Sprite *
Sprite::scale_bilinear(float w, float h)
const
{
	int result_w, result_h;
	
	result_w = w; if (w > result_w) result_w++;
	result_h = h; if (h > result_h) result_h++;
	
	if (result_w == 0 || result_h == 0)
		return NULL;

	Sprite *result = new Sprite(result_w, result_h);
	
	Component r, g, b, a;
	Component *src, *src_base;
	Color *dst = result->data;
	int x, y, x_int, y_int;
	float f, x_f, y_f;
	
	result_w--; result_h--;
	for (y = 0; y < result_h; ++y)
	{
		y_int = (int)(y_f = (float)(y * this->h) / h);
		y_f -= y_int;
		src_base = (Component *)(this->data + y_int * this->w);
		for (x = 0; x < result_w; ++x)
		{
			x_int = (int)(x_f = (float)(x * this->w) / w);
			x_f -= x_int;
			
			/* (+0, +0) */
			f = (1.0f - x_f) * (1.0f - y_f);
			src = src_base + x_int * Sprite::BYTES_PER_PIXEL;
			a = src[3] * f; r = src[2] * f; g = src[1] * f; b = src[0] * f;
			
			/* (+1, +0) */
			f = x_f * (1.0f - y_f);
			src += Sprite::BYTES_PER_PIXEL;
			a += src[3] * f; r += src[2] * f; g += src[1] * f; b += src[0] * f;
			
			/* (+1, +1) */
			f = x_f * y_f;
			src += this->w * Sprite::BYTES_PER_PIXEL;
			a += src[3] * f; r += src[2] * f; g += src[1] * f; b += src[0] * f;
			
			/* (+0, +1) */
			f = (1.0f - x_f) * y_f;
			src -= Sprite::BYTES_PER_PIXEL;
			a += src[3] * f; r += src[2] * f; g += src[1] * f; b += src[0] * f;
			
			*dst = (a << 24) | (r << 16) | (g << 8) | b;
			dst++;
		}
		
		/* last pixel of the row */
		x_int = (int)(x_f = (float)(x * this->w) / w);
		x_f -= x_int;
		
		/* (+0, +0) */
		f = 1.0f - y_f;
		src = src_base + x_int * Sprite::BYTES_PER_PIXEL;
		a = src[3] * f; r = src[2] * f; g = src[1] * f; b = src[0] * f;
		
		/* (+0, +1) */
		f = y_f;
		src += this->w * Sprite::BYTES_PER_PIXEL;
		a += src[3] * f; r += src[2] * f; g += src[1] * f; b += src[0] * f;
		
		*dst = (a << 24) | (r << 16) | (g << 8) | b;
		dst++;
	}
	
	/* final row */
	y_int = (int)((float)(y * this->h) / h);
	src_base = (Component *)(this->data + y_int * this->w);
	
	for (x = 0; x < result_w; ++x)
	{
		x_int = (int)(x_f = (x * this->w + .5f) / w);
		x_f -= x_int;
		
		/* (+0, +0) */
		f = (1.0f - x_f);
		src = (Component *)(this->data + x_int + y_int * this->w);
		a = src[3] * f; r = src[2] * f; g = src[1] * f; b = src[0] * f;
		
		/* (+1, +0) */
		f = x_f;
		src += Sprite::BYTES_PER_PIXEL;
		a += src[3] * f; r += src[2] * f; g += src[1] * f; b += src[0] * f;
		
		*dst = (a << 24) | (r << 16) | (g << 8) | b;
		dst++;
	}
	
	/* bottom right corner */
	*dst = *(this->data + this->w * this->h - 1);
	
	
	result->alpha_blending = (w == this->w || h == this->h)
		? this->alpha_blending
		: Sprite::FULL_ALPHA;
	
	return result;
}

Sprite *
Sprite::scale_bicubic(float w, float h)
const
{
	int result_w, result_h;
	
	result_w = w; if (w > result_w) result_w++;
	result_h = h; if (h > result_h) result_h++;
	
	if (result_w == 0 || result_h == 0)
		return NULL;

	Sprite *result = new Sprite(result_w, result_h);
	
	int channel;
	
	float x_f, y_f;
	int x_int, y_int;
	
	int x, y, i;
	Color src[16];
	int c;
	float f[16];
	
	Component *dst = (Component *)result->data;
	for (y = 0; y < result_h; ++y)
	{
		y_int = (int)(y_f = (float)(y * this->h) / h);
		y_f -= y_int;
		for (x = 0; x < result_w; ++x)
		{
			x_int = (int)(x_f = (float)(x * this->w) / w);
			x_f -= x_int;
			src[0]  = this->getpixel(x_int - 1, y_int - 1);
			src[1]  = this->getpixel(x_int - 0, y_int - 1);
			src[2]  = this->getpixel(x_int + 1, y_int - 1);
			src[3]  = this->getpixel(x_int + 2, y_int - 1);
			src[4]  = this->getpixel(x_int - 1, y_int - 0);
			src[5]  = this->getpixel(x_int - 0, y_int - 0);
			src[6]  = this->getpixel(x_int + 1, y_int - 0);
			src[7]  = this->getpixel(x_int + 2, y_int - 0);
			src[8]  = this->getpixel(x_int - 1, y_int + 1);
			src[9]  = this->getpixel(x_int - 0, y_int + 1);
			src[10] = this->getpixel(x_int + 1, y_int + 1);
			src[11] = this->getpixel(x_int + 2, y_int + 1);
			src[12] = this->getpixel(x_int - 1, y_int + 2);
			src[13] = this->getpixel(x_int - 0, y_int + 2);
			src[14] = this->getpixel(x_int + 1, y_int + 2);
			src[15] = this->getpixel(x_int + 2, y_int + 2);
			for (channel = 0; channel < 32; channel += 8)
			{
				for (i = 0; i < 16; ++i)
					f[i] = (src[i] >> channel) & 0xff;
				
				c = _interpolate_bicubic(f, x_f, y_f);
				
				*dst = (c < 0)    ? 0x00
				     : (c > 0xff) ? 0xff
				                  : c;
				
				dst++;
			}
		}
	}
	
	result->alpha_blending = (w == this->w || h == this->h)
		? this->alpha_blending
		: Sprite::FULL_ALPHA;
	
	return result;
}

void
Sprite::colorize(Color color, Color mask)
{
	Color *dst;
	
	color &=  mask;
	mask   = ~mask;
	
	for (dst = this->data + this->w * this->h - 1; dst >= this->data; --dst)
	{
		*dst = (*dst & mask) | color;
	}
}

void
Sprite::make_transparent(Color c)
{
	this->make_transparent(c, 0.0f);
}

void
Sprite::make_transparent(Color c, float opacity)
{
	Color *p, alpha_color;
	Sprite::AlphaMode alpha_blending;
	c &= 0x00ffffff;
	
	if (opacity >= 1.0f)
	{
		return;
	}
	else if (opacity <= 0.0f)
	{
		alpha_blending = Sprite::BINARY_ALPHA;
		alpha_color = c;
	}
	else /* between 0.0 and 1.0 */
	{
		alpha_blending = Sprite::FULL_ALPHA;
		alpha_color = ((Color)(0xff * opacity) << 24) | c;
	}
	
	/* set alpha for matching pixels */
	bool alpha_found = false;
	for (p = this->data + this->w * this->h - 1; p >= this->data; --p)
	{
		if ((*p & 0x00ffffff) == c)
		{
			alpha_found = true;
			*p = alpha_color;
		}
	}
	
	/* update sprite blend mode */
	if (alpha_found && this->alpha_blending < alpha_blending)
	{
		this->alpha_blending = alpha_blending;
	}
}

void
Sprite::multiply_alpha(float opacity)
{
	Component *c;
	
	for (c = (Component *)(this->data + this->w * this->h) - 1;
		c >= (Component *)this->data; c -= 4)
	{
		*c *= opacity;
	}
	
	this->alpha_blending = Sprite::FULL_ALPHA;
}

void
Sprite::channel_to_alpha(Color channel_mask)
{
	int shift;
	Color *p;
	
	if ((channel_mask & 0x00ffffff) == 0)
	{
		return;
	}
	
	for (shift = 0; shift < 24 && ((channel_mask >> shift) & 0xff) == 0;
		shift += 8);
	
	shift = 24 - shift;
	
	for (p = this->data + this->w * this->h - 1; p >= this->data; --p)
	{
		*((Component *)p + 3) = ((*p) >> shift) & 0xff;
	}
	
	this->alpha_blending = Sprite::FULL_ALPHA;
}

void
Sprite::fill(Color c)
{
	Color *data;
	
	if (this->alpha_blending)
	{
		c &= 0x00ffffff;
		for (data = this->data + this->w * this->h - 1; data >= this->data; --data)
		{
			*data &= 0xff000000;
			*data |= c;
		}
	}
	else
	{
		for (data = this->data + this->w * this->h - 1; data >= this->data; --data)
		{
			*data = c;
		}
	}
}

void
Sprite::fill(Color c, float opacity)
{
	float f = 1.0f - opacity;
	Component red   = ((c >> 16)       ) * opacity;
	Component green = ((c >> 8 ) & 0xff) * opacity;
	Component blue  = ((c      ) & 0xff) * opacity;
	Component *p;
	
	p = (Component *)(this->data + this->w * this->h);
	do
	{
		p--;
		p--; *p = red   + f * *p;
		p--; *p = green + f * *p;
		p--; *p = blue  + f * *p;
	}
	while (p > (Component *)this->data);
}

/*
________________________________________________________________________________

	P I X E L   I / O
________________________________________________________________________________

*/

void
Sprite::putpixel(int x, int y, Color color)
{
#ifdef DEBUG
	if (x < 0) { x = 0; color = 0xffff0000; }
	else if (x >= this->w) { x = this->w - 1; color = 0xffff0000; }
	if (y < 0) { y = 0; color = 0xffff0000; }
	else if (y >= this->h) { y = this->h - 1; color = 0xffff0000; }
#endif
	this->data[y * this->w + x] = color;
}

void
Sprite::putpixel_a(float x, float y, Color c)
{
	Component *p, red, green, blue;
	int x_int, y_int;
	float f_src, f_dst, left_x, top_y;
	
#ifdef DEBUG
	if (x < 0) { x = 0; c = 0xffff0000; }
	else if (x > this->w - 1) { x = this->w - 1.0001f; c = 0xffff0000; }
	if (y < 0) { y = 0; c = 0xffff0000; }
	else if (y > this->h - 1) { y = this->h - 1.0001f; c = 0xffff0000; }
#endif
	
	left_x = 1.0f - (x -= (x_int = x));
	top_y  = 1.0f - (y -= (y_int = y));
	
	red   = (c >> 16);
	green = (c >> 8 ) & 0xff;
	blue  = (c      ) & 0xff;
	
	/* top left pixel */
	p = (Component *)(this->data + y_int * this->w + x_int);
	f_dst = left_x * top_y;
	f_src = 1.0f - f_dst;
	
	*p = f_dst * blue  + f_src * *p; p++;
	*p = f_dst * green + f_src * *p; p++;
	*p = f_dst * red   + f_src * *p;
	
	/* top right pixel (step right) */
	p += Sprite::BYTES_PER_PIXEL - 2;
	f_dst = x * top_y;
	f_src = 1.0f - f_dst;
	
	*p = f_dst * blue  + f_src * *p; p++;
	*p = f_dst * green + f_src * *p; p++;
	*p = f_dst * red   + f_src * *p;
	
	/* bottom left pixel (step down & left) */
	p += this->w * Sprite::BYTES_PER_PIXEL - Sprite::BYTES_PER_PIXEL - 2;
	f_dst = left_x * y;
	f_src = 1.0f - f_dst;
	
	*p = f_dst * blue  + f_src * *p; p++;
	*p = f_dst * green + f_src * *p; p++;
	*p = f_dst * red   + f_src * *p;
	
	/* bottom right pixel (step right) */
	p += Sprite::BYTES_PER_PIXEL - 2;
	f_dst = x * y;
	f_src = 1.0f - f_dst;
	
	*p = f_dst * blue  + f_src * *p; p++;
	*p = f_dst * green + f_src * *p; p++;
	*p = f_dst * red   + f_src * *p;
}

void
Sprite::putpixel(int x, int y, Color c, float opacity)
{
	Component *p;
	float f;

#ifdef DEBUG
	if (x < 0) { x = 0; c = 0xffff0000; opacity = 1.0f; }
	else if (x >= this->w) { x = this->w - 1; c = 0xffff0000; opacity = 1.0f; }
	if (y < 0) { y = 0; c = 0xffff0000; opacity = 1.0f; }
	else if (y >= this->h) { y = this->h - 1; c = 0xffff0000; opacity = 1.0f; }
#endif
	
	p = (Component *)(this->data + x + y * this->w);
	f = 1.0f - opacity;
	
	/* blue */
	*p = *p * f + (Component)c * opacity;
	p++;
	
	/* green */
	*p = *p * f + (Component)(c >>  8) * opacity;
	p++;
	
	/* red */
	*p = *p * f + (Component)(c >> 16) * opacity;
}

Color
Sprite::getpixel(int x, int y)
const
{
#ifdef DEBUG
	if (x < 0 || y < 0 || x >= this->w || y >= this->h)
		return 0xff0000;
#endif
	return this->data[y * this->w + x];
}

Color
Sprite::getpixel_a(float x, float y)
const
{
	Component *p, red, green, blue;
	int x_int, y_int;
	float f, left_x, top_y;

#ifdef DEBUG
	if (x < 0 || y < 0 || x >= this->w - 1 || y >= this->h - 1)
		return 0xff0000;
#endif
	
	left_x = 1.0f - (x -= (x_int = x));
	top_y  = 1.0f - (y -= (y_int = y));
	
	/* top left pixel */
	p = (Component *)(this->data + y_int * this->w + x_int);
	f = left_x * top_y;
	
	blue  = *p * f; p++;
	green = *p * f; p++;
	red   = *p * f;
	
	/* top right pixel (step right) */
	p += Sprite::BYTES_PER_PIXEL - 2;
	f = x * top_y;
	
	blue  += *p * f; p++;
	green += *p * f; p++;
	red   += *p * f;
	
	/* bottom left pixel (step down & left) */
	p += this->w * Sprite::BYTES_PER_PIXEL - Sprite::BYTES_PER_PIXEL - 2;
	f = left_x * y;
	
	blue  += *p * f; p++;
	green += *p * f; p++;
	red   += *p * f;
	
	/* bottom right pixel (step right) */
	p += Sprite::BYTES_PER_PIXEL - 2;
	f = x * y;
	
	blue  += *p * f; p++;
	green += *p * f; p++;
	red   += *p * f;
	
	return (red << 16) | (green << 8) | blue;
}

Color
Sprite::get_average(void)
const
{
	unsigned int
		r = 0x00,
		g = 0x00,
		b = 0x00,
		a = 0x00;
	
	const Component
		*src = (Component *)this->data;
	
	for (int i = this->w * this->h; i > 0; --i)
	{
		b += *(src++);
		g += *(src++);
		r += *(src++);
		a += *(src++);
	}
	
	float
		f = 1.0f / (float)(this->w * this->h);
	
	return (((Color)(f * a) & 0xff) << 24)
		|  (((Color)(f * r) & 0xff) << 16)
		|  (((Color)(f * g) & 0xff) << 8)
		|  (((Color)(f * b) & 0xff));
}

Color
Sprite::get_random(void)
const
{
	return this->data[rand() % (this->w * this->h)];
}

void
Sprite::blendpixel(
	Color (*blend_function)(Color, Color),
	int x, int y, Color c)
{
	Color *p;
	Component *c2;
	
#ifdef DEBUG
	if (x < 0) { x = 0; c = 0xffff0000; }
	if (x >= this->w) { x = this->w - 1; c = 0xffff0000; }
	if (y < 0) { y = 0; c = 0xffff0000; }
	if (y >= this->h) { y = this->h - 1; c = 0xffff0000; }
#endif

	p  = this->data + y * this->w + x;
	c2 = (Component *)p;
	*p = blend_function(c, c2[0] | c2[1] << 8 | c2[2] << 16);
}

Color
gfx::blend::bw(Color color)
{
	Color c = ((color & 0xff)
		+ ((color >> 8) & 0xff)
		+ ((color >> 16) & 0xff)) / 3;

	return c | c << 8 | c << 16 | color << 24;
}

Color
gfx::blend::avg(Color c1, Color c2)
{
	Color a1 = (c1 >> 24) & 0xff, a2 = (c2 >> 24) & 0xff;
	Color r1 = (c1 >> 16) & 0xff, r2 = (c2 >> 16) & 0xff;
	Color g1 = (c1 >>  8) & 0xff, g2 = (c2 >>  8) & 0xff;
	Color b1 =  c1        & 0xff, b2 =  c2        & 0xff;
	
	a2 = (a1 + a2) / 2;
	r2 = (r1 + r2) / 2;
	g2 = (g1 + g2) / 2;
	b2 = (b1 + b2) / 2;
	
	return a2 << 24 | r2 << 16 | g2 << 8 | b2;
}

Color
gfx::blend::add(Color c1, Color c2)
{
	Color r1 = (c1 >> 16) & 0xff, r2 = (c2 >> 16) & 0xff;
	Color g1 = (c1 >>  8) & 0xff, g2 = (c2 >>  8) & 0xff;
	Color b1 =  c1        & 0xff, b2 =  c2        & 0xff;
	
	if ((r2 += r1) > 0xff) r2 = 0xff;
	if ((g2 += g1) > 0xff) g2 = 0xff;
	if ((b2 += b1) > 0xff) b2 = 0xff;
	
	return 0xff000000 | r2 << 16 | g2 << 8 | b2;
}

Color
gfx::blend::burn(Color c1, Color c2)
{
	Color r1 = (c1 >> 16) & 0xff, r2 = (c2 >> 16) & 0xff;
	Color g1 = (c1 >>  8) & 0xff, g2 = (c2 >>  8) & 0xff;
	Color b1 =  c1        & 0xff, b2 =  c2        & 0xff;
	
	if (r1 < r2) r1 = 0x00; else r1 -= r2;
	if (g1 < g2) g1 = 0x00; else g1 -= g2;
	if (b1 < b2) b1 = 0x00; else b1 -= b2;
	
	return 0xff000000 | r1 << 16 | g1 << 8 | b1;
}

Color
gfx::blend::ratio(Color c1, Color c2, float c2_weight)
{
	float c1_weight = 1.0f - c2_weight;
	Color
		r1 = ((c1 >> 16) & 0xff) * c1_weight,
		r2 = ((c2 >> 16) & 0xff) * c2_weight,
		g1 = ((c1 >>  8) & 0xff) * c1_weight,
		g2 = ((c2 >>  8) & 0xff) * c2_weight,
		b1 = ( c1        & 0xff) * c1_weight,
		b2 = ( c2        & 0xff) * c2_weight;
	
	return 0xff000000 | (r1 + r2) << 16 | (g1 + g2) << 8 | (b1 + b2);
}

Color
gfx::RGB_color(Component red, Component green, Component blue)
{
	return 0xff000000 | (red << 16) | (green << 8) | blue;
}

Color
gfx::HSL_color(float hue, float saturation, float lightness)
{
	Component r, g, b;
	HSL_to_RGB(hue, saturation, lightness, &r, &g, &b);
	
	return RGB_color(r, g, b);
}

void
gfx::HSL_to_RGB(float hue, float saturation, float lightness, Color *color)
{
	*color = HSL_color(hue, saturation, lightness);
}

void
gfx::RGB_to_HSL(Color color, float *hue, float *saturation, float *lightness)
{
	Component
		r = (color >> 16) & 0xff,
		g = (color >> 8 ) & 0xff,
		b =  color        & 0xff;
	
	RGB_to_HSL(r, g, b, hue, saturation, lightness);
}

void
gfx::HSL_to_RGB(
	float hue, float saturation, float lightness,
	Component *red, Component *green, Component *blue)
{
	float h = hue, s = saturation, l = lightness;
	float r = 0.0f, g = 0.0f, b = 0.0f;
	float m, C, X, hh;
	
	if (red == NULL || green == NULL || blue == NULL) return;
	if (h < 0.0f) h = 0.0f; else if (h > 1.0f) h = 1.0f;
	if (s < 0.0f) s = 0.0f; else if (s > 1.0f) s = 1.0f;
	if (l < 0.0f) l = 0.0f; else if (l > 1.0f) l = 1.0f;
	C = (1.0f - fabs(l * 2.0f - 1.0f)) * s;
	hh = h * 6.0f;
	X = C * (1.0f - fabs(fmod(hh, 2.0f) - 1.0f));
	m = l - C / 2.0f;
	
	     if (hh < 1.0f) { r = C; g = X; }
	else if (hh < 2.0f) { r = X; g = C; }
	else if (hh < 3.0f) { g = C; b = X; }
	else if (hh < 4.0f) { g = X; b = C; }
	else if (hh < 5.0f) { r = X; b = C; }
	else                { r = C; b = X; }
	
	*red   = 0xff * (r + m);
	*green = 0xff * (g + m);
	*blue  = 0xff * (b + m);
}

void
gfx::RGB_to_HSL(
	Component red, Component green, Component blue,
	float *hue, float *saturation, float *lightness)
{
	float r = red / 255.0f, g = green / 255.0f, b = blue / 255.0f;
	float max, min, diff;
	
	if (r > g)
	{
		if (r > b) max = r;
		else       max = b;
		if (g < b) min = g;
		else       min = b;
	}
	else
	{
		if (g > b) max = g;
		else       max = b;
		if (r < b) min = r;
		else       min = b;
	}
	
	*lightness = (max + min) / 2.0f;
	
	diff = max - min;
	if (diff == 0)
	{
		*hue = 0;
		*saturation = 0;
		return;
	}
	
	*saturation = (*lightness > .5f) ? diff / (2.0f - max - min) : diff / (max + min);
	if (max == r)
		*hue = (g - b) / diff + ((g < b) ? 6.0f : 0.0f);
	else if (max == g)
		*hue = (b - r) / diff + 2.0f;
	else
		*hue = (r - g) / diff + 4.0f;

	*hue /= 6.0f;
}

/*
________________________________________________________________________________

	S P R I T E   D R A W I N G
________________________________________________________________________________

*/

void
Sprite::blit(const Sprite *s, int x, int y)
{
	this->blit(s, x, y, 0, 0, s->w, s->h);
}

void
Sprite::blit(const Sprite *s, int x, int y, int x_src, int y_src, int w, int h)
{
	Color *src, *dst;
	int line_w;
	float f_src, f_dst;
	Component *c_dst, *c_src;
	
	/* clip as necessary */
	if (x < 0) { w += x; x_src -= x; x = 0; }
	if (y < 0) { h += y; y_src -= y; y = 0; }
	
	if (x_src < 0) { w += x_src; x -= x_src; x_src = 0; }
	if (y_src < 0) { h += y_src; y -= y_src; y_src = 0; }
	
	if (x + w > this->w) w = this->w - x;
	if (y + h > this->h) h = this->h - y;
	
	if (x_src + w > s->w) w = s->w - x_src;
	if (y_src + h > s->h) h = s->h - y_src;
	
	if (w <= 0)
	{
		return;
	}
	
	/* set pointers to upper left corners */
	src = x_src + y_src * s->w        + s->data;
	dst = x     + y     * this->w + this->data;
	
	/* copy lines */
	switch (s->alpha_blending)
	{
		case Sprite::IGNORE_ALPHA:
			w *= Sprite::BYTES_PER_PIXEL;
			for (; h > 0; --h)
			{
				memcpy(dst, src, w);
				
				src += s->w;
				dst += this->w;
			}
			return;
			
		case Sprite::BINARY_ALPHA:
			for (; h > 0; --h)
			{
				for (line_w = w; line_w > 0; --line_w)
				{
					if (*src & 0xff000000)
					{
						*dst = *src;
					}
					src++;
					dst++;
				}
				
				src += s->w - w;
				dst += this->w - w;
			}
			return;
			
		case Sprite::FULL_ALPHA:
			c_src = (Component *)(src + w) - 1;
			c_dst = (Component *)(dst + w) - 2;
			for (; h > 0; --h)
			{
				for (line_w = w; line_w > 0; --line_w)
				{
					/* alpha */
					f_src = (float)(*c_src) / 0xff;
					f_dst = 1.0f - f_src;
					c_src--;
					
					/* red */
					*c_dst = f_src * *c_src + f_dst * *c_dst;
					c_src--; c_dst--;
					
					/* green */
					*c_dst = f_src * *c_src + f_dst * *c_dst;
					c_src--; c_dst--;
					
					/* blue */
					*c_dst = f_src * *c_src + f_dst * *c_dst;
					c_src--; c_dst -= 2;
				}
				
				c_src += (s->w + w) * Sprite::BYTES_PER_PIXEL;
				c_dst += (this->w + w) * Sprite::BYTES_PER_PIXEL;
			}
			return;
	}
}

void
Sprite::blit(const Sprite *s, int x, int y, float opacity)
{
	this->blit(s, x, y, 0, 0, s->w, s->h, opacity);
}

void
Sprite::blit(const Sprite *s, int x, int y, int x_src, int y_src, int w, int h, float opacity)
{
	/*
		basically an exact copy of clipsprite, with all sprites
		regarded as Sprite::FULL_ALPHA, and opacity multiplier added
		to the main loop
		
		sorry :'(
	*/
	Color *src, *dst;
	int line_w;
	float f_src, f_dst;
	Component *c_dst, *c_src;
	
	/* clip as necessary */
	if (x < 0) { w += x; x_src -= x; x = 0; }
	if (y < 0) { h += y; y_src -= y; y = 0; }
	
	if (x_src < 0) { w += x_src; x -= x_src; x_src = 0; }
	if (y_src < 0) { h += y_src; y -= y_src; y_src = 0; }
	
	if (x + w > this->w) w = this->w - x;
	if (y + h > this->h) h = this->h - y;
	
	if (x_src + w > s->w) w = s->w - x_src;
	if (y_src + h > s->h) h = s->h - y_src;
	
	if (w <= 0)
	{
		return;
	}
	
	/* set pointers to upper left corners */
	src = x_src + y_src * s->w        + s->data;
	dst = x     + y     * this->w + this->data;
	
	/* copy lines */
	c_src = (Component *)(src + w) - 1;
	c_dst = (Component *)(dst + w) - 2;
	opacity /= 0xff;
	for (; h > 0; --h)
	{
		for (line_w = w; line_w > 0; --line_w)
		{
			/* alpha */
			f_src = opacity * (*c_src);
			f_dst = 1.0f - f_src;
			c_src--;
			
			/* red */
			*c_dst = f_src * *c_src + f_dst * *c_dst;
			c_src--; c_dst--;
			
			/* green */
			*c_dst = f_src * *c_src + f_dst * *c_dst;
			c_src--; c_dst--;
			
			/* blue */
			*c_dst = f_src * *c_src + f_dst * *c_dst;
			c_src--; c_dst -= 2;
		}
		
		c_src += (s->w + w) * Sprite::BYTES_PER_PIXEL;
		c_dst += (this->w + w) * Sprite::BYTES_PER_PIXEL;
	}
}

void
Sprite::blit_a(const Sprite *s, float x, float y)
{
	int x_int, y_int, right_edge;
	int x_src, y_src;
	int w, h, line_w, line_step;
	
	Component *dst, *nw, *sw, *ne, *se;
	
	float f_dst;
	float f_nw, f_ne, f_sw, f_se;
	float f_nw_base, f_ne_base, f_sw_base, f_se_base;
	
	x_src = 0; w = s->w;
	y_src = 0; h = s->h;
	
	x -= (x_int = x);
	y -= (y_int = y);
	
	if (x < 0.0f) { x += 1.0f; x_int--; }
	if (y < 0.0f) { y += 1.0f; y_int--; }
	
	/* clip as necessary */
	if (x_int < -1) { w += x_int + 1; x_src -= x_int + 1; x_int = -1; }
	if (y_int < -1) { h += y_int + 1; y_src -= y_int + 1; y_int = -1; }
	
	if (x_int + w > this->w) w = this->w - x_int;
	if (y_int + h > this->h) h = this->h - y_int;
	if (w <= 0 || h <= 0) return;
	
	/* calculate source weigh ratios for each quadrant */
	f_nw_base = x * y;
	f_ne_base = (1.0f - x) * y;
	f_sw_base = x * (1.0f - y);
	f_se_base = (1.0f - x) * (1.0f - y);
	
	/* draw the clipped sprite */
	/* set pointers to upper right corners */
	right_edge = x_int + w;
	
	dst = (Component *)(this->data
		+ right_edge + y_int * this->w) + 2;
	nw  = (Component *)(s->data
		+ x_src + w + (y_src - 1) * s->w) - 1;
	
	right_edge = (right_edge < this->w);
	
	ne  = nw + Sprite::BYTES_PER_PIXEL;
	sw  = nw + s->w * Sprite::BYTES_PER_PIXEL;
	se  = ne + s->w * Sprite::BYTES_PER_PIXEL;
	
	line_step = (s->w + w + 1) * Sprite::BYTES_PER_PIXEL;
	
	if (y_int < 0)
	{
		/* skip top row if at or above upper boundary */
		dst += this->w * Sprite::BYTES_PER_PIXEL;
		nw += s->w * Sprite::BYTES_PER_PIXEL;
		ne += s->w * Sprite::BYTES_PER_PIXEL;
		sw += s->w * Sprite::BYTES_PER_PIXEL;
		se += s->w * Sprite::BYTES_PER_PIXEL;
		goto draw_sprite_lines;
	}
	
	/* TOP RIGHT CORNER: */
	
	nw -= Sprite::BYTES_PER_PIXEL; ne -= Sprite::BYTES_PER_PIXEL; se -= Sprite::BYTES_PER_PIXEL;
	if (right_edge)
	{
		f_dst = 1.0f - (f_sw = f_sw_base * *sw / (float)0xff);
		sw--;
		
		*dst = *dst * f_dst + f_sw * *sw; sw--; dst--;
		*dst = *dst * f_dst + f_sw * *sw; sw--; dst--;
		*dst = *dst * f_dst + f_sw * *sw; sw--; dst -= 2;
	}
	else
	{
		dst -= Sprite::BYTES_PER_PIXEL;
		sw  -= Sprite::BYTES_PER_PIXEL;
	}
	
	
	/* TOP EDGE: */
	
	for (line_w = w; line_w > 1; --line_w)
	{
		/* alpha */
		f_dst = 1.0f
			- (f_sw = f_sw_base * *sw / (float)0xff)
			- (f_se = f_se_base * *se / (float)0xff);
		sw--; se--;
		
		/* red */
		*dst = *dst * f_dst + f_sw * *sw + f_se * *se;
		sw--; se--; dst--;
		
		/* green */
		*dst = *dst * f_dst + f_sw * *sw + f_se * *se;
		sw--; se--; dst--;
		
		/* blue */
		*dst = *dst * f_dst + f_sw * *sw + f_se * *se;
		sw--; se--; dst -= 2;
		
		nw -= Sprite::BYTES_PER_PIXEL; ne -= Sprite::BYTES_PER_PIXEL;
	}
	
	
	/* TOP LEFT CORNER: */
	
	nw += line_step - Sprite::BYTES_PER_PIXEL;
	ne += line_step - Sprite::BYTES_PER_PIXEL;
	sw += line_step - Sprite::BYTES_PER_PIXEL;
	
	if (x_int < 0)
	{
		se  += line_step - Sprite::BYTES_PER_PIXEL;
		dst += (this->w + w) * Sprite::BYTES_PER_PIXEL;
	}
	else
	{
		f_dst = 1.0f - (f_se = f_se_base * *se / (float)0xff);
		se--;
		
		*dst = *dst * f_dst + f_se * *se; se--; dst--;
		*dst = *dst * f_dst + f_se * *se; se--; dst--;
		*dst = *dst * f_dst + f_se * *se;
		
		se += line_step - 1;
		dst += (this->w + w + 1) * Sprite::BYTES_PER_PIXEL - 2;
	}
	
	
draw_sprite_lines:
	for (h--; h > 0; --h)
	{
	/* RIGHT EDGE: */
		
		ne -= Sprite::BYTES_PER_PIXEL; se -= Sprite::BYTES_PER_PIXEL;
		if (right_edge)
		{
			/* alpha */
			f_dst = 1.0f
				- (f_nw = f_nw_base * *nw / (float)0xff)
				- (f_sw = f_sw_base * *sw / (float)0xff);
			nw--; sw--;
			
			/* red */
			*dst = *dst * f_dst + f_nw * *nw + f_sw * *sw;
			nw--; sw--; dst--;
			
			/* green */
			*dst = *dst * f_dst + f_nw * *nw + f_sw * *sw;
			nw--; sw--; dst--;
			
			/* blue */
			*dst = *dst * f_dst + f_nw * *nw + f_sw * *sw;
			nw--; sw--; dst -= 2;
		}
		else
		{
			dst -= Sprite::BYTES_PER_PIXEL;
			nw  -= Sprite::BYTES_PER_PIXEL;
			sw  -= Sprite::BYTES_PER_PIXEL;
		}
		
		
	/* LINE BETWEEN EDGES: */
		
		for (line_w = w; line_w > 1; --line_w)
		{
			/* alpha */
			f_dst = 1.0f
				- (f_nw = f_nw_base * *nw / (float)0xff)
				- (f_ne = f_ne_base * *ne / (float)0xff)
				- (f_sw = f_sw_base * *sw / (float)0xff)
				- (f_se = f_se_base * *se / (float)0xff);
			nw--; ne--;
			sw--; se--;
			
			/* red */
			*dst = *dst * f_dst
			     + *nw  * f_nw + *ne * f_ne
			     + *sw  * f_sw + *se * f_se;
			nw--; ne--;
			sw--; se--;
			dst--;
			
			/* green */
			*dst = *dst * f_dst
			     + *nw  * f_nw + *ne * f_ne
			     + *sw  * f_sw + *se * f_se;
			nw--; ne--;
			sw--; se--;
			dst--;
			
			/* blue */
			*dst = *dst * f_dst
			     + *nw  * f_nw + *ne * f_ne
			     + *sw  * f_sw + *se * f_se;
			nw--; ne--;
			sw--; se--;
			dst -= 2;
		}
		
		if (x_int < 0)
		{
			/* skip left edge if at or beyond the left boundary */
			nw += line_step - Sprite::BYTES_PER_PIXEL;
			ne += line_step - Sprite::BYTES_PER_PIXEL;
			sw += line_step - Sprite::BYTES_PER_PIXEL;
			se += line_step - Sprite::BYTES_PER_PIXEL;
			dst += (this->w + w) * Sprite::BYTES_PER_PIXEL;
			continue;
		}
		
	/* LEFT EDGE: */
		
		/* alpha */
		f_dst = 1.0f
			- (f_ne = f_ne_base * *ne / (float)0xff)
			- (f_se = f_se_base * *se / (float)0xff);
		ne--; se--;
		
		/* blue */
		*dst = *dst * f_dst + f_ne * *ne + f_se * *se;
		ne--; se--; dst--;
		
		/* green */
		*dst = *dst * f_dst + f_ne * *ne + f_se * *se;
		ne--; se--; dst--;
		
		/* blue */
		*dst = *dst * f_dst + f_ne * *ne + f_se * *se;
		ne--; se--; dst -= 2;
		
	/* NEXT LINE: */
		nw += line_step - Sprite::BYTES_PER_PIXEL;
		sw += line_step - Sprite::BYTES_PER_PIXEL;
		ne += line_step; se += line_step;
		dst += (this->w + w + 1) * Sprite::BYTES_PER_PIXEL;
	}
	
	if ((Color *)dst >= this->data + this->w * this->h)
	{
		/* skip bottom row if at or below lower boundary */
		return;
	}
	
	/* BOTTOM RIGHT CORNER: */
	
	ne -= Sprite::BYTES_PER_PIXEL; sw -= Sprite::BYTES_PER_PIXEL; se -= Sprite::BYTES_PER_PIXEL;
	if (right_edge)
	{
		f_dst = 1.0f - (f_nw = f_nw_base * *nw / (float)0xff);
		nw--;
		
		*dst = *dst * f_dst + f_nw * *nw; nw--; dst--;
		*dst = *dst * f_dst + f_nw * *nw; nw--; dst--;
		*dst = *dst * f_dst + f_nw * *nw; nw--; dst -= 2;
	}
	else
	{
		dst -= Sprite::BYTES_PER_PIXEL;
		nw  -= Sprite::BYTES_PER_PIXEL;
	}
	
	
	/* BOTTOM EDGE: */
	
	for (line_w = w; line_w > 1; --line_w)
	{
		/* alpha */
		f_dst = 1.0f
			- (f_nw = f_nw_base * *nw / (float)0xff)
			- (f_ne = f_ne_base * *ne / (float)0xff);
		nw--; ne--;
		
		/* red */
		*dst = *dst * f_dst + f_nw * *nw + f_ne * *ne;
		nw--; ne--; dst--;
		
		/* green */
		*dst = *dst * f_dst + f_nw * *nw + f_ne * *ne;
		nw--; ne--; dst--;
		
		/* blue */
		*dst = *dst * f_dst + f_nw * *nw + f_ne * *ne;
		nw--; ne--; dst -= 2;
		
		sw -= Sprite::BYTES_PER_PIXEL; se -= Sprite::BYTES_PER_PIXEL;
	}
	
	
	/* BOTTOM LEFT CORNER: */
	
	if (x_int < 0) return;
	
	f_dst = 1.0f - (f_ne = f_ne_base * *ne / (float)0xff);
	ne--;
	
	*dst = *dst * f_dst + f_ne * *ne; ne--; dst--;
	*dst = *dst * f_dst + f_ne * *ne; ne--; dst--;
	*dst = *dst * f_dst + f_ne * *ne;
}

void
Sprite::blit_a(const Sprite *s, float x, float y, float opacity)
{
	int x_int, y_int, right_edge;
	int x_src, y_src;
	int w, h, line_w, line_step;
	
	Component *dst, *nw, *sw, *ne, *se;
	
	float f_dst;
	float f_nw, f_ne, f_sw, f_se;
	float f_nw_base, f_ne_base, f_sw_base, f_se_base;
	
	x_src = 0; w = s->w;
	y_src = 0; h = s->h;
	
	x -= (x_int = x);
	y -= (y_int = y);
	
	if (x < 0.0f) { x += 1.0f; x_int--; }
	if (y < 0.0f) { y += 1.0f; y_int--; }
	
	/* clip as necessary */
	if (x_int < -1) { w += x_int + 1; x_src -= x_int + 1; x_int = -1; }
	if (y_int < -1) { h += y_int + 1; y_src -= y_int + 1; y_int = -1; }
	
	if (x_int + w > this->w) w = this->w - x_int;
	if (y_int + h > this->h) h = this->h - y_int;
	if (w <= 0 || h <= 0) return;
	
	/* calculate source weigh ratios for each quadrant */
	f_nw_base = x * y;
	f_ne_base = (1.0f - x) * y;
	f_sw_base = x * (1.0f - y);
	f_se_base = (1.0f - x) * (1.0f - y);
	
	/* draw the clipped sprite */
	/* set pointers to upper right corners */
	right_edge = x_int + w;
	
	dst = (Component *)(this->data
		+ right_edge + y_int * this->w) + 2;
	nw  = (Component *)(s->data
		+ x_src + w + (y_src - 1) * s->w) - 1;
	
	right_edge = (right_edge < this->w);
	
	ne  = nw + Sprite::BYTES_PER_PIXEL;
	sw  = nw + s->w * Sprite::BYTES_PER_PIXEL;
	se  = ne + s->w * Sprite::BYTES_PER_PIXEL;
	
	line_step = (s->w + w + 1) * Sprite::BYTES_PER_PIXEL;
	
	if (y_int < 0)
	{
		/* skip top row if at or above upper boundary */
		dst += this->w * Sprite::BYTES_PER_PIXEL;
		nw += s->w * Sprite::BYTES_PER_PIXEL;
		ne += s->w * Sprite::BYTES_PER_PIXEL;
		sw += s->w * Sprite::BYTES_PER_PIXEL;
		se += s->w * Sprite::BYTES_PER_PIXEL;
		goto draw_sprite_lines;
	}
	
	/* TOP RIGHT CORNER: */
	
	nw -= Sprite::BYTES_PER_PIXEL; ne -= Sprite::BYTES_PER_PIXEL; se -= Sprite::BYTES_PER_PIXEL;
	if (right_edge)
	{
		f_dst = 1.0f - (f_sw = f_sw_base * *sw / (float)0xff);
		sw--;
		
		*dst = *dst * f_dst + f_sw * *sw; sw--; dst--;
		*dst = *dst * f_dst + f_sw * *sw; sw--; dst--;
		*dst = *dst * f_dst + f_sw * *sw; sw--; dst -= 2;
	}
	else
	{
		dst -= Sprite::BYTES_PER_PIXEL;
		sw  -= Sprite::BYTES_PER_PIXEL;
	}
	
	
	/* TOP EDGE: */
	
	for (line_w = w; line_w > 1; --line_w)
	{
		/* alpha */
		f_dst = 1.0f - opacity * (
			  (f_sw = f_sw_base * *sw / (float)0xff)
			+ (f_se = f_se_base * *se / (float)0xff));
		sw--; se--;
		
		/* red */
		*dst = *dst * f_dst + f_sw * *sw + f_se * *se;
		sw--; se--; dst--;
		
		/* green */
		*dst = *dst * f_dst + f_sw * *sw + f_se * *se;
		sw--; se--; dst--;
		
		/* blue */
		*dst = *dst * f_dst + f_sw * *sw + f_se * *se;
		sw--; se--; dst -= 2;
		
		nw -= Sprite::BYTES_PER_PIXEL; ne -= Sprite::BYTES_PER_PIXEL;
	}
	
	
	/* TOP LEFT CORNER: */
	
	nw += line_step - Sprite::BYTES_PER_PIXEL;
	ne += line_step - Sprite::BYTES_PER_PIXEL;
	sw += line_step - Sprite::BYTES_PER_PIXEL;
	
	if (x_int < 0)
	{
		se  += line_step - Sprite::BYTES_PER_PIXEL;
		dst += (this->w + w) * Sprite::BYTES_PER_PIXEL;
	}
	else
	{
		f_dst = 1.0f - opacity * (f_se = f_se_base * *se / (float)0xff);
		se--;
		
		*dst = *dst * f_dst + f_se * *se; se--; dst--;
		*dst = *dst * f_dst + f_se * *se; se--; dst--;
		*dst = *dst * f_dst + f_se * *se;
		
		se += line_step - 1;
		dst += (this->w + w + 1) * Sprite::BYTES_PER_PIXEL - 2;
	}
	
	
draw_sprite_lines:
	for (h--; h > 0; --h)
	{
	/* RIGHT EDGE: */
		
		ne -= Sprite::BYTES_PER_PIXEL; se -= Sprite::BYTES_PER_PIXEL;
		if (right_edge)
		{
			/* alpha */
			f_dst = 1.0f - opacity * (
				  (f_nw = f_nw_base * *nw / (float)0xff)
				+ (f_sw = f_sw_base * *sw / (float)0xff));
			nw--; sw--;
			
			/* red */
			*dst = *dst * f_dst + f_nw * *nw + f_sw * *sw;
			nw--; sw--; dst--;
			
			/* green */
			*dst = *dst * f_dst + f_nw * *nw + f_sw * *sw;
			nw--; sw--; dst--;
			
			/* blue */
			*dst = *dst * f_dst + f_nw * *nw + f_sw * *sw;
			nw--; sw--; dst -= 2;
		}
		else
		{
			dst -= Sprite::BYTES_PER_PIXEL;
			nw  -= Sprite::BYTES_PER_PIXEL;
			sw  -= Sprite::BYTES_PER_PIXEL;
		}
		
		
	/* LINE BETWEEN EDGES: */
		
		for (line_w = w; line_w > 1; --line_w)
		{
			/* alpha */
			f_dst = 1.0f - opacity * (
				  (f_nw = f_nw_base * *nw / (float)0xff)
				+ (f_ne = f_ne_base * *ne / (float)0xff)
				+ (f_sw = f_sw_base * *sw / (float)0xff)
				+ (f_se = f_se_base * *se / (float)0xff));
			nw--; ne--;
			sw--; se--;
			
			/* red */
			*dst = *dst * f_dst
			     + *nw  * f_nw + *ne * f_ne
			     + *sw  * f_sw + *se * f_se;
			nw--; ne--;
			sw--; se--;
			dst--;
			
			/* green */
			*dst = *dst * f_dst
			     + *nw  * f_nw + *ne * f_ne
			     + *sw  * f_sw + *se * f_se;
			nw--; ne--;
			sw--; se--;
			dst--;
			
			/* blue */
			*dst = *dst * f_dst
			     + *nw  * f_nw + *ne * f_ne
			     + *sw  * f_sw + *se * f_se;
			nw--; ne--;
			sw--; se--;
			dst -= 2;
		}
		
		if (x_int < 0)
		{
			/* skip left edge if at or beyond the left boundary */
			nw += line_step - Sprite::BYTES_PER_PIXEL;
			ne += line_step - Sprite::BYTES_PER_PIXEL;
			sw += line_step - Sprite::BYTES_PER_PIXEL;
			se += line_step - Sprite::BYTES_PER_PIXEL;
			dst += (this->w + w) * Sprite::BYTES_PER_PIXEL;
			continue;
		}
		
	/* LEFT EDGE: */
		
		/* alpha */
		f_dst = 1.0f - opacity * (
			  (f_ne = f_ne_base * *ne / (float)0xff)
			+ (f_se = f_se_base * *se / (float)0xff));
		ne--; se--;
		
		/* blue */
		*dst = *dst * f_dst + f_ne * *ne + f_se * *se;
		ne--; se--; dst--;
		
		/* green */
		*dst = *dst * f_dst + f_ne * *ne + f_se * *se;
		ne--; se--; dst--;
		
		/* blue */
		*dst = *dst * f_dst + f_ne * *ne + f_se * *se;
		ne--; se--; dst -= 2;
		
	/* NEXT LINE: */
		nw += line_step - Sprite::BYTES_PER_PIXEL;
		sw += line_step - Sprite::BYTES_PER_PIXEL;
		ne += line_step; se += line_step;
		dst += (this->w + w + 1) * Sprite::BYTES_PER_PIXEL;
	}
	
	if ((Color *)dst >= this->data + this->w * this->h)
	{
		/* skip bottom row if at or below lower boundary */
		return;
	}
	
	/* BOTTOM RIGHT CORNER: */
	
	ne -= Sprite::BYTES_PER_PIXEL; sw -= Sprite::BYTES_PER_PIXEL; se -= Sprite::BYTES_PER_PIXEL;
	if (right_edge)
	{
		f_dst = 1.0f - opacity * (f_nw = f_nw_base * *nw / (float)0xff);
		nw--;
		
		*dst = *dst * f_dst + f_nw * *nw; nw--; dst--;
		*dst = *dst * f_dst + f_nw * *nw; nw--; dst--;
		*dst = *dst * f_dst + f_nw * *nw; nw--; dst -= 2;
	}
	else
	{
		dst -= Sprite::BYTES_PER_PIXEL;
		nw  -= Sprite::BYTES_PER_PIXEL;
	}
	
	
	/* BOTTOM EDGE: */
	
	for (line_w = w; line_w > 1; --line_w)
	{
		/* alpha */
		f_dst = 1.0f - opacity * (
			  (f_nw = f_nw_base * *nw / (float)0xff)
			- (f_ne = f_ne_base * *ne / (float)0xff));
		nw--; ne--;
		
		/* red */
		*dst = *dst * f_dst + f_nw * *nw + f_ne * *ne;
		nw--; ne--; dst--;
		
		/* green */
		*dst = *dst * f_dst + f_nw * *nw + f_ne * *ne;
		nw--; ne--; dst--;
		
		/* blue */
		*dst = *dst * f_dst + f_nw * *nw + f_ne * *ne;
		nw--; ne--; dst -= 2;
		
		sw -= Sprite::BYTES_PER_PIXEL; se -= Sprite::BYTES_PER_PIXEL;
	}
	
	
	/* BOTTOM LEFT CORNER: */
	
	if (x_int < 0) return;
	
	f_dst = 1.0f - opacity * (f_ne = f_ne_base * *ne / (float)0xff);
	ne--;
	
	*dst = *dst * f_dst + f_ne * *ne; ne--; dst--;
	*dst = *dst * f_dst + f_ne * *ne; ne--; dst--;
	*dst = *dst * f_dst + f_ne * *ne;
}

void
Sprite::alphablit(const Sprite *s, float x, float y, float opacity)
{
	int x_int, y_int, right_edge;
	int x_src, y_src;
	int w, h, line_w, line_step;
	
	Component *dst, *nw, *sw, *ne, *se;
	
	float f_dst;
	float f_nw, f_ne, f_sw, f_se;
	float f_nw_base, f_ne_base, f_sw_base, f_se_base;
	
	x_src = 0; w = s->w;
	y_src = 0; h = s->h;
	
	x -= (x_int = x);
	y -= (y_int = y);
	
	if (x < 0.0f) { x += 1.0f; x_int--; }
	if (y < 0.0f) { y += 1.0f; y_int--; }
	
	/* clip as necessary */
	if (x_int < -1) { w += x_int + 1; x_src -= x_int + 1; x_int = -1; }
	if (y_int < -1) { h += y_int + 1; y_src -= y_int + 1; y_int = -1; }
	
	if (x_int + w > this->w) w = this->w - x_int;
	if (y_int + h > this->h) h = this->h - y_int;
	if (w <= 0 || h <= 0) return;
	
	/* calculate source weigh ratios for each quadrant */
	f_nw_base = x * y;
	f_ne_base = (1.0f - x) * y;
	f_sw_base = x * (1.0f - y);
	f_se_base = (1.0f - x) * (1.0f - y);
	
	/* draw the clipped sprite */
	/* set pointers to upper right corners */
	right_edge = x_int + w;
	
	dst = (Component *)(this->data
		+ right_edge + y_int * this->w) + 3;
	nw  = (Component *)(s->data
		+ x_src + w + (y_src - 1) * s->w) - 1;
	
	right_edge = (right_edge < this->w);
	
	ne  = nw + Sprite::BYTES_PER_PIXEL;
	sw  = nw + s->w * Sprite::BYTES_PER_PIXEL;
	se  = ne + s->w * Sprite::BYTES_PER_PIXEL;
	
	line_step = (s->w + w + 1) * Sprite::BYTES_PER_PIXEL;
	
	if (y_int < 0)
	{
		/* skip top row if at or above upper boundary */
		dst += this->w * Sprite::BYTES_PER_PIXEL;
		nw += s->w * Sprite::BYTES_PER_PIXEL;
		ne += s->w * Sprite::BYTES_PER_PIXEL;
		sw += s->w * Sprite::BYTES_PER_PIXEL;
		se += s->w * Sprite::BYTES_PER_PIXEL;
		goto draw_sprite_lines;
	}
	
	/* TOP RIGHT CORNER: */
	
	nw -= Sprite::BYTES_PER_PIXEL; ne -= Sprite::BYTES_PER_PIXEL; se -= Sprite::BYTES_PER_PIXEL;
	if (right_edge)
	{
		f_dst = opacity * (f_sw = f_sw_base * *sw / (float)0xff);
		if (*dst < 0xff * f_dst) *dst = 0xff * f_dst;
		f_dst = 1.0f - f_dst;
		sw--; dst--;
		
		*dst = *dst * f_dst + f_sw * *sw; sw--; dst--;
		*dst = *dst * f_dst + f_sw * *sw; sw--; dst--;
		*dst = *dst * f_dst + f_sw * *sw; sw--; dst--;
	}
	else
	{
		dst -= Sprite::BYTES_PER_PIXEL;
		sw  -= Sprite::BYTES_PER_PIXEL;
	}
	
	
	/* TOP EDGE: */
	
	for (line_w = w; line_w > 1; --line_w)
	{
		/* alpha */
		f_dst = opacity * (
			  (f_sw = f_sw_base * *sw / (float)0xff)
			+ (f_se = f_se_base * *se / (float)0xff));
		if (*dst < 0xff * f_dst) *dst = 0xff * f_dst;
		f_dst = 1.0f - f_dst;
		sw--; se--; dst--;
		
		/* red */
		*dst = *dst * f_dst + f_sw * *sw + f_se * *se;
		sw--; se--; dst--;
		
		/* green */
		*dst = *dst * f_dst + f_sw * *sw + f_se * *se;
		sw--; se--; dst--;
		
		/* blue */
		*dst = *dst * f_dst + f_sw * *sw + f_se * *se;
		sw--; se--; dst--;
		
		nw -= Sprite::BYTES_PER_PIXEL; ne -= Sprite::BYTES_PER_PIXEL;
	}
	
	
	/* TOP LEFT CORNER: */
	
	nw += line_step - Sprite::BYTES_PER_PIXEL;
	ne += line_step - Sprite::BYTES_PER_PIXEL;
	sw += line_step - Sprite::BYTES_PER_PIXEL;
	
	if (x_int < 0)
	{
		se  += line_step - Sprite::BYTES_PER_PIXEL;
		dst += (this->w + w) * Sprite::BYTES_PER_PIXEL;
	}
	else
	{
		f_dst = opacity * (f_se = f_se_base * *se / (float)0xff);
		if (*dst < 0xff * f_dst) *dst = 0xff * f_dst;
		f_dst = 1.0f - f_dst;
		se--; dst--;
		
		*dst = *dst * f_dst + f_se * *se; se--; dst--;
		*dst = *dst * f_dst + f_se * *se; se--; dst--;
		*dst = *dst * f_dst + f_se * *se;
		
		se += line_step - 1;
		dst += (this->w + w + 1) * Sprite::BYTES_PER_PIXEL - 1;
	}
	
	
draw_sprite_lines:
	for (h--; h > 0; --h)
	{
	/* RIGHT EDGE: */
		
		ne -= Sprite::BYTES_PER_PIXEL; se -= Sprite::BYTES_PER_PIXEL;
		if (right_edge)
		{
			/* alpha */
			f_dst = opacity * (
				  (f_nw = f_nw_base * *nw / (float)0xff)
				+ (f_sw = f_sw_base * *sw / (float)0xff));
			if (*dst < 0xff * f_dst) *dst = 0xff * f_dst;
			f_dst = 1.0f - f_dst;
			nw--; sw--; dst--;
			
			/* red */
			*dst = *dst * f_dst + f_nw * *nw + f_sw * *sw;
			nw--; sw--; dst--;
			
			/* green */
			*dst = *dst * f_dst + f_nw * *nw + f_sw * *sw;
			nw--; sw--; dst--;
			
			/* blue */
			*dst = *dst * f_dst + f_nw * *nw + f_sw * *sw;
			nw--; sw--; dst--;
		}
		else
		{
			dst -= Sprite::BYTES_PER_PIXEL;
			nw  -= Sprite::BYTES_PER_PIXEL;
			sw  -= Sprite::BYTES_PER_PIXEL;
		}
		
		
	/* LINE BETWEEN EDGES: */
		
		for (line_w = w; line_w > 1; --line_w)
		{
			/* alpha */
			f_dst = opacity * (
				  (f_nw = f_nw_base * *nw / (float)0xff)
				+ (f_ne = f_ne_base * *ne / (float)0xff)
				+ (f_sw = f_sw_base * *sw / (float)0xff)
				+ (f_se = f_se_base * *se / (float)0xff));
			if (*dst < 0xff * f_dst) *dst = 0xff * f_dst;
			f_dst = 1.0f - f_dst;
			nw--; ne--;
			sw--; se--;
			dst--;
			
			/* red */
			*dst = *dst * f_dst
			     + *nw  * f_nw + *ne * f_ne
			     + *sw  * f_sw + *se * f_se;
			nw--; ne--;
			sw--; se--;
			dst--;
			
			/* green */
			*dst = *dst * f_dst
			     + *nw  * f_nw + *ne * f_ne
			     + *sw  * f_sw + *se * f_se;
			nw--; ne--;
			sw--; se--;
			dst--;
			
			/* blue */
			*dst = *dst * f_dst
			     + *nw  * f_nw + *ne * f_ne
			     + *sw  * f_sw + *se * f_se;
			nw--; ne--;
			sw--; se--;
			dst--;
		}
		
		if (x_int < 0)
		{
			/* skip left edge if at or beyond the left boundary */
			nw += line_step - Sprite::BYTES_PER_PIXEL;
			ne += line_step - Sprite::BYTES_PER_PIXEL;
			sw += line_step - Sprite::BYTES_PER_PIXEL;
			se += line_step - Sprite::BYTES_PER_PIXEL;
			dst += (this->w + w) * Sprite::BYTES_PER_PIXEL;
			continue;
		}
		
	/* LEFT EDGE: */
		
		/* alpha */
		f_dst = opacity * (
			  (f_ne = f_ne_base * *ne / (float)0xff)
			+ (f_se = f_se_base * *se / (float)0xff));
		if (*dst < 0xff * f_dst) *dst = 0xff * f_dst;
		f_dst = 1.0f - f_dst;
		ne--; se--; dst--;
		
		/* blue */
		*dst = *dst * f_dst + f_ne * *ne + f_se * *se;
		ne--; se--; dst--;
		
		/* green */
		*dst = *dst * f_dst + f_ne * *ne + f_se * *se;
		ne--; se--; dst--;
		
		/* blue */
		*dst = *dst * f_dst + f_ne * *ne + f_se * *se;
		ne--; se--; dst--;
		
	/* NEXT LINE: */
		nw += line_step - Sprite::BYTES_PER_PIXEL;
		sw += line_step - Sprite::BYTES_PER_PIXEL;
		ne += line_step; se += line_step;
		dst += (this->w + w + 1) * Sprite::BYTES_PER_PIXEL;
	}
	
	if ((Color *)dst >= this->data + this->w * this->h)
	{
		/* skip bottom row if at or below lower boundary */
		return;
	}
	
	/* BOTTOM RIGHT CORNER: */
	
	ne -= Sprite::BYTES_PER_PIXEL; sw -= Sprite::BYTES_PER_PIXEL; se -= Sprite::BYTES_PER_PIXEL;
	if (right_edge)
	{
		f_dst = opacity * (f_nw = f_nw_base * *nw / (float)0xff);
		if (*dst < 0xff * f_dst) *dst = 0xff * f_dst;
		f_dst = 1.0f - f_dst;
		nw--; dst--;
		
		*dst = *dst * f_dst + f_nw * *nw; nw--; dst--;
		*dst = *dst * f_dst + f_nw * *nw; nw--; dst--;
		*dst = *dst * f_dst + f_nw * *nw; nw--; dst--;
	}
	else
	{
		dst -= Sprite::BYTES_PER_PIXEL;
		nw  -= Sprite::BYTES_PER_PIXEL;
	}
	
	
	/* BOTTOM EDGE: */
	
	for (line_w = w; line_w > 1; --line_w)
	{
		/* alpha */
		f_dst = opacity * (
			  (f_nw = f_nw_base * *nw / (float)0xff)
			- (f_ne = f_ne_base * *ne / (float)0xff));
		if (*dst < 0xff * f_dst) *dst = 0xff * f_dst;
		f_dst = 1.0f - f_dst;
		nw--; ne--; dst--;
		
		/* red */
		*dst = *dst * f_dst + f_nw * *nw + f_ne * *ne;
		nw--; ne--; dst--;
		
		/* green */
		*dst = *dst * f_dst + f_nw * *nw + f_ne * *ne;
		nw--; ne--; dst--;
		
		/* blue */
		*dst = *dst * f_dst + f_nw * *nw + f_ne * *ne;
		nw--; ne--; dst--;
		
		sw -= Sprite::BYTES_PER_PIXEL; se -= Sprite::BYTES_PER_PIXEL;
	}
	
	
	/* BOTTOM LEFT CORNER: */
	
	if (x_int < 0) return;
	
	f_dst = opacity * (f_ne = f_ne_base * *ne / (float)0xff);
	if (*dst < 0xff * f_dst) *dst = 0xff * f_dst;
	f_dst = 1.0f - f_dst;
	ne--; dst--;
	
	*dst = *dst * f_dst + f_ne * *ne; ne--; dst--;
	*dst = *dst * f_dst + f_ne * *ne; ne--; dst--;
	*dst = *dst * f_dst + f_ne * *ne;
}

void
Sprite::blend(BlendMode blend_mode, const Sprite *s, int x, int y)
{
	this->blend(blend_mode, s, x, y, 1.0f);
}

void
Sprite::blend(BlendMode blend_mode, const Sprite *s, int x, int y, float strength)
{
	Color *src, *dst;
	int line_w;
	int x_src, y_src, w, h;
	float f, f_inv;
	Component *c_dst, *c_src;
	Color c;
	
	if (blend_mode == Sprite::CIRCLE) goto blend_sprite;
	
	x_src = 0; y_src = 0;
	w = s->w; h = s->h;
	
	/* clip as necessary */
	if (x < 0) { w += x; x_src -= x; x = 0; }
	if (y < 0) { h += y; y_src -= y; y = 0; }
	
	if (x_src < 0) { w += x_src; x -= x_src; x_src = 0; }
	if (y_src < 0) { h += y_src; y -= y_src; y_src = 0; }
	
	if (x + w > this->w) w = this->w - x;
	if (y + h > this->h) h = this->h - y;
	
	if (w <= 0)
	{
		return;
	}
	
	if (strength < 0.0f) strength = 0.0f;
	else if (strength > 1.0f) strength = 1.0f;
	
	/* set pointers to upper left corners */
	src = x_src + y_src * s->w        + s->data;
	dst = x     + y     * this->w + this->data;
	
	/* copy lines */
	c_src = (Component *)(src + w) - 1;
	c_dst = (Component *)(dst + w) - 2;
	
blend_sprite:
	switch (blend_mode)
	{
		case Sprite::FADE:
			strength /= 0xff;
			for (; h > 0; --h)
			{
				for (line_w = w; line_w > 0; --line_w)
				{
					/* alpha */
					f = strength * (float)(*c_src);
					f_inv = 1.0f - f;
					c_src--;
					
					/* red */
					*c_dst = f * *c_src + f_inv * *c_dst;
					c_src--; c_dst--;
					
					/* green */
					*c_dst = f * *c_src + f_inv * *c_dst;
					c_src--; c_dst--;
					
					/* blue */
					*c_dst = f * *c_src + f_inv * *c_dst;
					c_src--; c_dst -= 2;
				}
				
				c_src += (s->w + w) * Sprite::BYTES_PER_PIXEL;
				c_dst += (this->w + w) * Sprite::BYTES_PER_PIXEL;
			}
			break;
	
		case Sprite::ADD:
			strength /= 0xff;
			for (; h > 0; --h)
			{
				for (line_w = w; line_w > 0; --line_w)
				{
					/* alpha */
					f = strength * (float)(*c_src);
					c_src--;
					
					/* red */
					c = *c_dst + f * *c_src;
					*c_dst = (c > 0xff) ? 0xff : c;
					c_src--; c_dst--;
					
					/* green */
					c = *c_dst + f * *c_src;
					*c_dst = (c > 0xff) ? 0xff : c;
					c_src--; c_dst--;
					
					/* blue */
					c = *c_dst + f * *c_src;
					*c_dst = (c > 0xff) ? 0xff : c;
					c_src--; c_dst -= 2;
				}
				c_src += (s->w + w) * Sprite::BYTES_PER_PIXEL;
				c_dst += (this->w + w) * Sprite::BYTES_PER_PIXEL;
			}
			break;
	
		case Sprite::SUBTRACT:
			strength /= 0xff;
			for (; h > 0; --h)
			{
				for (line_w = w; line_w > 0; --line_w)
				{
					/* alpha */
					f = strength * (float)(*c_src);
					c_src--;
					
					/* red */
					c = *c_dst - f * *c_src;
					*c_dst = (c > 0xff) ? 0x00 : c;
					c_src--; c_dst--;
					
					/* green */
					c = *c_dst - f * *c_src;
					*c_dst = (c > 0xff) ? 0x00 : c;
					c_src--; c_dst--;
					
					/* blue */
					c = *c_dst - f * *c_src;
					*c_dst = (c > 0xff) ? 0x00 : c;
					c_src--; c_dst -= 2;
				}
				c_src += (s->w + w) * Sprite::BYTES_PER_PIXEL;
				c_dst += (this->w + w) * Sprite::BYTES_PER_PIXEL;
			}
			break;
			
		case Sprite::MULTIPLY:
			f_inv = 1.0f - strength;
			strength /= 0xff * 0xff;
			for (; h > 0; --h)
			{
				for (line_w = w; line_w > 0; --line_w)
				{
					/* alpha */
					f = strength * *c_src;
					c_src--;
					
					/* red */
					*c_dst = f_inv * *c_dst + f * *c_dst * *c_src;
					c_src--; c_dst--;
					
					/* green */
					*c_dst = f_inv * *c_dst + f * *c_dst * *c_src;
					c_src--; c_dst--;
					
					/* blue */
					*c_dst = f_inv * *c_dst + f * *c_dst * *c_src;
					c_src--; c_dst -= 2;
				}
				c_src += (s->w + w) * Sprite::BYTES_PER_PIXEL;
				c_dst += (this->w + w) * Sprite::BYTES_PER_PIXEL;
			}
			break;
		
		case Sprite::RANDOM:
			x = strength * RAND_MAX;
			for (; h > 0; --h)
			{
				for (line_w = w; line_w > 0; --line_w)
				{
					if (rand() % RAND_MAX < x)
					{
						*dst = *src;
					}
					
					src++;
					dst++;
				}
				src += s->w - w;
				dst += this->w - w;
			}
			break;
		
		case Sprite::DROP:
			h *= strength;
			w *= Sprite::BYTES_PER_PIXEL;
			for (; h > 0; --h)
			{
				memcpy(dst, src, w);
				src += s->w;
				dst += this->w;
			}
			break;
		
		case Sprite::SLIDE:
			x = strength * w * Sprite::BYTES_PER_PIXEL;
			for (; h > 0; --h)
			{
				memcpy(dst, src, x);
				src += s->w;
				dst += this->w;
			}
			break;
		
		case Sprite::DISSOLVE:
			x = 0xff * 0xff;
			strength = strength * 2 - 1;
			for (; h > 0; --h)
			{
				for (line_w = w; line_w > 0; --line_w)
				{
					/* alpha */
					f = (float)(*c_src * (rand() % 0xff)) / x + strength;
					if (f < 0.0f) f = 0.0f; else if (f > 1.0f) f = 1.0f;
					f_inv = 1.0f - f;
					c_src--;
					
					/* red */
					*c_dst = f * *c_src + f_inv * *c_dst;
					c_src--; c_dst--;
					
					/* green */
					*c_dst = f * *c_src + f_inv * *c_dst;
					c_src--; c_dst--;
					
					/* blue */
					*c_dst = f * *c_src + f_inv * *c_dst;
					c_src--; c_dst -= 2;
				}
				
				c_src += (s->w + w) * Sprite::BYTES_PER_PIXEL;
				c_dst += (this->w + w) * Sprite::BYTES_PER_PIXEL;
			}
			break;
		
		case Sprite::CIRCLE:
			c_dst = (Component *)(x + y * this->w + this->data);
			c_src = (Component *)(x + y * s->w        + s->data);
			
			x_src = this->w - x; x_src *= x_src;
			y_src = this->h - y; y_src *= y_src;
			
			w = sqrt(x * x + y * y);
			
			line_w = sqrt(x_src + y * y);
			if (line_w > w) w = line_w;
			
			line_w = sqrt(x * x + y_src);
			if (line_w > w) w = line_w;
			
			line_w = sqrt(x_src + y_src);
			if (line_w > w) w = line_w;
			
			w *= strength;
			h  = w;
			w *= w;
			
			while (h > 0)
			{
				line_w = sqrt(w - h * h);
				
				x_src = line_w;
				if (line_w > this->w - x) line_w = this->w - x;
				if (line_w > s->w - x) line_w = s->w - x;
				if (x_src > x) x_src = x;
				
				dst = (Color *)c_dst - x_src - h * this->w;
				src = (Color *)c_src - x_src - h * s->w;
				
				if (dst >= this->data && src >= s->data)
				{
					for (y_src = line_w + x_src; y_src > 0; --y_src)
					{
						*dst = *src;
						dst++;
						src++;
					}
				}
				
				h--;
				if (y + h < this->h && y + h < s->h)
				{
					dst = (Color *)c_dst - x_src + h * this->w;
					src = (Color *)c_src - x_src + h * s->w;
				
					for (y_src = line_w + x_src; y_src > 0; --y_src)
					{
						*dst = *src;
						dst++;
						src++;
					}
				}
			}
		break;
	}
}

void
Sprite::stretch(const Sprite *s, int x, int y, int w, int h)
{
	this->stretch(s, x, y, w, h, 0, 0, s->w, s->h);
}

void
Sprite::stretch(const Sprite *s, int x, int y, int w, int h, int x_src, int y_src, int w_src, int h_src)
{
	Color *src, *src_base, *dst, c;
	int x_dst, y_dst, x_end;
	float x_ratio, y_ratio;
	
	Component *c_dst, *c_src;
	float f_src, f_dst;

	x_ratio = (float)w_src / w;
	y_ratio = (float)h_src / h;
	
	/* clip as necessary */
	if (x < 0) { w += x; x_src -= x * x_ratio; w_src += x * x_ratio; x = 0; }
	if (y < 0) { h += y; y_src -= y * y_ratio; h_src += y * y_ratio; y = 0; }
	
	if (x_src < 0) { w += x_src / x_ratio; x -= x_src / x_ratio; x_src = 0; }
	if (y_src < 0) { h += y_src / y_ratio; y -= y_src / y_ratio; y_src = 0; }
	
	if (x + w > this->w)
	{
		w_src += (this->w - x - w) * x_ratio;
		w = this->w - x;
	}
	
	if (y + h > this->h)
	{
		h_src += (this->h - y - h) * y_ratio;
		h = this->h - y;
	}
	
	if (x_src + w_src > s->w)
	{
		w += (s->w - x_src - w_src) / x_ratio;
		w_src = s->w - x_src;
	}
	
	if (y_src + h_src > s->h)
	{
		h += (s->h - y_src - h_src) / y_ratio;
		h_src = s->h - y_src;
	}
	
	/* draw the clipped sprite */
	if (w <= 0 || w_src <= 0)
	{
		return;
	}
	
	x_end = w;
	
	dst = x + y * this->w + this->data;
	src_base = x_src + y_src * s->w + s->data;
	
	switch (s->alpha_blending)
	{
		case Sprite::IGNORE_ALPHA:
			
			for (y_dst = 0; y_dst < h; ++y_dst)
			{
				src = src_base + (y_dst * h_src / h) * s->w;
				
				for (x_dst = 0; x_dst < x_end; ++x_dst)
				{
					*dst = src[x_dst * w_src / w];
					dst++;
				}
				
				dst += this->w - w;
			}
			return;
		
		
		case Sprite::BINARY_ALPHA:
			for (y_dst = 0; y_dst < h; ++y_dst)
			{
				src = src_base + (y_dst * h_src / h) * s->w;
				
				for (x_dst = 0; x_dst < x_end; ++x_dst)
				{
					c = src[x_dst * w_src / w];
					if (c & 0xff000000)
					{
						*dst = c;
					}
					dst++;
				}
				
				dst += this->w - w;
			}
			return;
		
		
		case Sprite::FULL_ALPHA:
			x_end--;
			c_dst = (Component *)(dst + w) - 2;
			for (y_dst = 0; y_dst < h; ++y_dst)
			{
				src = src_base + (y_dst * h_src / h) * s->w;
				
				for (x_dst = x_end; x_dst >= 0; --x_dst)
				{
					c_src = (Component *)&src[x_dst * w_src / w] - 1;
					
					/* alpha */
					f_src = (float)(*c_src) / 0xff;
					f_dst = 1.0f - f_src;
					c_src--;
					
					/* red */
					*c_dst = f_src * *c_src + f_dst * *c_dst;
					c_src--; c_dst--;
					
					/* green */
					*c_dst = f_src * *c_src + f_dst * *c_dst;
					c_src--; c_dst--;
					
					/* blue */
					*c_dst = f_src * *c_src + f_dst * *c_dst;
					c_dst -= 2;
				}
				
				c_dst += (this->w + w) * Sprite::BYTES_PER_PIXEL;
			}
			return;
	}
}

void
Sprite::rotate(const Sprite *s, int x, int y, float angle, float zoom)
{
	/* FIXME: use matrices instead of atan2 (it's slow and hacky as hell) */
	
	int yy, x_src, y_src, center_x, center_y;
	int x_start, y_start, x_end, y_end;
	float a, d;
	
	Color *dst, *base, color;
	Component *c_src, *c_dst;
	float f_src, f_dst;
	
	angle += 3.141593f / 2.0f;
	
	base = this->data + x + y * this->w;
	
	center_x = s->w / 2; center_y = s->h / 2;
	d = sqrt(center_x * center_x + center_y * center_y) * zoom;
	x_end = (fabs(cos(atan2(-center_x, center_y) + angle))
	       > fabs(cos(atan2( center_x, center_y) + angle)))
		? d * fabs(cos(atan2(-center_x, center_y) + angle))
		: d * fabs(cos(atan2( center_x, center_y) + angle));
	
	y_end = (fabs(sin(atan2(center_x, -center_y) + angle))
	       > fabs(sin(atan2(center_x,  center_y) + angle)))
		? d * fabs(sin(atan2(center_x, -center_y) + angle))
		: d * fabs(sin(atan2(center_x,  center_y) + angle));
	
	x_end += zoom + 1.5f; x_start = -x_end;
	if (x_start + x < 0) x_start = -x;
	if (x_end + x >= this->w) x_end = this->w - x;
	
	y_end += zoom + 1.5f; y_start = -y_end;
	if (y_start + y < 0) y_start = -y;
	if (y_end + y >= this->h) y_end = this->h - y;
	
	base += x_start;
	
	switch (s->alpha_blending)
	{
		case Sprite::IGNORE_ALPHA:
			for (y = y_start; y < y_end; y++)
			{
				yy = y * y;
				dst = base + y * this->w;
				for (x = x_start; x < x_end; x++)
				{
					d = sqrt(x * x + yy) / zoom;
					a = atan2(x, y) + angle;
					x_src = center_x + d * cos(a);
					if (x_src >= 0 && x_src < s->w)
					{
						y_src = center_y + d * sin(a);
						if (y_src >= 0 && y_src < s->h)
						{
							*dst = *(s->data + x_src + y_src * s->w);
						}
					}
					dst++;
				}
			}
			return;
		
		case Sprite::BINARY_ALPHA:
			for (y = y_start; y < y_end; y++)
			{
				yy = y * y;
				dst = base + y * this->w;
				for (x = x_start; x < x_end; x++)
				{
					d = sqrt(x * x + yy) / zoom;
					a = atan2(x, y) + angle;
					x_src = center_x + d * cos(a);
					if (x_src >= 0 && x_src < s->w)
					{
						y_src = center_y + d * sin(a);
						if (y_src >= 0 && y_src < s->h)
						{
							color = *(s->data + x_src + y_src * s->w);
							if (color & 0xff000000)
							{
								*dst = color;
							}
						}
					}
					dst++;
				}
			}
			return;
			
		case Sprite::FULL_ALPHA:
			for (y = y_start; y < y_end; y++)
			{
				yy = y * y;
				c_dst = (Component *)(base + y * this->w);
				for (x = x_start; x < x_end; x++)
				{
					d = sqrt(x * x + yy) / zoom;
					a = atan2(x, y) + angle;
					x_src = center_x + d * cos(a);
					if (x_src >= 0 && x_src < s->w)
					{
						y_src = center_y + d * sin(a);
						if (y_src >= 0 && y_src < s->h)
						{
							c_src = (Component *)&s->data[x_src + y_src * s->w];
							
							/* alpha */
							f_src = (float)c_src[3] / 0xff;
							f_dst = 1.0f - f_src;
							
							/* blue */
							*c_dst = f_src * *c_src + f_dst * *c_dst;
							c_src++; c_dst++;
							
							/* green */
							*c_dst = f_src * *c_src + f_dst * *c_dst;
							c_src++; c_dst++;
							
							/* red */
							*c_dst = f_src * *c_src + f_dst * *c_dst;
							c_dst += 2;
						}
						else
						{
							c_dst += Sprite::BYTES_PER_PIXEL;
						}
					}
					else
					{
						c_dst += Sprite::BYTES_PER_PIXEL;
					}
				}
			}
			return;
	}
}

/*
________________________________________________________________________________

	S H A P E   D R A W I N G
________________________________________________________________________________

*/

void
Sprite::drawcircle(int x, int y, int radius, Color c)
{
	/* Draws a circle using the midpoint circle algorithm */
	/* FIXME: something's fucky when a circle is partly offscreen (top edge) */
	
	int x_dst, y_dst, x_dst_row, y_dst_row;
	int f;		/* f = x^2 + y^2 - radius^2 + 2x - y + 1; */
	int ddf_x;	/* ddf_x = 2x + 1; */
	int ddf_y;	/* ddf_y = -2y; */
	Color *dst;
	
	int min_x, min_y, max_x, max_y;
	
	if (radius < 0) radius = -radius;
	x_dst = 0; y_dst = radius;
	ddf_x = 1; ddf_y = -2 * radius;
	f = 1 - radius;
	
	/* center point */
	dst = this->data + x + y * this->w;
	
	if (this->clipping.enabled == 0)
	{
		goto circle_noclip;
	}
	
	min_x = this->clipping.x;
	max_x = this->clipping.x + this->clipping.w - 1;
	min_y = this->clipping.y;
	max_y = this->clipping.y + this->clipping.h - 1;
	
	/* skip drawing if everything is off-screen */
	if (x < min_x - radius || x > max_x + radius ||
		y < min_y - radius || y > max_y + radius)
	{
		return;
	}
	
	/* draw without boundary checking if all pixels fit */
	if (x >= min_x + radius && x < max_x - radius &&
		y >= min_y + radius && y < max_y - radius)
	{
		goto circle_noclip;
	}
	
	/* top- and bottommost pixels */
	if (x >= min_x && x <= max_x)
	{
		if (y <= max_y - radius) *(dst + radius * this->w) = c;
		if (y >= min_y + radius) *(dst - radius * this->w) = c;
	}
	/* left- and rightmost pixels */
	if (y >= min_y && y <= max_y)
	{
		if (x <= max_x - radius) *(dst + radius) = c;
		if (x >= min_x + radius) *(dst - radius) = c;
	}
	
	/* circle quadrants */
	while (x_dst < y_dst)
	{
		if (f >= 0)
		{
			y_dst--;
			ddf_y += 2;
			f += ddf_y;
		}
		x_dst++;
		ddf_x += 2;
		f += ddf_x;
		
		x_dst_row = x_dst * this->w;
		y_dst_row = y_dst * this->w;
		
		if (x + x_dst <= max_x)
		{
			if (y + y_dst <= max_y) *(dst + x_dst + y_dst_row) = c;
			if (y - y_dst >= min_y) *(dst + x_dst - y_dst_row) = c;
		}
		if (x + y_dst <= max_x)
		{
			if (y + x_dst <= max_y) *(dst + y_dst + x_dst_row) = c;
			if (y - x_dst >= min_y) *(dst + y_dst - x_dst_row) = c;
		}
		if (x - x_dst >= min_x)
		{
			if (y + y_dst <= max_y) *(dst - x_dst + y_dst_row) = c;
			if (y - y_dst >= min_y) *(dst - x_dst - y_dst_row) = c;
		}
		if (x - y_dst >= min_x)
		{
			if (y + x_dst <= max_y) *(dst - y_dst + x_dst_row) = c;
			if (y - x_dst >= min_y) *(dst - y_dst - x_dst_row) = c;
		}
	}
	
	return;
	
	
circle_noclip:
	*(dst + radius * this->w) = c;
	*(dst - radius * this->w) = c;
	*(dst + radius) = c;
	*(dst - radius) = c;
	
	while (x_dst < y_dst)
	{
		if (f >= 0)
		{
			y_dst--;
			ddf_y += 2;
			f += ddf_y;
		}
		x_dst++;
		ddf_x += 2;
		f += ddf_x;
		
		x_dst_row = x_dst * this->w;
		y_dst_row = y_dst * this->w;
		
		*(dst + x_dst + y_dst_row) = c;
		*(dst + x_dst - y_dst_row) = c;
		*(dst + y_dst + x_dst_row) = c;
		*(dst + y_dst - x_dst_row) = c;
		*(dst - x_dst + y_dst_row) = c;
		*(dst - x_dst - y_dst_row) = c;
		*(dst - y_dst + x_dst_row) = c;
		*(dst - y_dst - x_dst_row) = c;
	}
}

void
Sprite::drawcircle_a(float x, float y, float radius, Color c)
{
	/* Draws an anti-aliased circle using Xiaolin Wu's circle extension. */
	
	int i;
	float xj, yj;   /* real part of i */
	float f, f_inv; /* fractional part and its inverse */
	int f_int;      /* integer part */
	
	Color *center, *dst, *p_dst, *dst_min, *dst_max;
	
	if (radius <= 0)
	{
		if (++radius > 0 &&
			x >= 0 && y >= 0 && x < this->w && y < this->h)
		{
			this->putpixel(x, y, c, radius);
		}
		return;
	}
	
	/* EDGES (would be overdrawn otherwise) */
	f = radius - (f_int = (int)radius);
	f_inv = 1.0f - f;
	
	if (y >= 0 && y < this->h)
	{
		i = x + f_int;
		if (i >= 0 && i < this->w - 1)
		{
			this->putpixel(i, y, c, f_inv);
			if (++i) this->putpixel(i, y, c, f);
		}
		
		i = x - f_int;
		if (i > 0 && i < this->w)
		{
			this->putpixel(i, y, c, f_inv);
			if (--i) this->putpixel(i, y, c, f);
		}
	}
	
	if (x >= 0 && x < this->w)
	{
		i = y + f_int;
		if (i >= 0 && i < this->h - 1)
		{
			this->putpixel(x, i, c, f_inv);
			if (++i) this->putpixel(x, i, c, f);
		}
		
		i = y - f_int;
		if (i > 0 && i < this->h)
		{
			this->putpixel(x, i, c, f_inv);
			if (--i) this->putpixel(x, i, c, f);
		}
	}
	
	if (radius < 1)
	{
		return;
	}
	
	/* CIRCLE BODY */
	center  = this->data + (int)x + (int)y * this->w;
	dst_min = this->data + this->w;
	dst_max = this->data + (this->h - 1) * this->w;
	
	i = radius / sqrt(2) + .5f;
	radius *= radius;
	
	for (; i > 0; --i)
	{
		xj = yj = sqrt(radius - i * i);
		
		/*
		HORIZONTAL PART:
			p(i, f_int) = 1 - f;
			p(i, f_int + 1) = f;
			
			where p(x, y) draws at:
				center + (-x, -y), (+x, -y), (-x, +y), (+x, +y)
		*/
		f = yj - (f_int = (int)yj);
		f_inv = 1.0f - f;
		
		f_int *= this->w;
		if (x - i >= 0 && x - i < this->w)
		{
			/* pixels on the left side (x - i, <y>) */
			dst = center - i; /* (x - i, y + 0) */
			
			p_dst = dst - f_int; /* (x - i, y - f_int) */
			if (p_dst >= dst_min && p_dst < dst_max)
			{ /* (x - i, y - f_int) and (x - i, y - (f_int + 1)) */
				_putpixel((Component *)(p_dst), c, f_inv);
				_putpixel((Component *)(p_dst - this->w), c, f);
			}
			p_dst = dst + f_int; /* (x - i, y + f_int) */
			if (p_dst >= dst_min && p_dst < dst_max)
			{ /* (x - i, y + f_int) and (x - i, y + (f_int + 1)) */
				_putpixel((Component *)(p_dst), c, f_inv);
				_putpixel((Component *)(p_dst + this->w), c, f);
			}
		}
		
		if (x + i >= 0 && x + i < this->w)
		{
			/* pixels on the right side (x + i, <y>) */
			dst = center + i; /* (x + i, y + 0) */
			
			p_dst = dst - f_int; /* (x + i, y - f_int) */
			if (p_dst >= dst_min && p_dst < dst_max)
			{ /* (x + i, y - f_int) and (x + i, y - (f_int + 1)) */
				_putpixel((Component *)(p_dst - this->w), c, f);
				_putpixel((Component *)(p_dst), c, f_inv);
			}
			p_dst = dst + f_int; /* (x + i, y + f_int) */
			if (p_dst >= dst_min && p_dst < dst_max)
			{ /* (x + i, y + f_int) and (x + i, y + (f_int + 1)) */
				_putpixel((Component *)(p_dst), c, f_inv);
				_putpixel((Component *)(p_dst + this->w), c, f);
			}
		}
		
		/*
		VERTICAL PART:
			p(f_int, i) = 1 - f;
			p(f_int + 1, i) = f;
			
			where p(x, y) is the same as above, drawing at:
				center + (-x, -y), (+x, -y), (-x, +y), (+x, +y)
		*/
		f = xj - (f_int = (int)xj);
		f_inv = 1.0f - f;
		
		if (y - i >= 0 && y - i < this->h)
		{
			/* pixels on the upper side (<x>, y - i) */
			dst = center - i * this->w; /* (x + 0, y - i) */
			
			p_dst = dst - f_int; /* (x - f_int, y - i) */
			if (x - f_int >= 1 && x - f_int < this->w)
			{ /* (x - (f_int), y - i) and (x - (f_int + 1), y - i) */
				_putpixel((Component *)(p_dst), c, f_inv);
				_putpixel((Component *)(p_dst - 1), c, f);
			}
			p_dst = dst + f_int; /* (x + f_int, y - i) */
			if (x + f_int >= 0 && x + f_int < this->w - 1)
			{ /* (x + (f_int), y - i) and (x + (f_int + 1), y - i) */
				_putpixel((Component *)(p_dst), c, f_inv);
				_putpixel((Component *)(p_dst + 1), c, f);
			}
		}
		
		if (y + i >= 0 && y + i < this->h)
		{
			/* pixels on the upper side (<x>, y + i) */
			dst = center + i * this->w; /* (x + 0, y + i) */
			
			p_dst = dst - f_int; /* (x - f_int, y + i) */
			if (x - f_int >= 1 && x - f_int < this->w)
			{ /* (x - (f_int), y + i) and (x - (f_int + 1), y + i) */
				_putpixel((Component *)(p_dst), c, f_inv);
				_putpixel((Component *)(p_dst - 1), c, f);
			}
			p_dst = dst + f_int; /* (x + f_int, y + i) */
			if (x + f_int >= 0 && x + f_int < this->w - 1)
			{ /* (x + (f_int), y + i) and (x + (f_int + 1), y + i) */
				_putpixel((Component *)(p_dst), c, f_inv);
				_putpixel((Component *)(p_dst + 1), c, f);
			}
		}
	}
}

void
Sprite::drawline(int x0, int y0, int x1, int y1, Color c)
{
	/* Draw a line between two points using Bresenham's line algorithm.
	 * If a rect is given, only inside this rect will be drawn.
	 * Otherwise there will be no boundary checking.
	 */
	
	/* FIXME: something's fucky with clipping (left & top edges) */
	
	int dx, dy;
	int xpxl0, ypxl0, xpxl1, ypxl1;
	char steep;
	int temp, error, y_step;
	
	Color *p;
	
	/* check boundaries */
	if (this->clipping.enabled)
	{
		xpxl0 = this->clipping.x + 1;
		ypxl0 = this->clipping.y + 1;
		xpxl1 = this->clipping.x + this->clipping.w - 1;
		ypxl1 = this->clipping.y + this->clipping.h - 1;
		
		if (x0 < xpxl0)
		{
			if (x1 < xpxl0) return;
			if (x0 != x1) y0 += (y1 - y0) * (xpxl0 - x0) / (x1 - x0);
			x0 = xpxl0;
		}
		else if (x0 > xpxl1)
		{
			if (x1 > xpxl1) return;
			if (x0 != x1) y0 -= (y1 - y0) * abs(x0 - xpxl1) / (x1 - x0);
			x0 = xpxl1;
		}
	
		/* swap x and y */
		if (y0 < ypxl0)
		{
			if (y1 < ypxl0) return;
			if (y0 != y1) x0 += (x1 - x0) * (ypxl0 - y0) / (y1 - y0);
			y0 = ypxl0;
		}
		else if (y0 > ypxl1)
		{
			if (y1 > ypxl1) return;
			if (y0 != y1) x0 -= (x1 - x0) * abs(y0 - ypxl1) / (y1 - y0);
			y0 = ypxl1;
		}
	
		/* swap 0 and 1 */
		if (x1 < xpxl0)
		{
			if (x0 != x1) y1 += (y1 - y0) * (xpxl0 - x1) / (x1 - x0);
			x1 = xpxl0;
		}
		else if (x1 > xpxl1)
		{
			if (x0 != x1) y1 = y0 + (y1 - y0) * abs(x0 - xpxl1) / (x1 - x0);
			x1 = xpxl1;
		}
	
		/* swap x and y, swap 0 and 1 */
		if (y1 < ypxl0)
		{
			if (y0 != y1) x1 += (x1 - x0) * (ypxl0 - y1) / (y1 - y0);
			y1 = ypxl0;
		}
		else if (y1 > ypxl1)
		{
			if (y0 != y1) x1 = x0 + (x1 - x0) * abs(y0 - ypxl1) / (y1 - y0);
			y1 = ypxl1;
		}
	}
	
	/* Bresenham's algorithm */
    if (abs(y1 - y0) > abs(x1 - x0))
    {
		temp = x0; x0 = y0; y0 = temp;
		temp = x1; x1 = y1; y1 = temp;
		steep = 1;
	}
	else
	{
		steep = 0;
	}
	
	if (x0 > x1)
	{
		temp = x0; x0 = x1; x1 = temp;
		temp = y0; y0 = y1; y1 = temp;
	}
	dx = x1 - x0; dy = abs(y1 - y0);
	error = dx / 2;
	
	if (steep)
	{
		y_step = (y0 < y1) ? 1 : -1;
		p = this->data + y0 + x0 * this->w;
		for (x1 -= x0; x1 >= 0; --x1)
		{
			*p = c;
			p += this->w;
			if ((error -= dy) < 0)
			{
				p += y_step;
				error += dx;
			}
		}
	}
	else
	{
		y_step = (y0 < y1) ? this->w : -this->w;
		p = this->data + x0 + y0 * this->w;
		for (x1 -= x0; x1 >= 0; --x1)
		{
			*p = c;
			p++;
			if ((error -= dy) < 0)
			{
				p += y_step;
				error += dx;
			}
		}
	}
}

void
Sprite::drawline(int x0, int y0, int x1, int y1, Color c, float opacity)
{
	/* Draw a line between two points using Bresenham's line algorithm.
	 * If a rect is given, only inside this rect will be drawn.
	 * Otherwise there will be no boundary checking.
	   
	   This is a semi-translucent version.
	 */
	
	int dx, dy;
	int xpxl0, ypxl0, xpxl1, ypxl1;
	char steep;
	int temp, error, y_step;
	
	Color *p;
	
	/* check boundaries */
	if (this->clipping.enabled)
	{
		xpxl0 = this->clipping.x + 1;
		ypxl0 = this->clipping.y + 1;
		xpxl1 = this->clipping.x + this->clipping.w - 1;
		ypxl1 = this->clipping.y + this->clipping.h - 1;
		
		if (x0 < xpxl0)
		{
			if (x1 < xpxl0) return;
			if (x0 != x1) y0 += (y1 - y0) * (xpxl0 - x0) / (x1 - x0);
			x0 = xpxl0;
		}
		else if (x0 > xpxl1)
		{
			if (x1 > xpxl1) return;
			if (x0 != x1) y0 -= (y1 - y0) * abs(x0 - xpxl1) / (x1 - x0);
			x0 = xpxl1;
		}
	
		/* swap x and y */
		if (y0 < ypxl0)
		{
			if (y1 < ypxl0) return;
			if (y0 != y1) x0 += (x1 - x0) * (ypxl0 - y0) / (y1 - y0);
			y0 = ypxl0;
		}
		else if (y0 > ypxl1)
		{
			if (y1 > ypxl1) return;
			if (y0 != y1) x0 -= (x1 - x0) * abs(y0 - ypxl1) / (y1 - y0);
			y0 = ypxl1;
		}
	
		/* swap 0 and 1 */
		if (x1 < xpxl0)
		{
			if (x0 != x1) y1 += (y1 - y0) * (xpxl0 - x1) / (x1 - x0);
			x1 = xpxl0;
		}
		else if (x1 > xpxl1)
		{
			if (x0 != x1) y1 = y0 + (y1 - y0) * abs(x0 - xpxl1) / (x1 - x0);
			x1 = xpxl1;
		}
	
		/* swap x and y, swap 0 and 1 */
		if (y1 < ypxl0)
		{
			if (y0 != y1) x1 += (x1 - x0) * (ypxl0 - y1) / (y1 - y0);
			y1 = ypxl0;
		}
		else if (y1 > ypxl1)
		{
			if (y0 != y1) x1 = x0 + (x1 - x0) * abs(y0 - ypxl1) / (y1 - y0);
			y1 = ypxl1;
		}
	}
	
	/* Bresenham's algorithm */
    if (abs(y1 - y0) > abs(x1 - x0))
    {
		temp = x0; x0 = y0; y0 = temp;
		temp = x1; x1 = y1; y1 = temp;
		steep = 1;
	}
	else
	{
		steep = 0;
	}
	
	if (x0 > x1)
	{
		temp = x0; x0 = x1; x1 = temp;
		temp = y0; y0 = y1; y1 = temp;
	}
	dx = x1 - x0; dy = abs(y1 - y0);
	error = dx / 2;
	
	if (steep)
	{
		y_step = (y0 < y1) ? 1 : -1;
		p = this->data + y0 + x0 * this->w;
		for (x1 -= x0; x1 >= 0; --x1)
		{
			_putpixel((Component *)p, c, opacity);
			p += this->w;
			if ((error -= dy) < 0)
			{
				p += y_step;
				error += dx;
			}
		}
	}
	else
	{
		y_step = (y0 < y1) ? this->w : -this->w;
		p = this->data + x0 + y0 * this->w;
		for (x1 -= x0; x1 >= 0; --x1)
		{
			_putpixel((Component *)p, c, opacity);
			p++;
			if ((error -= dy) < 0)
			{
				p += y_step;
				error += dx;
			}
		}
	}
}

void
Sprite::drawline_a(float x0, float y0, float x1, float y1, Color c)
{
	/* Draw a line between two points using Xiaolin Wu's algorithm
	   for anti-aliased lines.
	   
	   This is a semi-translucent version.
	   
	   If a rect is given, only inside this rect will be drawn.
	   Otherwise there will be no boundary checking.
	*/
	
	/* FIXME: something's fucky here (prolly endpoints) */
	
	float temp;
	float dx, dy, gradient;
	float x_end, y_end;
	float x_gap;
	float xpxl0, ypxl0, xpxl1, ypxl1;
	float x, inter_y;
	
	/* check boundaries */
	if (this->clipping.enabled)
	{
		xpxl0 = this->clipping.x + 1;
		ypxl0 = this->clipping.y + 1;
		xpxl1 = this->clipping.x + this->clipping.w - 1;
		ypxl1 = this->clipping.y + this->clipping.h - 1;
		
		if (x0 < xpxl0)
		{
			if (x1 < xpxl0) return;
			if (x0 != x1) y0 += (y1 - y0) * (xpxl0 - x0) / (x1 - x0);
			x0 = xpxl0;
		}
		else if (x0 > xpxl1)
		{
			if (x1 > xpxl1) return;
			if (x0 != x1) y0 -= (y1 - y0) * fabs(x0 - xpxl1) / (x1 - x0);
			x0 = xpxl1;
		}
		/* swap x and y */
		if (y0 < ypxl0)
		{
			if (y1 < ypxl0) return;
			if (y0 != y1) x0 += (x1 - x0) * (ypxl0 - y0) / (y1 - y0);
			y0 = ypxl0;
		}
		else if (y0 > ypxl1)
		{
			if (y1 > ypxl1) return;
			if (y0 != y1) x0 -= (x1 - x0) * fabs(y0 - ypxl1) / (y1 - y0);
			y0 = ypxl1;
		}
		/* swap 0 and 1 */
		if (x1 < xpxl0)
		{
			if (x0 != x1) y1 += (y1 - y0) * (xpxl0 - x1) / (x1 - x0);
			x1 = xpxl0;
		}
		else if (x1 > xpxl1)
		{
			if (x0 != x1) y1 = y0 + (y1 - y0) * fabs(x0 - xpxl1) / (x1 - x0);
			x1 = xpxl1;
		}
		/* swap x and y, swap 0 and 1 */
		if (y1 < ypxl0)
		{
			if (y0 != y1) x1 += (x1 - x0) * (ypxl0 - y1) / (y1 - y0);
			y1 = ypxl0;
		}
		else if (y1 > ypxl1)
		{
			if (y0 != y1) x1 = x0 + (x1 - x0) * fabs(y0 - ypxl1) / (y1 - y0);
			y1 = ypxl1;
		}
	}
	
	dx = x1 - x0;
	dy = y1 - y0;
	
	/* handle horizontal lines */
	if (fabs(dy) < .00001f)
	{
		if (x1 < x0)
		{
			temp = x0;
			x0 = x1;
			x1 = temp;
		}
		dx = rfrac(y0);
		dy =  frac(y0);
		y1 = y0 + 1;
		this->putpixel(x0, y0, c, dx * rfrac(x0));
		this->putpixel(x0, y1, c, dy * rfrac(x0));
		for (x = x0; x < x1; x++)
		{
			this->putpixel(x, y0, c, dx);
			this->putpixel(x, y1, c, dy);
		}
		this->putpixel(x1, y0, c, dx * frac(x1));
		this->putpixel(x1, y1, c, dy * frac(x1));
	
	/* handle vertical lines */
	}
	else if (fabs(dx) < .00001f)
	{
		if (y1 < y0)
		{
			temp = y0;
			y0 = y1;
			y1 = temp;
		}
		dx = rfrac(x0);
		dy =  frac(x0);
		x1 = x0 + 1;
		this->putpixel(x0, y0, c, dx * rfrac(y0));
		this->putpixel(x1, y0, c, dy * rfrac(y0));
		for (x = y0; x < y1; x++)
		{
			this->putpixel(x0, x, c, dx);
			this->putpixel(x1, x, c, dy);
		}
		this->putpixel(x0, y1, c, dx * frac(y1));
		this->putpixel(x1, y1, c, dy * frac(y1));
	
	
	
	/* Xiaolin Wu's algorithm for anti-aliased lines */
	
	}
	else if (fabs(dx) > fabs(dy))
	{
	/* handle more horizontal than vertical diagonal lines */
		if (x1 < x0)
		{
			temp = x0; x0 = x1; x1 = temp;
			temp = y0; y0 = y1; y1 = temp;
		}
		
		gradient = dy / dx;
		
		/* handle first endpoint */
		x_end = (int)(x0 + .5f);
		y_end = y0 + gradient * (x_end - x0);
		x_gap = rfrac(x0 + .5f);
		xpxl0 = x_end; /* this will be used in the main loop */
		ypxl0 = (int)y_end;
		inter_y = y_end + gradient; /* first y-intersection for the main loop */
		
		/* draw first endpoint */
		this->putpixel(xpxl0, ypxl0, c, rfrac(y_end) * x_gap);
		this->putpixel(xpxl0, ypxl0 + 1, c, frac(y_end) * x_gap);
		
		/* handle second endpoint */
		x_end = (int)(x1 + .5f);
		y_end = y1 + gradient * (x_end - x1);
		x_gap = frac(x1 + .5f);
		xpxl1 = x_end; /* this will be used in the main loop */
		ypxl1 = (int)y_end;
		
		/* draw second endpoint */
		this->putpixel(xpxl1, ypxl1, c, rfrac(y_end) * x_gap);
		this->putpixel(xpxl1, ypxl1 + 1, c, frac(y_end) * x_gap);
		
		/* main loop */
		for (x = xpxl0 + 1; x <= xpxl1 - 1; x++)
		{
			this->putpixel(x, (int)inter_y, c, rfrac(inter_y));
			this->putpixel(x, (int)inter_y + 1, c, frac(inter_y));
			inter_y += gradient;
		}
		
	}
	else
	{
	/* handle more vertical than horizontal diagonal lines */
	/* same code as above but X takes the role of Y */
		if (y1 < y0)
		{
			temp = x0; x0 = x1; x1 = temp;
			temp = y0; y0 = y1; y1 = temp;
		}
		
		gradient = dx / dy;
		
		/* handle first endpoint */
		x_end = (int)(x0 + .5f);
		y_end = y0 + gradient * (x_end - x0);
		x_gap = rfrac(x0 + .5f);
		xpxl0 = x_end; /* this will be used in the main loop */
		ypxl0 = (int)y_end;
		inter_y = x_end + gradient; /* first x-intersection for the main loop */
		
		/* draw first endpoint */
		this->putpixel(xpxl0, ypxl0, c, rfrac(y_end) * x_gap);
		this->putpixel(xpxl0 + 1, ypxl0, c, frac(y_end) * x_gap);
		
		/* handle second endpoint */
		x_end = (int)(x1 + .5f);
		y_end = y1 + gradient * (x_end - x1);
		x_gap = frac(x1 + .5f);
		xpxl1 = x_end; /* this will be used in the main loop */
		ypxl1 = (int)y_end;
		
		/* draw second endpoint */
		this->putpixel(xpxl1, ypxl1, c, rfrac(y_end) * x_gap);
		this->putpixel(xpxl1 + 1, ypxl1, c, frac(y_end) * x_gap);
		
		/* main loop */
		for (x = ypxl0 + 1; x <= ypxl1 - 1; x++)
		{
			this->putpixel((int)inter_y, x, c, rfrac(inter_y));
			this->putpixel((int)inter_y + 1, x, c, frac(inter_y));
			inter_y += gradient;
		}
	}
}

void
Sprite::drawline_a(float x0, float y0, float x1, float y1, Color c, float opacity)
{
	/* Draw a line between two points using Xiaolin Wu's algorithm
	   for anti-aliased lines.
	   If a rect is given, only inside this rect will be drawn.
	   Otherwise there will be no boundary checking.
	*/
	
	/* FIXME: something's fucky here (prolly endpoints) */
	
	float temp;
	float dx, dy, gradient;
	float x_end, y_end;
	float x_gap;
	float xpxl0, ypxl0, xpxl1, ypxl1;
	float x, inter_y;
	
	/* check boundaries */
	if (this->clipping.enabled)
	{
		xpxl0 = this->clipping.x + 1;
		ypxl0 = this->clipping.y + 1;
		xpxl1 = this->clipping.x + this->clipping.w - 1;
		ypxl1 = this->clipping.y + this->clipping.h - 1;
		
		if (x0 < xpxl0)
		{
			if (x1 < xpxl0) return;
			if (x0 != x1) y0 += (y1 - y0) * (xpxl0 - x0) / (x1 - x0);
			x0 = xpxl0;
		}
		else if (x0 > xpxl1)
		{
			if (x1 > xpxl1) return;
			if (x0 != x1) y0 -= (y1 - y0) * fabs(x0 - xpxl1) / (x1 - x0);
			x0 = xpxl1;
		}
		/* swap x and y */
		if (y0 < ypxl0)
		{
			if (y1 < ypxl0) return;
			if (y0 != y1) x0 += (x1 - x0) * (ypxl0 - y0) / (y1 - y0);
			y0 = ypxl0;
		}
		else if (y0 > ypxl1)
		{
			if (y1 > ypxl1) return;
			if (y0 != y1) x0 -= (x1 - x0) * fabs(y0 - ypxl1) / (y1 - y0);
			y0 = ypxl1;
		}
		/* swap 0 and 1 */
		if (x1 < xpxl0)
		{
			if (x0 != x1) y1 += (y1 - y0) * (xpxl0 - x1) / (x1 - x0);
			x1 = xpxl0;
		}
		else if (x1 > xpxl1)
		{
			if (x0 != x1) y1 = y0 + (y1 - y0) * fabs(x0 - xpxl1) / (x1 - x0);
			x1 = xpxl1;
		}
		/* swap x and y, swap 0 and 1 */
		if (y1 < ypxl0)
		{
			if (y0 != y1) x1 += (x1 - x0) * (ypxl0 - y1) / (y1 - y0);
			y1 = ypxl0;
		}
		else if (y1 > ypxl1)
		{
			if (y0 != y1) x1 = x0 + (x1 - x0) * fabs(y0 - ypxl1) / (y1 - y0);
			y1 = ypxl1;
		}
	}
	
	dx = x1 - x0;
	dy = y1 - y0;
	
	/* handle horizontal lines */
	if (fabs(dy) < .00001f)
	{
		if (x1 < x0)
		{
			temp = x0;
			x0 = x1;
			x1 = temp;
		}
		dx = rfrac(y0) * opacity;
		dy =  frac(y0) * opacity;
		y1 = y0 + 1;
		this->putpixel(x0, y0, c, dx * rfrac(x0));
		this->putpixel(x0, y1, c, dy * rfrac(x0));
		for (x = x0; x < x1; x++)
		{
			this->putpixel(x, y0, c, dx);
			this->putpixel(x, y1, c, dy);
		}
		this->putpixel(x1, y0, c, dx * frac(x1));
		this->putpixel(x1, y1, c, dy * frac(x1));
	
	/* handle vertical lines */
	}
	else if (fabs(dx) < .00001f)
	{
		if (y1 < y0)
		{
			temp = y0;
			y0 = y1;
			y1 = temp;
		}
		dx = rfrac(x0) * opacity;
		dy =  frac(x0) * opacity;
		x1 = x0 + 1;
		this->putpixel(x0, y0, c, dx * rfrac(y0));
		this->putpixel(x1, y0, c, dy * rfrac(y0));
		for (x = y0; x < y1; x++)
		{
			this->putpixel(x0, x, c, dx);
			this->putpixel(x1, x, c, dy);
		}
		this->putpixel(x0, y1, c, dx * frac(y1));
		this->putpixel(x1, y1, c, dy * frac(y1));
	
	
	
	/* Xiaolin Wu's algorithm for anti-aliased lines */
	
	}
	else if (fabs(dx) > fabs(dy))
	{
	/* handle more horizontal than vertical diagonal lines */
		if (x1 < x0)
		{
			temp = x0; x0 = x1; x1 = temp;
			temp = y0; y0 = y1; y1 = temp;
		}
		
		gradient = dy / dx;
		
		/* handle first endpoint */
		x_end = (int)(x0 + .5f);
		y_end = y0 + gradient * (x_end - x0);
		x_gap = rfrac(x0 + .5f) * opacity;
		xpxl0 = x_end; /* this will be used in the main loop */
		ypxl0 = (int)y_end;
		inter_y = y_end + gradient; /* first y-intersection for the main loop */
		
		/* draw first endpoint */
		this->putpixel(xpxl0, ypxl0, c, rfrac(y_end) * x_gap);
		this->putpixel(xpxl0, ypxl0 + 1, c, frac(y_end) * x_gap);
		
		/* handle second endpoint */
		x_end = (int)(x1 + .5f);
		y_end = y1 + gradient * (x_end - x1);
		x_gap = frac(x1 + .5f) * opacity;
		xpxl1 = x_end; /* this will be used in the main loop */
		ypxl1 = (int)y_end;
		
		/* draw second endpoint */
		this->putpixel(xpxl1, ypxl1, c, rfrac(y_end) * x_gap);
		this->putpixel(xpxl1, ypxl1 + 1, c, frac(y_end) * x_gap);
		
		/* main loop */
		for (x = xpxl0 + 1; x <= xpxl1 - 1; x++)
		{
			this->putpixel(x, (int)inter_y, c, rfrac(inter_y) * opacity);
			this->putpixel(x, (int)inter_y + 1, c, frac(inter_y) * opacity);
			inter_y += gradient;
		}
		
	}
	else
	{
	/* handle more vertical than horizontal diagonal lines */
	/* same code as above but X takes the role of Y */
		if (y1 < y0)
		{
			temp = x0; x0 = x1; x1 = temp;
			temp = y0; y0 = y1; y1 = temp;
		}
		
		gradient = dx / dy;
		
		/* handle first endpoint */
		x_end = (int)(x0 + .5f);
		y_end = y0 + gradient * (x_end - x0);
		x_gap = rfrac(x0 + .5f) * opacity;
		xpxl0 = x_end; /* this will be used in the main loop */
		ypxl0 = (int)y_end;
		inter_y = x_end + gradient; /* first x-intersection for the main loop */
		
		/* draw first endpoint */
		this->putpixel(xpxl0, ypxl0, c, rfrac(y_end) * x_gap);
		this->putpixel(xpxl0 + 1, ypxl0, c, frac(y_end) * x_gap);
		
		/* handle second endpoint */
		x_end = (int)(x1 + .5f);
		y_end = y1 + gradient * (x_end - x1);
		x_gap = frac(x1 + .5f) * opacity;
		xpxl1 = x_end; /* this will be used in the main loop */
		ypxl1 = (int)y_end;
		
		/* draw second endpoint */
		this->putpixel(xpxl1, ypxl1, c, rfrac(y_end) * x_gap);
		this->putpixel(xpxl1 + 1, ypxl1, c, frac(y_end) * x_gap);
		
		/* main loop */
		for (x = ypxl0 + 1; x <= ypxl1 - 1; x++)
		{
			this->putpixel((int)inter_y, x, c, rfrac(inter_y) * opacity);
			this->putpixel((int)inter_y + 1, x, c, frac(inter_y) * opacity);
			inter_y += gradient;
		}
	}
}

void
Sprite::drawrect(int x0, int y0, int x1, int y1, Color c)
{
	int i, len;
	Color *dst0, *dst1;
	
	if (x1 < x0)
	{
		i  = x0;
		x0 = x1;
		x1 = i;
	}
	
	if (y1 < y0)
	{
		i  = y0;
		y0 = y1;
		y1 = i;
	}
	
	/* horizontal */
	i   = (x0 >= this->clipping.x) ? x0 : this->clipping.x;
	len = (x1 < this->clipping.w) ? x1 - i : this->clipping.w - i;
	
	if (y0 >= this->clipping.y && y0 < this->clipping.h)
	{
		dst0 = this->data + i + y0 * this->w;
		if (y1 >= this->clipping.x && y1 < this->clipping.h)
		{
			dst1 = this->data + i + y1 * this->w;
			for (; len > 0; --len)
			{
				*dst0 = c; dst0++;
				*dst1 = c; dst1++;
			}
		}
		else
		{
			for (; len > 0; --len)
			{
				*dst0 = c; dst0++;
			}
		}
	}
	else if (y0 >= this->clipping.y && y0 < this->clipping.h)
	{
		dst0 = this->data + i + y1 * this->w;
		for (; len > 0; --len)
		{
			*dst0 = c; dst0++;
		}
	}

	/* vertical */
	i   = (y0 >= this->clipping.y) ? y0 : this->clipping.y;
	len = (y1 < this->clipping.h) ? y1 - i : this->clipping.h - i;
	
	if (x0 >= this->clipping.x && x0 < this->clipping.w)
	{
		dst0 = this->data + x0 + i * this->w;
		if (x1 >= this->clipping.x && x1 < this->clipping.w)
		{
			dst1 = this->data + x1 + i * this->w;
			for (; len > 0; --len)
			{
				*dst0 = c; dst0 += this->w;
				*dst1 = c; dst1 += this->w;
			}
		}
		else
		{
			for (; len > 0; --len)
			{
				*dst0 = c; dst0 += this->w;
			}
		}
	}
	else if (x1 >= this->clipping.x && x1 < this->clipping.w)
	{
		dst0 = this->data + x1 + i * this->w;
		for (; len > 0; --len)
		{
			*dst0 = c; dst0 += this->w;
		}
	}
	
	if (x1 >= this->clipping.x && y1 >= this->clipping.y &&
		x1 < this->clipping.w && y1 < this->clipping.h)
	{
		putpixel(x1, y1, c);
	}
}

void
Sprite::drawrect_a(float x0, float y0, float x1, float y1, Color c)
{
	float f;
	
	if (x1 < x0)
	{
		f  = x0;
		x0 = x1;
		x1 = f;
	}
	
	if (y1 < y0)
	{
		f  = y0;
		y0 = y1;
		y1 = f;
	}
	
	/* horizontal */
	if (y0 >= this->clipping.y && y0 < this->clipping.h)
	{
		if (y1 >= this->clipping.y && y1 < this->clipping.h)
		{
			for (f = x0; f < x1; ++f)
			{
				putpixel_a(f, y0, c);
				putpixel_a(f, y1, c);
			}
		}
		else
		{
			for (f = x0; f < x1; ++f)
			{
				putpixel_a(f, y0, c);
			}
		}
	}
	else if (y1 >= this->clipping.y && y1 < this->clipping.h)
	{
		for (f = x0; f <= x1; ++f)
		{
			putpixel_a(f, y1, c);
		}
	}
	
	/* vertical */
	if (x0 >= this->clipping.x && x0 < this->clipping.w)
	{
		if (x1 >= this->clipping.x && x1 < this->clipping.w)
		{
			for (f = y0; f < y1; ++f)
			{
				putpixel_a(x0, f, c);
				putpixel_a(x1, f, c);
			}
		}
		else
		{
			for (f = y0; f < y1; ++f)
			{
				putpixel_a(x0, f, c);
			}
		}
	}
	else if (x1 >= this->clipping.x && x1 < this->clipping.w)
	{
		for (f = y0; f <= y1; ++f)
		{
			putpixel_a(x1, f, c);
		}
	}

	if (x1 >= this->clipping.x && y1 >= this->clipping.y &&
		x1 < this->clipping.w && y1 < this->clipping.h)
	{
		putpixel_a(x1, y1, c);
	}
}

void
Sprite::fillcircle(int x, int y, int radius, Color c)
{
	int w, h;
	Color *upper, *lower;
	Color *dst, *dst_end;
	Color *min, *max, *min_x, *max_x;
	
	min = this->data + this->clipping.y * this->w;
	max = min + this->clipping.h * this->w;
	
	min_x = this->data + this->clipping.x + y * this->w;
	max_x = min_x + this->clipping.w;
	
	upper = x + y * this->w + this->data;
	lower = upper + (radius + 1) * this->w;
	upper -= radius * this->w;
	
	h = radius;
	radius *= radius;
	for (; h >= 0; --h)
	{
		w = sqrt(radius - h * h);
		
		if (upper >= min && upper < max)
		{
			dst = (x - w >= this->clipping.x)
				? upper - w
				: min_x - h * this->w;
			
			dst_end = (x + w <= this->clipping.x + this->clipping.w)
				? upper + w
				: max_x - h * this->w;
			
			for (; dst < dst_end; ++dst)
			{
				*dst = c;
			}
		}
		upper += this->w;
		
		if (lower >= min && lower < max)
		{
			dst = (x - w >= this->clipping.x)
				? lower - w
				: min_x + (h + 1) * this->w;
			
			dst_end = (x + w <= this->clipping.x + this->clipping.w)
				? lower + w
				: max_x + (h + 1) * this->w;
			
			for (; dst < dst_end; ++dst)
			{
				*dst = c;
			}
		}
		lower -= this->w;
	}
}

void
Sprite::fillcircle_a(float x, float y, float radius, Color c)
{
	this->fillcircle(x + .5f, y + .5f, radius + .5f, c);
	this->drawcircle_a(x, y, radius, c);
}

void
Sprite::fillcircle(int x, int y, int radius, Color c, float opacity)
{
	throw 666;
}

void
Sprite::fillrect(int x0, int y0, int x1, int y1, Color c)
{
	int w, h;
	Color *base, *dst;
	
	if (x1 < x0)
	{
		w  = x0;
		x0 = x1;
		x1 = w;
	}
	
	if (y1 < y0)
	{
		h  = y0;
		y0 = y1;
		y1 = h;
	}
	
	if (x0 < this->clipping.x)       x0 = this->clipping.x;
	else if (x1 >= this->clipping.w) x1 = this->clipping.w;
	
	if (y0 < this->clipping.y)       y0 = this->clipping.y;
	else if (y1 >= this->clipping.h) y1 = this->clipping.h;
	
	w = x1 - x0;
	h = y1 - y0;
	
	base = this->data + x0 + y0 * this->w;
	
	for (; h >= 0; --h)
	{
		for (dst = base + w; dst >= base; --dst)
		{
			*dst = c;
		}
		
		base += this->w;
	}
}

void
Sprite::fillrect_a(float x0, float y0, float x1, float y1, Color c)
{
	this->fillrect(x0 + 1, y0 + 1, x1, y1, c);
	this->drawrect_a(x0, y0, x1, y1, c);
}

void
Sprite::fillrect(int x0, int y0, int x1, int y1, Color c, float opacity)
{
	int w, h;
	Color *base, *dst;
	
	if (x1 < x0)
	{
		w  = x0;
		x0 = x1;
		x1 = w;
	}
	
	if (y1 < y0)
	{
		h  = y0;
		y0 = y1;
		y1 = h;
	}
	
	if (x0 < this->clipping.x)       x0 = this->clipping.x;
	else if (x1 >= this->clipping.w) x1 = this->clipping.w;
	
	if (y0 < this->clipping.y)       y0 = this->clipping.y;
	else if (y1 >= this->clipping.h) y1 = this->clipping.h;
	
	w = x1 - x0;
	h = y1 - y0;
	
	base = this->data + x0 + y0 * this->w;
	
	for (; h >= 0; --h)
	{
		for (dst = base + w; dst >= base; --dst)
		{
			_putpixel((Component *)dst, c, opacity);
		}
		
		base += this->w;
	}
}

void
Sprite::floodfill(int x, int y, Color c)
{
	Color  **flood_stack      = NULL;
	size_t   flood_stack_size = 0;

	int up, down;
	Color *dst, *end, *test, **stack;
	Color match_color, write_color;
	
	/* make sure the stack is large enough */
	size_t size = this->w * this->h;
	if (size > flood_stack_size)
	{
		delete[] flood_stack;
		flood_stack = new Color *[size];
		flood_stack_size = size;
	}
	
	write_color = 0xff000000 | c;
	match_color = *(this->data + x + y * this->w);
	
	/* push (x, y) to the stack */
	*flood_stack = this->data + x + y * this->w;
	
	for (stack = flood_stack; stack >= flood_stack; --stack)
	{
		up = *stack - this->data;
		x = up % this->w;
		
		end = *stack + 1;
		for (y = x + 1; *end == match_color && y < this->w; ++y)
		{
			end++;
		}
		
		for (dst = *stack; *dst == match_color && x >= 0; --x)
		{
			dst--;
		}
		
		if (up < this->w)
		/* top row (only test downwards) */
		{
			up = 0;
			down = 0;
			for (dst++; dst < end; ++dst)
			{
				*dst = write_color;
				
				test = dst + this->w;
				if (*test == match_color)
				{
					if (down == 0)
					{
						*(stack++) = test;
						down = 1;
					}
				}
				else if (down)
				{
					down = 0;
				}
			}
		}
		else if (up >= (this->h - 1) * this->w)
		/* bottom row (only test upwards) */
		{
			up = 0;
			down = 0;
			for (dst++; dst < end; ++dst)
			{
				*dst = write_color;
				
				test = dst + this->w;
				if (*test == match_color)
				{
					if (down == 0)
					{
						*(stack++) = test;
						down = 1;
					}
				}
				else if (down)
				{
					down = 0;
				}
			}
		}
		else
		/* bulk data, test both up and down */
		{
			up = 0;
			down = 0;
			for (dst++; dst < end; ++dst)
			{
				*dst = write_color;
				
				test = dst - this->w;
				if (*test == match_color)
				{
					if (up == 0)
					{
						*(stack++) = test;
						up = 1;
					}
				}
				else if (up)
				{
					up = 0;
				}
				
				test = dst + this->w;
				if (*test == match_color)
				{
					if (down == 0)
					{
						*(stack++) = test;
						down = 1;
					}
				}
				else if (down)
				{
					down = 0;
				}
			}
		}
	}

	delete[] flood_stack;
}


/*
________________________________________________________________________________

	C O M P L E X   G E O M E T R Y
________________________________________________________________________________

*/

void
Sprite::drawgrid(
	int center_x, int center_y,
	float pan_x, float pan_y,
	int spacing, float zoom,
	Color color, float opacity,
	bool horizontal_lines, bool vertical_lines)
{
	struct {
		int x, y;
		int w, h;
	} rect;
	float x, y, scale;
	int i, base, i_start;
	
	rect.x = rect.y = 0;
	rect.w = this->w - 1;
	rect.h = this->h - 1;
	
	if (spacing == 0)
	{
	/* draw origo only */
		pan_x = rect.x + center_x - pan_x * zoom;
		pan_y = rect.y + center_y - pan_y * zoom;
		if (vertical_lines && pan_x >= rect.x && pan_x < rect.x + rect.w)
		{
			for (y = rect.y + rect.h - 1; y >= rect.y; --y)
			{
				this->putpixel(pan_x, y, color, opacity);
			}
		}
		if (horizontal_lines && pan_y >= rect.y && pan_y < rect.y + rect.h)
		{
			for (x = rect.x + rect.w - 1; x >= rect.x; --x)
			{
				this->putpixel(x, pan_y, color, opacity);
			}
		}
		return;
	}
	
	scale = zoom * spacing;
	
	y = pan_y * zoom;
	base = rect.y + center_y - y;
	i_start = (y - center_y) / scale;
	if (horizontal_lines)
	{
		for (i = i_start + rect.h / scale; i >= i_start; --i)
		{
			y = base + i * scale;
			if (y >= rect.y && y <= rect.y + rect.h - 1)
			{
				for (x = rect.x + rect.w - 1; x >= rect.x; --x)
				{
					this->putpixel(x, y, color, opacity);
				}
			}
		}
	}
	
	x = pan_x * zoom;
	base = rect.x + center_x - x;
	i_start = (x - center_x) / scale;
	if (vertical_lines)
	{
		for (i = i_start + rect.w / scale; i >= i_start; --i)
		{
			x = base + i * scale;
			if (x >= rect.x && x <= rect.x + rect.w - 1)
			{
				for (y = rect.y + rect.h - 1; y >= rect.y; --y)
				{
					this->putpixel(x, y, color, opacity);
				}
			}
		}
	}
}

void
Sprite::drawring(
    int value, int sectors,
    int x, int y, float radius,
    float thickness, float gap,
    Color less_than_value, Color greater_than_value, Color background,
    bool round_corners)
{
	int center_x, center_y;
	int x0, y0, x1, y1;
	float y_dist, dist;
	float angle, angle_threshold;
	float opacity, f;
	Color color;
	
	center_x = x;
	center_y = y;
	
	x0 = x - radius;
	y0 = y - radius;
	x1 = x + radius;
	y1 = y + radius;
	
	if (this->clipping.enabled)
	{
		x = this->clipping.x + this->clipping.w - 1;
		y = this->clipping.y + this->clipping.h - 1;
		
		if (x0 < 0) x0 = 0;
		if (x1 > x) x1 = x;
		
		if (y0 < 0) y0 = 0;
		if (y1 > y) y1 = y;
		
		if (x1 < 0 || y1 < 0 || x0 > x || y0 > y)
			return;
	}
	
	thickness /= 2;
	radius    -= thickness;
	
	if (sectors > 0)
	{
		/* range from 0.0f to 1.0f */
		angle_threshold = (value < sectors)
			? (float)value / sectors
			: 2 - 4 * (value == 0); /* <- over 1 or less than 0 */
	}
	else
	{
		/* 0 sectors => draw either fully ON or fully OFF */
		sectors = 1;
		angle_threshold = -1 + ((value != 0) << 4);
	}
	
	if (gap <= 0.00)
	/* Rings without sector gaps */
	{
		for (y = y0; y <= y1; ++y)
		{
			y_dist = y - center_y;
			for (x = x0; x <= x1; ++x)
			{
				/* Distance from and angle to center */
				dist  = x - center_x;
				angle = .5f + atan2(dist, y_dist) / 6.283185307f;
				dist  = sqrt(dist * dist + y_dist * y_dist);
				
				/* Ring thickness */
				opacity = thickness - fabs(dist - radius);
				if (!round_corners)
				{
					if (opacity < 0.0) opacity = 0.0f;
					else if (opacity > 1.0f) opacity = 1.0f;
				}
				
				/* Smooth threshold transition */
				f = (angle - angle_threshold) * dist * 5.0f;
				if (f < 0.0) f = 0.0f;
				else if (f > 1.0f) f = 1.0f;
			
				color = gfx::blend::ratio(less_than_value, greater_than_value, f);
				
				if (round_corners)
				{
					if (opacity < 0.0) opacity = 0.0f;
					else if (opacity > 1.0f) opacity = 1.0f;
				}
				
				this->putpixel(x, y,
					gfx::blend::ratio(background, color, opacity));
			}
		}
	}
	else
	/* Rings with gaps between sectors */
	{
		gap /= 2;
		for (y = y0; y <= y1; ++y)
		{
			y_dist = y - center_y;
			for (x = x0; x <= x1; ++x)
			{
				/* Distance from and angle to center */
				dist  = x - center_x;
				angle = .5f + atan2(dist, y_dist) / 6.283185307f;
				dist  = sqrt(dist * dist + y_dist * y_dist);
				
				/* Ring thickness */
				opacity = thickness - fabs(dist - radius);
				if (!round_corners)
				{
					if (opacity < 0.0) opacity = 0.0f;
					else if (opacity > 1.0f) opacity = 1.0f;
				}
				
				/* Sector gap */
				value = (int)(angle * sectors);
				f = (float)value / sectors;
				f = (fabs(angle - f) - gap) * dist * 2.5f;
				if (f < 0.0) f = 0.0f;
				else if (f > 1.0f) f = 1.0f;
				opacity *= f;
				
				/* Color based on threshold */
				color = (angle >= angle_threshold)
					? greater_than_value
					: less_than_value;
				
				if (round_corners)
				{
					if (opacity < 0.0) opacity = 0.0f;
					else if (opacity > 1.0f) opacity = 1.0f;
				}
				
				this->putpixel(x, y,
					gfx::blend::ratio(background, color, opacity));
			}
		}
	}
}

#include "sprite_io.cpp"

/* Fuck this was long */
