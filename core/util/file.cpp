#include "file.h"
#include "string.h"
// #include "../engine.h"

#include <cstdio>
#include <cstring>
#include <vector>

using namespace core;

typedef
    std::vector<char *>
    DirData;

Dir::Dir():
    path(mutable_path),
    ext(mutable_ext),
    flags(mutable_flags)
{
    this->mutable_path  = core::str::dup(".");
    this->mutable_ext   = core::str::dup("");
    this->mutable_flags = ALL;
    this->data          = new DirData();

    this->changed       = true;
}

Dir::Dir(const char *path, unsigned int flags, const char *extension):
    path(mutable_path),
    ext(mutable_ext),
    flags(mutable_flags)
{
    this->mutable_path  = NULL;
    this->mutable_ext   = NULL;
    this->mutable_flags = flags;
    this->data          = new DirData();

    this->filter(path, extension);
}

Dir::~Dir()
{
    for (DirData::iterator i = (*(DirData *)this->data).begin();
        i != (*(DirData *)this->data).end();
        ++i)
    {
        delete[] *i;
    }
    delete (DirData *)this->data;
    
    delete[] this->path;
    delete[] this->ext;
}

#include <windows.h>

void
Dir::refresh(void)
{
    this->changed      = false;

    this->files        = 0;
    this->hidden_files = 0;
    this->directories  = 0;

    char *s, *next;
    
    char buf[512];
    
    DWORD attributes, exclusions;
    WIN32_FIND_DATA win32_data;
    HANDLE find_handle;
    
    // flush old data
    for (DirData::iterator i = (*(DirData *)this->data).begin();
        i != (*(DirData *)this->data).end();
        ++i)
    {
        delete[] *i;
    }
    (*(DirData *)this->data).clear();

    if (this->ext[0] != '\0')
    {
        sprintf(buf, "%s*%s", this->path, this->ext);
    }
    else
    {
        sprintf(buf, "%s*", this->path);
    }
    
    find_handle = FindFirstFile(buf, &win32_data);
    
    this->is_valid = find_handle != INVALID_HANDLE_VALUE;
    
    if (this->is_valid)
    {
        attributes = 0;
        
        if (this->flags & FILES)
        {
            attributes |= ~FILE_ATTRIBUTE_DIRECTORY;
        }
        
        if (this->flags & DIRECTORIES)
        {
            attributes |= FILE_ATTRIBUTE_DIRECTORY;
        }
        
        exclusions = FILE_ATTRIBUTE_SYSTEM;
        
        if (this->flags & EXCLUDE_HIDDEN)
        {
            exclusions |= FILE_ATTRIBUTE_HIDDEN;
        }
    
        do
        {
            if (win32_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                this->directories++;
            }
            else
            {
                this->files++;
                
                if (win32_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
                {
                    this->hidden_files++;
                }
            }
            
            if ((win32_data.dwFileAttributes & attributes)
            && (win32_data.dwFileAttributes & exclusions) == 0)
            {
                s = win32_data.cFileName;
                
                /* ignore "." and ".." */
                if (s[0] != '.' || (s[1] != '\0' && (s[1] != '.' || s[2] != '\0')))
                {
                    if (this->flags & OMIT_EXTENSION)
                    {
                        /* find the last '.' and replace it with '\0' */
                        strcpy(buf, s);
                        
                        for (s = buf; (next = strchr(s, '.')) != NULL;
                            s = next + 1);
                        
                        if (s != buf)
                        {
                            s[-1] = '\0';
                        }
                        
                        s = buf;
                    }

                    if (this->flags & FULL_PATH)
                    {
                        s = core::str::cat(this->path, s);
                    }
                    else
                    {
                        s = core::str::dup(s);
                    }
                    (*(DirData *)this->data).push_back(s);
                }
            }
        }
        while (FindNextFile(find_handle, &win32_data));

        FindClose(find_handle);
    }
    // else if (GetLastError() != ERROR_FILE_NOT_FOUND)
    // {
    // }
}

const char *
Dir::operator[](int index)
{
    if (this->changed)
    {
        this->refresh();
    }
    
    if (index < 0 || index >= (int)((DirData *)this->data)->size())
    {
        return NULL;
    }
    
    return (*(DirData *)this->data)[(unsigned int)index];
}

int
Dir::count()
{
    return this->count(this->flags);
}

int
Dir::count(unsigned int flags)
{
    if (this->changed)
    {
        this->refresh();
    }
    
    return
        + this->files * (bool)(flags & FILES)
        + this->directories * (bool)(flags & DIRECTORIES)
        - this->hidden_files * (bool)(flags & EXCLUDE_HIDDEN);
}

void
Dir::filter(const char *path, const char *extension)
{
    delete[] this->mutable_path;
    if (path == NULL)
    {
        this->mutable_path = core::str::dup("");
    }
    else
    {
        char c = path[strlen(path) - 1];

        this->mutable_path = (c == '/' || c == '\\')
            ? core::str::dup(path)
            : core::str::cat(path, "/");
    }
    
    delete[] this->mutable_ext;
    if (extension == NULL)
    {
        this->mutable_ext = core::str::dup("");
    }
    else
    {
        this->mutable_ext = (extension[0] == '.')
            ? core::str::dup(extension)
            : core::str::cat(".", extension);
    }

    this->changed = true;
}

void
Dir::filter(unsigned int flags)
{
    if (this->flags != flags)
    {
        this->mutable_flags = flags;
        this->changed       = true;
    }
}

bool
Dir::exists(void)
{
    if (this->changed)
        this->refresh();
    
    return this->is_valid;
}

#define MODE_UNOPENED 0
#define MODE_READ     1
#define MODE_WRITE    2
#define MODE_APPEND   3

const char *fopen_mode[4] = {
    NULL,  // MODE_UNOPENED
    "rb+", // MODE_READ
    "wb+", // MODE_WRITE
    "ab+"  // MODE_APPEND (automatically turns into MODE_WRITE)
};

File::File(const char *filename):
    filename(mutable_filename),
    ext(mutable_ext),
    line_number(mutable_line_number)
{
    this->set_filename(filename);
    
    this->mode     = MODE_UNOPENED;
    this->data     = NULL;
    this->contents = NULL;
    
    this->mutable_line_number = 0;
}

File::~File()
{
    if (this->data != NULL)
    {
        fclose((FILE *)this->data);
    }

    delete[] this->filename;
	delete[] this->contents;
}

void
File::set_filename(const char *filename)
{
    if (filename == NULL)
    {
        this->mutable_filename = core::str::dup(DATA_DIRECTORY);
        this->mutable_ext      = NULL;
    }
    else
    {
        this->mutable_filename = core::str::cat(DATA_DIRECTORY, filename);
        
        const char *extension, *next;
        for (extension = filename;
            (next = strchr(extension, '.')) != NULL;
            extension = next + 1);

        this->mutable_ext = (extension == filename)
            ? NULL
            : this->filename + strlen(DATA_DIRECTORY) + (extension - filename);
    }
}

void
File::set_mode(int mode)
{
    if (this->mode == mode)
        return;
    
    switch (mode)
    {
        case MODE_UNOPENED:
            if (this->data != NULL)
            {
                fclose((FILE *)this->data);
                this->data = NULL;
            }
            this->mutable_line_number = 0;
            break;
        
        case MODE_READ:
        case MODE_WRITE:
        case MODE_APPEND:
            if (this->data != NULL)
            {
                fclose((FILE *)this->data);
            }
            
            this->data = fopen(this->filename, fopen_mode[mode]);
            if (this->data == NULL)
            {
                this->set_mode(MODE_UNOPENED);
                throw 666;
            }
            
            if (mode == MODE_APPEND)
                mode = MODE_WRITE;

            this->mutable_line_number = 1;
            break;
        
        default:
            throw 666;
    }
    
    this->mode = mode;
}

bool
File::exists(void)
{
    if (this->data != NULL)
        return true;
    
    FILE *file = fopen(this->filename, "rb");
    if (file != NULL)
    {
        fclose(file);
        return true;
    }
    
    return false;
}

void
File::rename(const char *filename)
{
    this->set_mode(MODE_UNOPENED);
    
    char *old_filename = this->filename;
    this->set_filename(filename);
    
    if (::rename(old_filename, this->filename) != 0)
    {
        delete[] this->mutable_filename;
        this->mutable_filename = old_filename;
        throw 666;
    }

    delete[] old_filename;
}

size_t
File::get_size(void)
{
    size_t size;
    if (this->data != NULL)
    {
        size_t pos = ftell((FILE *)this->data);
        
        fseek((FILE *)this->data, 0, SEEK_END);
        size = ftell((FILE *)this->data);
        
        fseek((FILE *)this->data, 0, SEEK_SET);
        size -= ftell((FILE *)this->data);
        
        fseek((FILE *)this->data, pos, SEEK_SET);
    }
    else
    {
        FILE *file = fopen(this->filename, "rb");
        if (file != NULL)
        {
            fseek(file, 0, SEEK_END);
            size = ftell(file);
            
            fseek(file, 0, SEEK_SET);
            size -= ftell(file);
            
            fclose(file);
        }
        else
        {
            size = 0;
        }
    }
    
    return size;
}

size_t
File::read(void *ptr, size_t size, size_t count)
{
    this->set_mode(MODE_READ);
    return fread(ptr, size, count, (FILE *)this->data);
}

size_t
File::write(const void *ptr, size_t size, size_t count)
{
    this->set_mode(MODE_WRITE);
    return fwrite(ptr, size, count, (FILE *)this->data);
}

void *
File::get_FILE(void)
{
    this->set_mode(MODE_WRITE);
    return this->data;
}

void
File::append(void)
{
    this->set_mode(MODE_UNOPENED);
    this->set_mode(MODE_APPEND);
}

void
File::empty(void)
{
    this->set_mode(MODE_UNOPENED);
    this->set_mode(MODE_WRITE);
}

void
File::close(void)
{
    this->set_mode(MODE_UNOPENED);
}

void
File::remove(void)
{
    this->set_mode(MODE_UNOPENED);

    if (::remove(this->filename) != 0)
        throw 666;
}

size_t
File::get_pos(void)
{
    if (this->data == NULL)
        return 0;
    
    return ftell((FILE *)this->data);
}

void
File::set_pos(long int byte)
{
    if (this->mode != MODE_WRITE)
        this->set_mode(MODE_READ);
    
    if (fseek((FILE *)this->data, byte, SEEK_SET) != 0)
        throw 666;
}

void
File::seek(long int byte_offset)
{
    if (this->mode != MODE_WRITE)
        this->set_mode(MODE_READ);
    
    if (fseek((FILE *)this->data, byte_offset, SEEK_CUR) != 0)
        throw 666;
}

void
File::rewind(void)
{
    if (this->mode != MODE_WRITE)
        this->set_mode(MODE_READ);

    ::rewind((FILE *)this->data);
}

bool
File::eof(void)
{
    return (this->data != NULL && feof((FILE *)this->data) != 0);
}

char *
File::read(void)
{
#define CHUNK_SIZE 256
    this->set_mode(MODE_READ);
    
    char buf[CHUNK_SIZE];
    char *result = NULL, *s, *end;
    
    size_t len = 0;
    size_t read_len;
    size_t orig_pos = ftell((FILE *)this->data);
    
    do
    {
        end = buf + fread(buf, 1, CHUNK_SIZE, (FILE *)this->data);

        for (s = buf; s < end; ++s)
        {
            if (*s == '\n')
            {
                // ignore carriage return in Windows style line breaks
                if (s > buf && s[-1] == '\r')
                {
                    orig_pos++;
                    s--;
                }
                else if (len && result[len - 1] == '\r')
                {
                    orig_pos++;
                    len--;
                }
                
                break;
            }
        }
        
        read_len = s - buf;
        
        s = new char[len + read_len + 1];
        
        if (len) // copy previously read
            memcpy(s, result, len);

        memcpy(s + len, buf, read_len); // concat what was just read
        
        delete[] result;

        result = s;
        len   += read_len;
    }
    while (read_len == CHUNK_SIZE);

    result[len] = '\0';
    
    if (!(end == buf && feof((FILE *)this->data)))
    {
        fseek((FILE *)this->data, orig_pos + len, SEEK_SET);
        
        // advance pointer & force EOF flag update:
        fread(buf, 1, 1, (FILE *)this->data);
        
        this->mutable_line_number++;
    }

    return result;
    
#undef CHUNK_SIZE
}

void
File::write(const char *s)
{
    const char *p, *newline = "\r\n";
    char c;
    
    if (s == NULL)
        s = "";
    
    // count line breaks within the string and get string lenght
    for (p = s; (c = *p) != '\0'; ++p)
    {
        this->mutable_line_number += (int)(c == '\n');
    }
    
    this->write(s, 1, p - s);
    this->write(newline, 1, 2);
    
    this->mutable_line_number++;
}

const char *
File::get_contents(bool force_update)
{
    size_t current_size;
    
    this->set_mode(MODE_UNOPENED);
    
    FILE *file = fopen(this->filename, "r");
    if (file == NULL)
    {
        // engine.log("(!) File not found: %s", this->filename);
        return "";
    }
    
    fseek(file, 0, SEEK_END);
    current_size = ftell(file);
    
    // read if size has changed
    if (this->contents == NULL || force_update)
    {
        // engine.log("Reading %s", this->filename);
        
        delete[] this->contents;
        this->contents = new char[current_size + 1];
        
        ::rewind(file);
        current_size = this->read(this->contents, sizeof(char), current_size);
        this->contents[current_size] = '\0';
    }
    fclose(file);
    
    return this->contents;
}

void
File::write_uint8(unsigned int n)
{
    unsigned char v = n;
    this->write(&v, 1, 1);
}

void
File::write_uint16(unsigned int n)
{
    unsigned char v[2];
    v[0] = (n >> 8) & 0xff;
    v[1] =  n       & 0xff;
    this->write(v, 1, 2);
}

void
File::write_uint32(unsigned long int n)
{
    unsigned char v[4];
    v[0] = (n >> 24) & 0xff;
    v[1] = (n >> 16) & 0xff;
    v[2] = (n >> 8)  & 0xff;
    v[3] =  n        & 0xff;
    this->write(v, 1, 4);
}

void
File::write_int(int n)
{
    int sign_bit;
    unsigned long int i;
    unsigned char v[4];
    
    if (n < 0)
    {
        i = -n;
        sign_bit = 1;
    }
    else
    {
        i = n;
        sign_bit = 0;
    }
    
    v[0] = (i >> 24) & 0xff;
    v[1] = (i >> 16) & 0xff;
    v[2] = (i >> 8 ) & 0xff;
    v[3] =  i        & 0xff;
    
    if (sign_bit)
    {
        v[0] |= 0x80;
    }
    else
    {
        v[0] &= 0x7f;
    }
    
    this->write(v, 1, 4);
}

void
File::write_float(float f)
{
    int sign_bit;
    unsigned long int i;
    unsigned char v[4];
    
    if (f < 0)
    {
        f *= -1;
        sign_bit = 1;
    }
    else
    {
        sign_bit = 0;
    }
    
    i = f * 65535.0f;
    v[0] = (i >> 24) & 0xff;
    v[1] = (i >> 16) & 0xff;
    v[2] = (i >> 8 ) & 0xff;
    v[3] =  i        & 0xff;
    
    if (sign_bit)
    {
        v[0] |= 0x80;
    }
    else
    {
        v[0] &= 0x7f;
    }
    
    this->write(v, 1, 4);
}

void
File::write_string(const char *s)
{
    int len;
    if (s != NULL)
    {
        len = strlen(s);
        this->write_uint16(len);
        this->write(s, 1, len);
    }
    else
    {
        this->write_uint16(0xffff);
    }
}

void
File::write_magic(const char *magic_word)
{
    this->write(magic_word, 1, strlen(magic_word));
}

unsigned int
File::read_uint8(void)
{
    unsigned char v;
    this->read(&v, 1, 1);
    
    return v;
}

unsigned int
File::read_uint16(void)
{
    unsigned char v[2];
    this->read(v, 1, 2);
    return (v[0] << 8) | v[1];
}

unsigned long int
File::read_uint32(void)
{
    unsigned char v[4];
    this->read(v, 1, 4);
    return (v[0] << 24) | (v[1] << 16) | (v[2] << 8) | v[3];
}

int
File::read_int(void)
{
    int sign_bit;
    unsigned long int i;
    unsigned char v[4];
    
    this->read(v, 1, 4);
    
    sign_bit = (v[0] & 0x80);
    
    v[0] &= 0x7f; /* clear sign bit */
    
    i = ((unsigned long int)v[0] << 24)
      | ((unsigned long int)v[1] << 16)
      | ((unsigned long int)v[2] << 8 )
      |  (unsigned long int)v[3];
    
    return (sign_bit) ? -((int)i) : (int)i;
}

float
File::read_float(void)
{
    int sign_bit;
    unsigned long int i;
    unsigned char v[4];
    
    this->read(v, 1, 4);
    
    sign_bit = (v[0] & 0x80);
    
    v[0] &= 0x7f; /* clear sign bit */
    
    i = ((unsigned long int)v[0] << 24)
      | ((unsigned long int)v[1] << 16)
      | ((unsigned long int)v[2] << 8 )
      |  (unsigned long int)v[3];
    
    return (sign_bit)
        ? (float)i / -65535.0f
        : (float)i / 65535.0f;
}

char *
File::read_string(void)
{
    int len;
    char *s;
    
    len = this->read_uint16();
    
    if (len == 0xffff)
    {
        /* used to indicate NULL value */
        return NULL;
    }
    
    s = new char[len + 1];
    
    this->read(s, 1, len);
    s[len] = '\0';
    
    return s;
}

void
File::test_magic(const char *magic_word)
{
    size_t  len = strlen(magic_word);
    char   *s   = new char[len];
    
    this->read(s, 1, len);
    
    int cmp = strncmp(magic_word, s, len);

    delete[] s;
    
    if (cmp)
    {
        throw 666;
    }
}


static bool _dir_delimiter[0xff + 1] = { // '/', '\\' and '\0'
/*       00, 01, 02, 03, 04, 05, 06, 07, 08, 09, 0a, 0b, 0c, 0d, 0e, 0f, */
/* 00 */ 1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 10 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 20 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
/* 30 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 40 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/* 50 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,
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
core::str::normalize_path(const char *path, bool trailing_slash)
{
    char
        *normalized = new char[strlen(path) + 2],
        *dst = normalized,
         c;

    for (const char *src = path; (c = *src) != '\0'; ++src, ++dst)
    {
        *dst = c;

        if (c == '.' && src > path
        && (dst == normalized || _dir_delimiter[(int)src[-1]]))
        {
            // current dir "."
            if (_dir_delimiter[(int)src[1]])
            {
                src += 1;
                dst -= 1;
            }
            // previous dir ".."
            else if (src[1] == '.' && _dir_delimiter[(int)src[2]])
            {
                src += 2;
                dst -= 1 + (dst > normalized);
                for (; dst > normalized && !_dir_delimiter[(int)*dst]; --dst);
            }
        }
    }
    // add trailing '/' if omitted
    if (trailing_slash && dst > normalized
    && (*dst == '\0' || !_dir_delimiter[(int)*dst]))
    {
        *dst = '/';
        dst++;
    }
    *dst = '\0';
    
    return normalized;
}

char *
core::str::get_directory(const char *path)
{
    char c;
    const char *end = NULL;
    for (const char *s = path; (c = *s) != '\0'; ++s)
    {
        if (c == '/' || c == '\\')
        {
            end = s;
        }
    }
    
    return (end != NULL)
        ? core::str::substring(path, end)
        : core::str::cat(path, '/');
}

char *
core::str::get_filename(const char *path)
{
    char c;
    const char *start = path, *end = NULL;
    for (; (c = *path) != '\0'; ++path)
    {
        if (c == '/' || c == '\\')
        {
            start = path + 1;
        }
        else if (c == '.')
        {
            end = path;
        }
    }
    
    return (end >= start)
        ? core::str::substring(start, end - 1)
        : core::str::dup(start);
}

char *
core::str::get_extension(const char *path)
{
    const char *start = NULL;
    for (; *path != '\0'; ++path)
    {
        if (*path == '.')
        {
            start = path;
        }
    }
    
    return (start != NULL)
        ? core::str::dup(start + 1)
        : NULL;
}

void
core::str::get_path_components(const char *path, char **dir, char **filename, char **ext)
{
    if (path == NULL)
    {
        path = "";
    }
    
    if (dir != NULL)
    {
        *dir = core::str::get_directory(path);
        core::str::set(*dir, core::str::normalize_path(*dir));
    }

    if (filename != NULL)
    {
        *filename = core::str::get_filename(path);
    }

    if (ext != NULL)
    {
        *ext = core::str::get_extension(path);
    }
}
