#include "string.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdarg>

static unsigned char _lcase_table[0xff + 1] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
    'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

static unsigned char _ucase_table[0xff + 1] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
    'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

static bool _number_table[0xff + 1] = {
/*       00, 01, 02, 03, 04, 05, 06, 07, 08, 09, 0a, 0b, 0c, 0d, 0e, 0f, */
/* 00 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 10 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 20 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  1,  1,  0,
/* 30 */ 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,
/* 40 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 50 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 60 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 70 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 80 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 90 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* a0 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* b0 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* c0 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* d0 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* e0 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* f0 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

char *
core::str::empty(void)
{
    char *s = new char[1];
    s[0] = '\0';
    
    return s;
}

char *
core::str::empty(char *&s)
{
    delete[] s;
    s = new char[1];
    s[0] = '\0';
    
    return s;
}

char *
core::str::set(char *&s, char *value)
{
    if (s != value)
    {
        delete[] s;
        s = value;
    }
    
    return s;
}

char *
core::str::dup(const char *s)
{
    if (s == NULL)
    {
        s = "";
    }
    
    size_t  len = strlen(s) + 1;
    char   *dup = new char[len];
    
    return (char *)memcpy(dup, s, len);
}

char *
core::str::dupn(const char *s, int n)
{
    if (s == NULL)
    {
        s = "";
    }
    
    size_t len = strlen(s);
    if ((int)len >= n)
    {
        len = n * (n > 0);
    }

    char *dup = new char[len + 1];
    dup[len] = '\0';
    
    return (char *)memcpy(dup, s, len);
}

char *
core::str::format(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char *dup = core::str::vformat(format, args);
    va_end(args);
    
    return dup;
}

char *
core::str::vformat(const char *format, va_list args)
{
    char buf[128];
    
    size_t dup_len = 0, format_len = strlen(format) + 1;
    char  *dup = new char[format_len];
    dup[0] = '\0';
    
    char *cat;
    
    char c;
    for (const char *p = format;; ++p)
    {
        c = *p;
        if (c == '%')
        {
            c = *(++p);
            switch (c)
            {
                case 's':
                    cat = va_arg(args, char *);
                    break;
                    
                case 'c':
                    sprintf(buf, "%c", va_arg(args, int));
                    cat = buf;
                    break;
                
                case 'f':
                    sprintf(buf, "%f", va_arg(args, double));
                    cat = buf;
                    break;
                
                case 'g':
                    sprintf(buf, "%g", va_arg(args, double));
                    cat = buf;
                    break;

                case 'd':
                case 'i':
                    sprintf(buf, "%i", va_arg(args, int));
                    cat = buf;
                    break;
                    
                case 'x':
                    sprintf(buf, "%x", va_arg(args, unsigned int));
                    cat = buf;
                    break;
                
                default:
                    cat = NULL;
                    dup[dup_len++] = c;
                    format_len--;
            }
            
            if (cat != NULL)
            {
                size_t len = strlen(cat);
                char *temp = new char[dup_len + len + format_len];
                memcpy(temp, dup, dup_len);
                memcpy(temp + dup_len, cat, len);
                delete[] dup;
                
                dup_len += len;
                dup      = temp;
            }

        }
        else
        {
            dup[dup_len++] = c;
            format_len--;
        }
        
        
        if (c == '\0')
        {
            break;
        }
    }
    
    return dup;
}

char *
core::str::cat(char c1, char c2)
{
    char *dup = new char[3];
    dup[0] = c1;
    dup[1] = c2;
    dup[2] = '\0';
    
    return dup;
}

char *
core::str::cat(char c1, const char *s2)
{
    if (s2 == NULL)
    {
        s2 = "";
    }
    
    size_t  len = strlen(s2) + 1;
    char   *dup = new char[len + 1];
    
    dup[0] = c1;
    memcpy(dup + 1, s2, len);
    
    return dup;
}

char *
core::str::cat(const char *s1, char c2)
{
    if (s1 == NULL)
    {
        s1 = "";
    }
    
    size_t  len = strlen(s1) + 1;
    char   *dup = new char[len + 1];
    
    memcpy(dup, s1, len);
    dup[len - 1] = c2;
    dup[len]     = '\0';

    return dup;
}

char *
core::str::cat(const char *s1, const char *s2)
{
    if (s1 == NULL)
    {
        s1 = "";
    }
    
    if (s2 == NULL)
    {
        s2 = "";
    }
    
    size_t  len1 = strlen(s1);
    size_t  len2 = strlen(s2) + 1;
    char   *dup  = new char[len1 + len2];
    
    memcpy(dup + len1, s2, len2);
    return (char *)memcpy(dup, s1, len1);
}

char *
core::str::insert(const char *base, const char *s, unsigned int position, unsigned int length)
{
    if (base == NULL)
    {
        base = "";
    }
    
    if (s == NULL)
    {
        s = "";
    }
    
    size_t  base_len = strlen(base);
    size_t  s_len    = strlen(s);
    char   *result   = new char[base_len + s_len + 1];
    
    memcpy(result, base, position);
    memcpy(result + position, s, s_len);
    
    s_len    += position;
    position += length;
    if (position > base_len)
    {
        position = base_len;
    }
    
    memcpy(result + s_len, base + position, base_len - position + 1);
    
    return result;
}

char *
core::str::cut(const char *s, unsigned int position, unsigned int length)
{
    return insert(s, NULL, position, length);
}

char *
core::str::substring(const char *start, const char *end)
{
    size_t len = end - start + 1;
    char *s = new char[len + 1];
    
    memcpy(s, start, len);
    s[len] = '\0';
    
    return s;
}

char *
core::str::ltrim(char *s)
{
    char *src;
    for (src = s; *src != '\0' && *src <= ' '; ++src);
    for (char *dst = s; (*(dst++) = *src) != '\0'; ++src);

    return s;
}

char *
core::str::rtrim(char *s)
{
    char c, *last = s;
    for (char *i = s; (c = *i) != '\0';)
    {
        i++;
        if (c > ' ')
        {
            last = i;
        }
    }
    
    *last = '\0';

    return s;
}

char *
core::str::trim(char *s)
{
    char *src;
    for (src = s; *src != '\0' && *src <= ' '; ++src);

    char c, *last = s;
    for (char *dst = s; (c = *dst = *src) != '\0'; ++src)
    {
        dst++;
        if (c > ' ')
        {
            last = dst;
        }
    }

    *last = '\0';

    return s;
}

char *
core::str::ltrim(char *s, const char *match)
{
    size_t len = strlen(match);
    if (strncmp(s, match, len) == 0)
    {
        for (char *src = s + len, *dst = s; (*dst = *src) != '\0'; ++src, ++dst);
    }

    return s;
}

char *
core::str::rtrim(char *s, const char *match)
{
    size_t s_len     = strlen(s);
    size_t match_len = strlen(match);

    if (match_len > s_len)
    {
        return s;
    }

    char *dst = s + s_len - match_len;
    if (strncmp(dst, match, match_len) == 0)
    {
        *dst = '\0';
    }

    return s;
}

char *
core::str::strip(char *s, char c)
{
    for (char *src = s, *dst = s; (*dst = *src) != '\0'; ++src)
    {
        dst += (*dst != c);
    }

    return s;
}

char *
core::str::strip(char *s, const char *c_list)
{
    for (char *src = s, *dst = s; (*dst = *src) != '\0'; ++src, ++dst)
    {
        for (const char *c = c_list; *c != '\0'; ++c)
        {
            if (*dst == *c)
            {
                dst--;
                break;
            }
        }
    }

    return s;
}

char *
core::str::strip_non_visible(char *s)
{
    for (char *src = s, *dst = s; (*dst = *src) != '\0'; ++src)
    {
        dst += (*dst >= ' ' && *dst < 0x80);
    }

    return s;
}

char *
core::str::strip_duplicate(char *s, char c)
{
    for (char *src = s, *dst = s;; ++src, ++dst)
    {
        *dst = *src;

        if (*src == c)
        {
            while (src[1] == c)
            {
                src++;
            }
        }
        
        if (*src == '\0')
        {
            break;
        }
    }
    
    return s;
}

char *
core::str::replace(char *s, char match, char replacement)
{
    for (char *c = s; *c != '\0'; ++c)
    {
        if (*c == match)
        {
            *c = replacement;
        }
    }
    
    return s;
}

char *
core::str::replace(char *s, char match, char replacement, int max_replacements)
{
    for (char *c = s; max_replacements > 0 && *c != '\0'; ++c, max_replacements--)
    {
        if (*c == match)
        {
            *c = replacement;
        }
    }
    
    return s;
}

char *
core::str::replace(const char *s, const char *match, const char *replacement)
{
    char *pos = strstr(s, match);
    if (pos == NULL)
    {
        return str::dup(s);
    }

    if (replacement == NULL)
    {
        replacement = "";
    }

    size_t s_len           = strlen(s);
    size_t match_len       = strlen(match);
    size_t replacement_len = strlen(replacement);
    size_t start           = pos - s;

    char *dup = new char[s_len - match_len + replacement_len + 1];

    memcpy(dup, s, start);
    memcpy(dup + start, replacement, replacement_len);
    memcpy(dup + start + replacement_len, pos + match_len, s_len - start - match_len + 1);
    
    return dup;
}

char *
core::str::replace(const char *s, const char *match, const char *replacement, int max_replacements)
{
    if (replacement == NULL)
    {
        replacement = "";
    }

    size_t s_len           = strlen(s);
    size_t match_len       = strlen(match);
    size_t replacement_len = strlen(replacement);

    const char *src;
    char       *dst;
    char       *dup = new char[s_len + 1];

    for (src = s, dst = dup; (*dst = *src) != '\0'; ++src, ++dst)
    {
        if (strncmp(src, match, match_len) == 0)
        {
            size_t offset = dst - dup;

            if (max_replacements--)
            {
                char *p = new char[s_len + replacement_len - match_len + offset + 1];

                memcpy(p, dup, offset);
                memcpy(p + offset, replacement, replacement_len);
                
                delete[] dup;
                dup  = p;
                dst  = p + offset + replacement_len - 1;
                src += match_len - 1;
            }
            else
            {
                memcpy(dst, src, s_len - (src - s) + 1);
                break;
            }
        }
    }

    return dup;

}

char *
core::str::dup_escaped(const char *s)
{
    const char *i;
    char        c;
    size_t      len = 0;
    
    for (i = s; (c = *i) != '\0'; ++i)
    {
        len += 1 + (c == '\t' || c == '\n' || c == '"');
    }
    
    char *result = new char[len + 3];
    char *p = result;
    
    *(p++) = '"';
    
    for (i = s; (c = *i) != '\0'; ++i)
    {
        switch (c)
        {
            case '\t':
                *(p++) = '\\';
                *(p++) = 't';
                break;
            
            case '\n':
                *(p++) = '\\';
                *(p++) = 'n';
                break;

            case '"':
                *(p++) = '\\';
                *(p++) = '"';
                break;

            default:
                *(p++) = c;
        }
    }
    
    *(p++) = '"';
    *p     = '\0';

    return result;
}

char *
core::str::unescape(char *s)
{
    if (s[0] != '"')
    {
        core::str::ltrim(s);
    }
    
    if (s[0] == '"')
    {
        char *dst = s;
        for (char *src = s + 1;; ++src)
        {
            if (*src == '\0' || *src == '"')
            {
                break;
            }
            else if (*src == '\\')
            {
                switch (*(++src))
                {
                    case '\0':
                        break;
                    
                    case 't':
                        *(dst++) = '\t';
                        break;
                    
                    case 'n':
                        *(dst++) = '\n';
                        break;
                    
                    default:
                        *(dst++) = *src;
                }
            }
            else
            {
                *(dst++) = *src;
            }
        }
        *dst = '\0';
    }
    
    return s;
}

char
core::str::ucase(char c)
{
    return (c >= 'a' && c <= 'z')
        ? c - ('a' - 'A')
        : c;
}

char
core::str::lcase(char c)
{
    return (c >= 'A' && c <= 'Z')
        ? c + ('a' - 'A')
        : c;
}

char *
core::str::ucase(char *s)
{
    char *i;
    for (i = s; (*i = _ucase_table[(int)*i]) != '\0'; ++i);
    
    return s;
}

char *
core::str::lcase(char *s)
{
    char *i;
    for (i = s; (*i = _lcase_table[(int)*i]) != '\0'; ++i);
    
    return s;
}

bool
core::str::is_numeric(char c)
{
    return _number_table[(int)c];
}

bool
core::str::is_numeric(const char *s)
{
    int c = *s;

    if (!core::str::is_numeric(c))
    {
        return false;
    }
    
    bool decimal = (c == '.');
    for (s++; (c = *s) != '\0'; ++s)
    {
        if (c == '.')
        {
            if (decimal)
            {
                return false;
            }
            decimal = true;
        }
        else if (c < '0' || c > '9')
        {
            return false;
        }
    }
    
    return true;
}

char *
core::str::decode_html(const char *s)
{
    char *result = core::str::dup(s);

    const char    *match[5]      = {"&amp;", "&auml;", "&ouml;", "&Auml;", "&Ouml;"};
    unsigned char  ascii_code[5] = {'&',     132,      148,      142,      153};
    char           replace[2]    = "x";

    for (int i = 0; i < 5; ++i)
    {
        replace[0] = ascii_code[i];
        core::str::set(result,
            core::str::replace(result, match[i], replace, 999));
    }

    return result;
}

char
core::str::first(const char *s)
{
    return s == NULL ? '\0' : s[0];
}

char
core::str::last(const char *s)
{
    if (s == NULL || s[0] == '\0')
    {
        return '\0';
    }

    for (; s[1] != '\0'; ++s);

    return *s;
}

const char *
core::str::last(const char *s, int n)
{
    if (s == NULL || s[0] == '\0')
    {
        return s;
    }

    const char *p = s + strlen(s) - n;
    return p < s ? s : p;
}

int
core::str::len(const char *s)
{
    const char *c;
    for (c = s; *c != '\0'; ++c);
    return c - s;
}

int
core::str::cmp(const char *s1, const char *s2)
{
    for (; *s1 == *s2 && *s1 != '\0'; ++s1, ++s2);
    return *s2 - *s1;
}

int
core::str::ncmp(const char *s1, const char *s2, int n)
{
    for (; --n > 0 && *s1 == *s2 && *s1 != '\0'; ++s1, ++s2);
    return *s2 - *s1;
}

int
core::str::icmp(const char *s1, const char *s2)
{
    for (;; ++s1, ++s2)
    {
        char
            c1 = _lcase_table[(int)*s1],
            c2 = _lcase_table[(int)*s2];
        
        if (c1 != c2 || c1 == '\0')
        {
            return c2 - c1;
        }
    }
}
        
int
core::str::nicmp(const char *s1, const char *s2, int n)
{
    for (; --n > 0; ++s1, ++s2)
    {
        char
            c1 = _lcase_table[(int)*s1],
            c2 = _lcase_table[(int)*s2];
        
        if (c1 != c2 || c1 == '\0')
        {
            break;
        }
    }

   return *s2 - *s1;
}

int
core::str::count(const char *s, const char *match)
{
    int result = 0;
    size_t len = strlen(match);

    for (; *s != '\0'; result += (strncmp(s++, match, len) == 0));
    return result;
}

int
core::str::icount(const char *s, const char *match)
{
    int result = 0;
    size_t len = strlen(match);

    for (; *s != '\0'; result += (core::str::nicmp(s++, match, len) == 0));
    return result;
}

char *
core::str::find(const char *haystack, const char *needle)
{
    size_t len = strlen(needle);

    for (; *haystack != '\0' && strncmp(haystack, needle, len); ++haystack);
    return (*haystack != '\0')
        ? (char *)haystack
        : NULL;
}

char *
core::str::ifind(const char *haystack, const char *needle)
{
    size_t len = strlen(needle);

    for (; *haystack != '\0'
        && core::str::nicmp(haystack, needle, len); ++haystack);
    return (*haystack != '\0')
        ? (char *)haystack
        : NULL;
}

char *
core::str::find(const char *haystack, char needle)
{
    for (; *haystack != needle && *haystack != '\0'; ++haystack);
    return (*haystack != '\0')
        ? (char *)haystack
        : NULL;
}

char *
core::str::ifind(const char *haystack, char needle)
{
    needle = _lcase_table[(int)needle];
    for (; _lcase_table[(int)*haystack] != needle
        && *haystack != '\0'; ++haystack);
    return (*haystack != '\0')
        ? (char *)haystack
        : NULL;
}

bool
core::str::contains(const char *haystack, const char *needle)
{
    return find(haystack, needle) != NULL;
}

bool
core::str::begins(const char *haystack, const char *needle)
{
    return ncmp(haystack, needle, strlen(needle)) == 0;
}

bool
core::str::ends(const char *haystack, const char *needle)
{
    return cmp(haystack + strlen(haystack) - strlen(needle), needle) == 0;
}

bool
core::str::icontains(const char *haystack, const char *needle)
{
    return ifind(haystack, needle) != NULL;
}

bool
core::str::ibegins(const char *haystack, const char *needle)
{
    return nicmp(haystack, needle, strlen(needle)) == 0;
}

bool
core::str::iends(const char *haystack, const char *needle)
{
    return icmp(haystack + strlen(haystack) - strlen(needle), needle) == 0;
}

core::Config *
core::str::split(const char *s, char delimiter, bool include_empty)
{
    core::Config *list = new core::Config();

    char *token    = NULL;
    char threshold = (delimiter) ? '\0' : ' ';
    
    const char *start = s;
    
    for (;; ++s)
    {
        char c = *s;
        if (c == delimiter || c <= threshold)
        {
            core::str::set(token, core::str::substring(start, s - 1));
            (*list)[core::Config::APPEND] = token;
            
            if (c == '\0')
            {
                break;
            }

            if (include_empty)
            {
                start = s + 1;
            }
            else
            {
                for (s++; *s == delimiter; ++s);
                start = s;
            }
        }
    }
    
    delete[] token;
    return list;
}

char *
core::str::join(const core::Config *list, char delimiter, bool include_empty)
{
    char buf[2] = { delimiter, '\0' };
    return core::str::join(list, buf, include_empty);
}

char *
core::str::join(const core::Config *list, const char *delimiter, bool include_empty)
{
    char *s = NULL;
    
    for (core::Config::Value::const_iterator i = list->begin();
    i != list->end(); ++i)
    {
        const char *token = i->second->string();
        
        if (include_empty == false && strlen(token) == 0)
        {
            continue;
        }

        if (s != NULL && delimiter != NULL)
        {
            core::str::set(s, core::str::cat(s, delimiter));
        }

        core::str::set(s, core::str::cat(s, token));
    }
    
    return s;
}

core::Config *
core::str::split_unescaped(const char *s, char delimiter)
{
    core::Config *list = new core::Config();

    char *token    = NULL;
    char threshold = (delimiter) ? '\0' : ' ';
    
    const char *start = s, *end = s - 1;
    
    for (;; ++s)
    {
        char c = *s;

        if (c == '"')
        {
            start = s;
            for (s++; (c = *s) != '\0'; ++s)
            {
                if (c == '\\' && s[1] != '\0')
                {
                    s++;
                    continue;
                }
                
                if (c == '"')
                {
                    end = s - 1;
                    for (; (c = *s) != delimiter && c > threshold; ++s);

                    break;
                }
            }
        }
        
        if (c == delimiter || c <= threshold)
        {
            if (end < start)
            {
                end = s - 1;
            }
            
            core::str::set(token, core::str::substring(start, end));
            (*list)[core::Config::APPEND] = core::str::unescape(token);
            
            if (c == '\0')
            {
                break;
            }

            start = s + 1;
        }
    }
    
    delete[] token;
    return list;
}

