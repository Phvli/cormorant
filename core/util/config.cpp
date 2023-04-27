#include "config.h"
#include "file.h"
#include "string.h"

#include <cstdio>  // fprintf
#include <cstring> // strcmp, strncmp, strlen
#include <cstdlib> // rand()

#include <map>
#include <string>

using namespace core;

#define TYPE_UNDEFINED 0
#define TYPE_ARRAY     1
#define TYPE_BOOL      2
#define TYPE_CHAR      3
#define TYPE_INT       4
#define TYPE_FLOAT     5
#define TYPE_UINT32    6
#define TYPE_STRING    7


/* Jenkins one-at-a-time hash */
Config::Hash
Config::get_hash(const char *s)
{
    Config::Hash hash;
    
    for (hash = 0x00000000; *s != '\0'; ++s)
    {
        hash += *s;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    
    return hash;
}

class KeyStorage
{
    /* Global storage for key -> hash mapping
      instead of storing duplicates for every Config instance with the same key.
    */
public:
    std::map<Config::Hash, char *> data;
    
    const char *
    get(Config::Hash hash)
    {
        std::map<Config::Hash, char *>::iterator i = this->data.find(hash);
        
        return (i != this->data.end())
            ? i->second
            : NULL;
    }
    
    void
    store(Config::Hash hash, const char *key)
    {
        if (this->data.find(hash) == this->data.end())
        {
            this->data.insert(std::make_pair(hash, str::dup(key)));
        }
    }
    
    ~KeyStorage()
    {
        for (std::map<Config::Hash, char *>::iterator i = this->data.begin();
        i != this->data.end(); ++i)
        {
            delete[] i->second;
        }
    }
    
} stored_keys;

Config::Config()
{
    this->parent = NULL;
    
    this->type   = TYPE_UNDEFINED;
    this->value  = NULL;
    this->s      = NULL;
}

Config::Config(const Config &config)
{
    this->parent = NULL;
    
    this->type   = TYPE_UNDEFINED;
    this->value  = NULL;
    this->s      = NULL;

    *this = config;
}

Config::Config(const char *filename)
{
    this->parent = NULL;
    
    this->type   = TYPE_UNDEFINED;
    this->value  = NULL;
    this->s      = NULL;

    this->load(filename);
}

Config &
Config::operator=(const Config &v)
{
    // clear old data
    if (this->type == TYPE_STRING && this->value == this->data)
    {
        delete[] *(char **)this->value;
    }
    delete[] this->s;
    this->s = NULL;
    this->children.clear();

    // copy data from v to this
    this->type = v.type;
    if (v.value == v.data)
    {
        if (this->type == TYPE_STRING)
        {
            this->value = this->data;
            *(char **)this->value = str::dup(*(const char **)v.value);
        }
        else
        {
            memcpy(&this->data, &v.data, sizeof(void *));
            this->value = this->data;
        }

    }
    else
    {
        this->value = v.value;
    }

    // copy children recursively
    for (Config::Value::const_iterator i = v.children.begin();
    i != v.children.end(); ++i)
    {
        (*this)[i->first] = *i->second;
    }

    return *this;
}

Config::~Config()
{
    this->pop();
    this->clear();
}

Config *
Config::pop(bool free_memory)
{
    if (this->parent != NULL)
    {
        // remove from parent's bookkeeping
        for (Config::Value::iterator i = this->parent->children.begin();
        i != this->parent->children.end(); ++i)
        {
            if (i->second == this)
            {
                this->parent->children.erase(i);
                break;
            }
        }
        this->parent = NULL;
    }

    if (free_memory)
    {
        delete this;
        return NULL;
    }
    
    return this;
}

void
Config::clear()
{
    for (Config::Value::iterator i = this->children.begin(); i != this->children.end(); ++i)
    {
        i->second->parent = NULL; // prevent destructor from meddling with the container while we iterate
        delete i->second;
    }
    
    this->children.clear();
    
    this->_bind(NULL, TYPE_UNDEFINED);
    
    delete[] this->s;
    this->s = NULL;
}

void
Config::_bind(void *p, int type)
{
    if (this->type == TYPE_STRING && this->value == this->data)
    {
        delete[] *(char **)this->value;
    }
    this->type  = type;
    this->value = p;
}

void
Config::unbind()
{
    this->_bind(NULL, TYPE_UNDEFINED);
}

void
Config::bind(bool &variable, bool def)
{
    variable = def;
    this->_bind(&variable, TYPE_BOOL);
}

void
Config::bind(char &variable, char def)
{
    variable = def;
    this->_bind(&variable, TYPE_CHAR);
}

void
Config::bind(int &variable, int def)
{
    variable = def;
    this->_bind(&variable, TYPE_INT);
}

void
Config::bind(float &variable, float def)
{
    variable = def;
    this->_bind(&variable, TYPE_FLOAT);
}

void
Config::bind(unsigned int &variable, float def)
{
    variable = def;
    this->_bind(&variable, TYPE_UINT32);
}

void
Config::bind(char *&variable, const char *def)
{
    variable = (def != NULL)
        ? str::dup(def)
        : NULL;
    
    this->_bind(&variable, TYPE_STRING);
}

void
Config::bind(Config &nested_config)
{
    Config *c;
    Config::Value::iterator i;
    
    if (&nested_config == this)
        return;
    
    /* Prevent from binding an ancestor to its children */
    for (c = this->parent; c != NULL; c = c->parent)
    {
        if (c == &nested_config)
            throw std::runtime_error("Tried to bind an ancestor to its children");
    }
    
    if (nested_config.parent != NULL)
    {
        /* Free nested_config from its parent if already bound */
        c = nested_config.parent;
        for (i = c->children.begin(); i != c->children.end(); ++i)
        {
            if (i->second == &nested_config)
            {
                c->children.erase(i);
                break;
            }
        }
        
        /* If nested_config was further down on the same branch, delete the branches in-between */
        for (c = nested_config.parent; c->parent != NULL; c = c->parent)
        {
            if (c->parent == this)
            {
                for (i = this->children.begin(); i != this->children.end(); ++i)
                {
                    if (i->second == c)
                    {
                        this->children.erase(i);
                        
                        c->parent = NULL;
                        delete c;
                        
                        break;
                    }
                }
                break;
            }
        }
        
    }
    
    /* Find this in parent's children and replace the reference with nested_config */
    c = this->parent;
    for (i = c->children.begin(); i != c->children.end(); ++i)
    {
        if (i->second == this)
        {
            /* Move nested_config where this was */
            nested_config.parent = c;
            i->second = &nested_config;
            
            this->parent = NULL;
            delete this;
            return;
        }
    }
}

const char *
Config::string(const char *def)
{
    switch (this->type)
    {
    case TYPE_BOOL:
        return (*(bool *)this->value) ? "yes" : "no";
    
    case TYPE_CHAR:
        delete[] this->s;
        this->s = new char[4];
        sprintf(this->s, "#%02x", *(char *)this->value);
        break;
    
    case TYPE_INT:
        delete[] this->s;
        this->s = new char[21];
        sprintf(this->s, "%d", *(int *)this->value);
        break;
    
    case TYPE_FLOAT:
        delete[] this->s;
        this->s = new char[32];
        sprintf(this->s, "%g", *(float *)this->value);
        break;
    
    case TYPE_UINT32:
        delete[] this->s;
        this->s = new char[10];
        sprintf(this->s, "#%06x", *(unsigned int *)this->value);
        break;
    
    case TYPE_STRING:
        return *(const char **)this->value;
    
    default:
        return def;
    }
    
    return this->s;
}

bool
Config::boolean(bool def)
const
{
    switch (this->type)
    {
    case TYPE_BOOL:   return *(bool *)this->value;
    case TYPE_CHAR:   return (*(char *)this->value != 0);
    case TYPE_INT:    return (*(int *)this->value != 0);
    case TYPE_FLOAT:  return (*(float *)this->value != 0);
    case TYPE_UINT32: return (*(unsigned int *)this->value != 0);
    case TYPE_STRING:
        return (*(char **)this->value != NULL
            && strcmp(*(char **)this->value, "false")
            && strcmp(*(char **)this->value, "no")
            && strcmp(*(char **)this->value, "0"));
    }
    
    return def;
}

int
Config::integer(int def)
const
{
    switch (this->type)
    {
    case TYPE_BOOL:   return *(bool *)this->value;
    case TYPE_CHAR:   return *(char *)this->value;
    case TYPE_INT:    return *(int *)this->value;
    case TYPE_FLOAT:  return *(float *)this->value + .5f;
    case TYPE_UINT32: return *(unsigned int *)this->value;
    case TYPE_STRING:
        sscanf(*(const char **)this->value, "%d", &def);
        return def;
    }
    
    return def;
}

unsigned int
Config::uint32(unsigned int def)
const
{
    switch (this->type)
    {
    case TYPE_BOOL:   return *(bool *)this->value;
    case TYPE_CHAR:   return *(char *)this->value;
    case TYPE_INT:    return *(int *)this->value;
    case TYPE_FLOAT:  return *(float *)this->value + .5f;
    case TYPE_UINT32: return *(unsigned int *)this->value;
    case TYPE_STRING:
        sscanf(*(const char **)this->value, "%u", &def);
        return def;
    }
    
    return def;
}

float
Config::real(float def)
const
{
    switch (this->type)
    {
    case TYPE_BOOL:   return *(bool *)this->value;
    case TYPE_CHAR:   return *(char *)this->value;
    case TYPE_INT:    return *(int *)this->value;
    case TYPE_FLOAT:  return *(float *)this->value;
    case TYPE_UINT32: return *(unsigned int *)this->value;
    case TYPE_STRING:
        sscanf(*(const char **)this->value, "%f", &def);
        return def;
    }
    
    return def;
}

float
Config::percentage(int def)
const
{
    float f = def;
    
    switch (this->type)
    {
    case TYPE_BOOL:   f = (float)*(bool *)this->value; break;
    case TYPE_CHAR:   f = (float)*(char *)this->value; break;
    case TYPE_INT:    f = (float)*(int *)this->value; break;
    case TYPE_FLOAT:  f = (float)*(float *)this->value; break;
    case TYPE_UINT32: f = (float)*(unsigned int *)this->value; break;
    case TYPE_STRING:
        sscanf(*(const char **)this->value, "%f", &f);
        break;
    }
    
    f /= 100.0f;

    if      (f < 0.0f) f = 0.0f;
    else if (f > 1.0f) f = 1.0f;
    
    return f;
}

bool
Config::chance(int def)
const
{
    // return ((rand() % 100) <= this->integer(50));
    return ((rand() & 0xffff) <= 0xffff * this->uint32(def) / 100);
}

Config &
Config::operator=(bool v)
{
    if (this->value == NULL)
    {
        this->value = this->data;
        this->type  = TYPE_BOOL;
    }
    
    switch (this->type)
    {
    case TYPE_BOOL:   *(bool *)this->value = v; break;
    case TYPE_INT:    *(int *)this->value = v; break;
    case TYPE_FLOAT:  *(float *)this->value = v; break;
    case TYPE_UINT32: *(unsigned int *)this->value = v; break;
    case TYPE_CHAR:   *(char *)this->value = v; break;
    case TYPE_STRING:
        delete[] *(char **)this->value;
        if (v)
        {
            *(char **)this->value = new char[4];
            strcpy(*(char **)this->value, "yes");
        }
        else
        {
            *(char **)this->value = new char[3];
            strcpy(*(char **)this->value, "no");
        }
        break;
    }
    
    return *this;
}

Config &
Config::operator=(char v)
{
    if (this->value == NULL)
    {
        this->value = this->data;
        this->type  = TYPE_CHAR;
    }
    
    switch (this->type)
    {
    case TYPE_BOOL:   *(bool *)this->value = v; break;
    case TYPE_INT:    *(int *)this->value = v; break;
    case TYPE_FLOAT:  *(float *)this->value = v; break;
    case TYPE_UINT32: *(unsigned int *)this->value = v; break;
    case TYPE_CHAR:   *(char *)this->value = v; break;
    case TYPE_STRING:
        delete[] *(char **)this->value;
        *(char **)this->value = new char[4];
        sprintf(*(char **)this->value, "#%02x", v);
        break;
    }
    
    return *this;
}

Config &
Config::operator=(int v)
{
    if (this->value == NULL)
    {
        this->value = this->data;
        this->type  = TYPE_INT;
    }
    
    switch (this->type)
    {
    case TYPE_BOOL:   *(bool *)this->value = v; break;
    case TYPE_INT:    *(int *)this->value = v; break;
    case TYPE_FLOAT:  *(float *)this->value = v; break;
    case TYPE_UINT32: *(unsigned int *)this->value = v; break;
    case TYPE_CHAR:   *(char *)this->value = v; break;
    case TYPE_STRING:
        delete[] *(char **)this->value;
        *(char **)this->value = new char[21];
        sprintf(*(char **)this->value, "%d", v);
        break;
    }
    
    return *this;
}

Config &
Config::operator=(float v)
{
    if (this->value == NULL)
    {
        this->value = this->data;
        this->type  = TYPE_FLOAT;
    }
    
    switch (this->type)
    {
    case TYPE_BOOL:   *(bool *)this->value = v; break;
    case TYPE_INT:    *(int *)this->value = v; break;
    case TYPE_FLOAT:  *(float *)this->value = v; break;
    case TYPE_UINT32: *(unsigned int *)this->value = v; break;
    case TYPE_CHAR:   *(char *)this->value = v; break;
    case TYPE_STRING:
        delete[] *(char **)this->value;
        *(char **)this->value = new char[32];
        sprintf(*(char **)this->value, "%g", v);
        break;
    }
    
    return *this;
}

Config &
Config::operator=(unsigned int v)
{
    if (this->value == NULL)
    {
        this->value = this->data;
        this->type  = TYPE_UINT32;
    }
    
    switch (this->type)
    {
    case TYPE_BOOL:   *(bool *)this->value = v; break;
    case TYPE_INT:    *(int *)this->value = v; break;
    case TYPE_FLOAT:  *(float *)this->value = v; break;
    case TYPE_UINT32: *(unsigned int *)this->value = v; break;
    case TYPE_CHAR:   *(char *)this->value = v; break;
    case TYPE_STRING:
        delete[] *(char **)this->value;
        *(char **)this->value = new char[10];
        sprintf(*(char **)this->value, "#%06x", v);
        break;
    }
    
    return *this;
}

Config &
Config::operator=(const char *v)
{
    if (this->value == NULL)
    {
        this->value = this->data;
        this->type  = TYPE_STRING;
        *(void **)this->data = NULL;
    }
    
    switch (this->type)
    {
    case TYPE_BOOL:
        *(bool *)this->value = (v[0] != '\0'
            && strcmp(v, "false")
            && strcmp(v, "no")
            && strcmp(v, "0"));
        break;
        
    case TYPE_INT:
    case TYPE_FLOAT:
    case TYPE_UINT32:
    case TYPE_CHAR:
        if (v[0] == '#')
        {
            unsigned int temp;
            sscanf(++v, "%x", &temp);
            switch (this->type)
            {
            case TYPE_INT: *(int *)this->value = temp; break;
            case TYPE_FLOAT: *(float *)this->value = temp; break;
            case TYPE_UINT32: *(unsigned int *)this->value = temp; break;
            case TYPE_CHAR: *(char *)this->value = temp & 0xff; break;
            }
        }
        else
        {
            switch (this->type)
            {
            case TYPE_INT:    sscanf(v, "%d",  (int *)this->value); break;
            case TYPE_FLOAT:  sscanf(v, "%f",  (float *)this->value); break;
            case TYPE_UINT32: sscanf(v, "%u", (unsigned int *)this->value); break;
            case TYPE_CHAR:
                int temp;
                sscanf(v, "%d", &temp);
                *(char *)this->value = temp;
                break;
            }
        }
        break;
        
    case TYPE_STRING:
        delete[] *(char **)this->value;
        *(char **)this->value = str::dup(v);
        break;
    }
    
    return *this;
}

void *
Config::get(void)
{
    return (this->value != this->data)
        ? this->value
        : NULL;
}

void
Config::set(const char *v)
{
    const char *s;
    char c;

    if (this->value == NULL)
    {
        this->value = this->data;
        
        /* PARSED STRING */
        if (v[0] == '"')
        {
            int len = strlen(v);

            if (v[len - 1] == '"')
            {
                this->type = TYPE_STRING;
                *(void **)this->data = str::unescape(str::substring(v + 1, v + len - 2));
                return;
            }
        }
         /* UINT */
        else if (v[0] == '#')
        {
            for (s = v + 1; (c = *s) != '\0'; ++s)
            {
                if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')))
                    break;
            }

            if (*s == '\0' && s != v)
            {
                this->type = TYPE_UINT32;
                *this = v;
                return;
            }
        }
        /* INT, FLOAT */
        else
        {
            int decimal_points = 0;
            for (s = v; (c = *s) != '\0'; ++s)
            {
                if (c == '.')
                    decimal_points++;
                else if (c < '0' || c > '9')
                    break;
            }

            if (*s == '\0' && s != v && decimal_points <= 1)
            {
                this->type = (decimal_points) ? TYPE_FLOAT : TYPE_INT;
                *this = v;
                return;
            }
        }

        /* BOOLEAN */
        if (strcmp(v, "yes") == 0)
        {
            this->type = TYPE_BOOL;
            *(bool *)this->value = true;
        }
        else if (strcmp(v, "no") == 0)
        {
            this->type = TYPE_BOOL;
            *(bool *)this->value = false;
        }
        /* FALL BACK TO A STRING */
        else
        {
            this->type = TYPE_STRING;
            *(void **)this->data = NULL;
        }
    }

    *this = v;
}

bool
Config::operator==(const char *key)
const
{
    return (this->parent != NULL)
        ? (this == &(*this->parent)[key])
        : false;
}

bool
Config::operator==(const Config &c)
const
{
    if (this->type != c.type)
        return false;
    
    float mismatch = false;
    switch (this->type)
    {
    case TYPE_BOOL:
        mismatch = (*(bool *)this->value != *(bool *)c.value);
        break;
    
    case TYPE_INT:
        mismatch = (*(int *)this->value != *(int *)c.value);
        break;
    
    case TYPE_FLOAT:
        mismatch = (*(float *)this->value != *(float*)c.value);
        break;
    
    case TYPE_UINT32:
        mismatch = (*(unsigned int *)this->value != *(unsigned int *)c.value);
        break;
    
    case TYPE_CHAR:
        mismatch = (*(char *)this->value != *(char *)c.value);
        break;
    
    case TYPE_STRING:
        mismatch = (strcmp(*(char **)this->value, *(char **)c.value) != 0);
        break;
    }
    
    if (mismatch)
        return false;
    
    Config::Value::const_iterator i0, i1;
    i0 = this->children.begin();
    i1 = c.children.begin();
    
    for (;;)
    {
        if (i0 == this->children.end() || i1 == c.children.end())
            break;
        
        if (i0->first != i1->first || *i0->second != *i1->second)
            return false;
        
        i0++;
        i1++;
    }
    
    return (i0 == this->children.end() && i1 == c.children.end());
}

bool Config::operator!=(const Config &c)
const { return !(*this == c);   }
bool Config::operator!=(const char *key)
const { return !(*this == key); }

const char *
Config::key(void)
{
    if (this->parent != NULL)
    {
        for (Config::Value::iterator i = this->parent->children.begin();
        i != this->parent->children.end(); ++i)
        {
            if (i->second == this)
            {
                return stored_keys.get(i->first);
            }
        }
    }
    
    return NULL;
}

int
Config::numeric_key(void)
{
    if (this->parent != NULL)
    {
        for (Config::Value::iterator i = this->parent->children.begin();
        i != this->parent->children.end(); ++i)
        {
            if (i->second == this)
            {
                return i->first;
            }
        }
    }
    
    return -1;
}

Config &
Config::find(const char *path)
{
    const char *s;
    Config *c = this;
    
    for (s = path;; ++s)
    {
        if (*s == '/' || *s == '\0')
        {
            path = str::unescape(str::substring(path, s - 1));
            c = &(*c)[path];
            delete[] (char *)path;
            
            if (*s == '\0')
                return *c;
            
            path = s + 1;
        }
    }
}

Config &
Config::operator[](const char *key)
{
    Config::Hash hash = Config::get_hash(key);
    Config::Value::iterator i = this->children.find(hash);
    
    if (i == this->children.end())
    {
        i = this->children.insert(std::make_pair(hash, new Config())).first;
        i->second->parent = this;
        stored_keys.store(hash, key);
    }
    
    return *i->second;
}

Config &
Config::operator[](int numeric_key)
{
    Config::Hash hash = numeric_key;
    Config::Value::iterator i = this->children.find(hash);
    
    if (i == this->children.end())
    {
        i = this->children.insert(std::make_pair(hash, new Config())).first;
        i->second->parent = this;
    }
    
    return *i->second;
}

Config &
Config::operator[](Accessor token)
{
    Config *result;
    if (this->children.size())
    {
        Config::Value::iterator i;
        Config::Hash hash;
        int n;
        
        switch (token)
        {
            // get existing elements
            case Config::FIRST:
                result = this->children.begin()->second;

                break;
            

            case Config::LAST:
                result = this->children.rbegin()->second;

                break;


            case Config::RANDOM:
            case Config::MIDDLE:
                n = this->children.size();
            
                if (token == Config::RANDOM)
                    n = rand() % n;
                else
                    n /= 2;
                
                for (i = this->children.begin(); --n >= 0; ++i);
                result = i->second;
            
                break;
            

           case Config::WEIGHTED:
                i = this->children.begin();
                hash = this->children.rbegin()->first;
                if (hash)
                {
                    for (hash = rand() % hash; i->first < hash; ++i);
                }
                result = i->second;

                break;
 

            // create new elements
            case Config::PREPEND:
                result = &(*this)[this->children.begin()->first - 1];

                break;
            

            case Config::APPEND:
                result = &(*this)[this->children.rbegin()->first + 1];

                break;

            
            case Config::NEXT:
                i = this->children.find(n = 0);
                for (i = this->children.begin();
                    i != this->children.end() && (int)i->first <= n;
                    ++i, ++n
                );
                    
                result = &(*this)[n];

                break;
            

            default:
                // result = this;
                result = &(*this)[0];
        }
    }
    else
    // no elements => create new or return this
    {
        result = &(*this)[0];
        // switch (token)
        // {
        //     // create new elements
        //     case Config::PREPEND:
        //     case Config::APPEND:
        //     case Config::NEXT:
        //         result = &(*this)[0];
        //         break;
            

        //     default:
        //         result = this;
        // }
    }
    
    return *result;
}

bool
Config::exists()
const
{
    return (this->type != TYPE_UNDEFINED);
}

bool
Config::is_bound()
const
{
    return (this->value != this->data && this->value != NULL);
}

bool
Config::is_empty()
const
{
    return (this->children.size() == 0);
}

int
Config::count()
const
{
    return ((int)this->children.size());
}

Config *
Config::search(int match)
const
{
    for (Config::Value::const_iterator i = this->children.begin();
        i != this->children.end(); ++i)
    {
        if (i->second->integer() == match)
        {
            return i->second;
        }
    }

    return NULL;
}

Config *
Config::search(float match, float threshold)
const
{
    match    -= threshold;
    threshold = match + threshold * 2.0f;
    for (Config::Value::const_iterator i = this->children.begin();
        i != this->children.end(); ++i)
    {
        float f = i->second->real();
        if (f >= match && f <= threshold)
        {
            return i->second;
        }
    }

    return NULL;
}

Config *
Config::search(const char *match)
{
    for (Config::Value::iterator i = this->children.begin();
        i != this->children.end(); ++i)
    {
        if (strcmp(i->second->string(), match) == 0)
        {
            return i->second;
        }
    }

    return NULL;
}

void
Config::load(const char *filename)
{
    core::File file(filename);

    if (file.ext == NULL || (
       strcmp(file.ext,  "txt")
    && strcmp(file.ext,  "cfg")
    && strncmp(file.ext, "conf", 4)))
        this->read_binary(&file);
    else
        this->parse_file_contents(file.get_contents());
}

void
Config::save(const char *filename)
{
    core::File file(filename);
    
    if (file.ext == NULL || (
       strcmp(file.ext,  "txt")
    && strcmp(file.ext,  "cfg")
    && strncmp(file.ext, "conf", 4)))
        this->write_binary(&file);
    else
        this->print(file.get_FILE());
}

void
Config::write_binary(void *stream)
{
    core::File *file = (core::File *)stream;
    
    file->write_uint16(this->children.size());
    if (this->children.size())
    {
        for (Config::Value::iterator i = this->children.begin(); i != this->children.end(); ++i)
        {
            file->write_uint32(i->first);
            i->second->write_binary(stream);
        }
    }
    else
    {
        file->write_uint8(this->type);
        switch (this->type)
        {
            case TYPE_BOOL:   file->write_uint8(*(bool *)this->value); break;
            case TYPE_CHAR:   file->write_uint8((*(char *)this->value)); break;
            case TYPE_INT:    file->write_int((*(int *)this->value)); break;
            case TYPE_FLOAT:  file->write_float((*(float *)this->value)); break;
            case TYPE_UINT32: file->write_uint32((*(unsigned int *)this->value)); break;
            case TYPE_STRING: file->write_string(*(char **)this->value); break;
        }
    }
}

void
Config::read_binary(void *stream)
{
    core::File *file = (core::File *)stream;
    char *temp;

    int children = file->read_uint16();
    if (children)
    {
        for (; children; --children)
        {
            Hash hash = file->read_uint32();
            (*this)[hash].read_binary(stream);
        }
    }
    else
    {
        switch (file->read_uint8())
        {
            case TYPE_BOOL:   *this = (bool)file->read_uint8(); break;
            case TYPE_CHAR:   *this = (char)file->read_uint8(); break;
            case TYPE_INT:    *this = (int)file->read_int(); break;
            case TYPE_FLOAT:  *this = (float)file->read_float(); break;
            case TYPE_UINT32: *this = (unsigned int)file->read_uint32(); break;
            case TYPE_STRING:
                *this = temp = file->read_string();
                delete[] temp;
                break;
            
            default:
                throw 666;
        }
    }

    if (file->eof())
        throw 666;
}

void
Config::print(void *stream, int indent)
{
    static const char *indent_str = "                                ";
    
    FILE *file = (stream != NULL)
        ? (FILE *)stream
        : stdout;
    
    if (this->children.size())
    {
        // TODO: sort alphabetically by key
        for (Config::Value::const_iterator i = this->children.begin();
        i != this->children.end(); ++i)
        {
            const char *separator;
            if (i->second->children.size())
            {
                if (i != this->children.begin())
                    fprintf(file, LINE_BREAK);
                
                separator = ":" LINE_BREAK;
            }
            else
                separator = ": ";
            
            // indent lines
            int n;
            for (n = indent; n > 32; n -= fwrite(indent_str, 1, 32, file));
            fwrite(indent_str, 1, n, file);
            
            // string key
            const char *key = stored_keys.get(i->first);
            if (key != NULL)
                fprintf(file, "%s", key);
            
            // numeric key
            else if (i->first >= 0 && i->first <= 9999)
                fprintf(file, "(%i)", i->first);
            
            // unresolved hash
            else
                fprintf(file, "%x", i->first);

            fprintf(file, separator);
            i->second->print(stream, indent + 4);
        }
    }
    else if (this->type == TYPE_STRING)
    {
        char *s = str::dup_escaped(this->string());
        fprintf(file, "%s" LINE_BREAK, s);
        delete[] s;
    }
    else
        fprintf(file, "%s" LINE_BREAK, this->string());
}

bool
Config::parse_file_contents(const char *s)
{
#define MAX_NESTING_DEPTH 64
    struct
    {
        int indent;
        Config *ptr;
    } nest_table[MAX_NESTING_DEPTH], *nesting;

    const char *pos, *start_pos, *end_pos;
    char *key = NULL;
    char c;
    
    int line_number = 1;

    int state  = 0;
    int indent = 0;
    start_pos  = NULL;
    
    bool is_not_space;
    
    nesting = nest_table;
    nesting->indent = 0;
    nesting->ptr = this;
 
    for (pos = s;; ++pos)
    {
        c = *pos;
        
        /* Skip over quoted strings */
        if (c == '"')
        {
            start_pos = pos;
            for (pos++;; ++pos)
            {
                c = *pos;
                
                if (c == '\\')
                    c = *(++pos);
                else if (c == '"')
                    break;
                
                if (c == '\n' || c == '\0')
                    break;
            }
            end_pos = pos;
            
            if (c == '"')
                continue;
        }
        
        /* Line breaks and EOF */
        if (c == '\n' || c == '\0')
        {
            if (state > 0)
            // check if the indentation has lessened (ignore empty lines)
            {
                while (indent < nesting->indent)
                {
                    nesting--;
                }
            }

            Config *dst;

            if (key != NULL)
            /* resolve key */
            {
                nesting->ptr->type = TYPE_ARRAY;
                
                if (key[0] == '(' && str::last(key) == ')')
                /* resolve key functions */
                {
                    int numeric_key;
                    if (key[1] >= '0' && key[1] <= '9')
                    {
                        sscanf(key + 1, "%d", &numeric_key);
                    }
                    else
                    {
                        sscanf(key + 2, "%d", &numeric_key);
                        switch (key[1] * (nesting->ptr->children.size() > 0))
                        {
                            /* (+n): add n to highest key */
                            case '+':
                                numeric_key
                                    += nesting->ptr->children.rbegin()->first;
                                break;

                            /* (-n): subtract n from smallest key */
                            case '-':
                                numeric_key
                                    -= nesting->ptr->children.begin()->first;
                                break;
                        }
                    }
                    dst = &(*nesting->ptr)[numeric_key];
                }
                else
                {
                    dst = &(*nesting->ptr)[key];
                }

                str::set(key, NULL);
            }
            else if (start_pos != NULL)
            /* value listed alone => assing to the next unused numeric key */
            {
                nesting->ptr->type = TYPE_ARRAY;
                dst = &(*nesting->ptr)[nesting->ptr->count()];
            }
            else
            /* else it's just a line break (empty line) => do nothing */
            {
                dst = NULL;
            }

            if (dst != NULL)
            /* assing value */
            {
                if (start_pos != NULL
                    && !(*start_pos == '-' && end_pos == start_pos))
                {
                    char *value  = str::substring(start_pos, end_pos);
                    Config *list = str::split_unescaped(value);
                    
                    if (list->count() > 1)
                    {
                        *dst = *list;
                    }
                    else
                    {
                        dst->set((*list)[Config::FIRST].string());
                    }
                    delete   list;
                    delete[] value;
                }
                else
                {
                    // Create nested array if no value specified
                    // or if the line was just '-' alone in a row
                    nesting[1].ptr = dst;
                    nesting++;
                    nesting->indent = indent + 1;
                }
            }

            /* break on EOF */
            if (c == '\0')
                break;
            
            state     = 0;
            indent    = 0;
            start_pos = NULL;
            
            line_number++;
            continue;
        }
        
        is_not_space = (c > ' ');
        
        switch (state)
        {
            /* comment */
            case -1:
                continue;
            
            /* indentation preceding anything */
            case 0:
                if (is_not_space)
                {
                    /* Comments (first non-space character is hash) */
                    if (c == '#')
                    {
                        state = -1;
                        continue;
                    }
                    state++;
                }
                else
                {
                    indent++;
                    break;
                }
            
            /* value or key-value pair */
            case 1:
                if (c == ':')
                {
                    delete[] key;
                    key = str::unescape(str::substring(start_pos, end_pos));
                    
                    start_pos = NULL;
                    state++;
                    continue;
                }
                break;
        }
        
        if (is_not_space)
        {
            if (start_pos == NULL)
            {
                start_pos = pos;
            }
            
            end_pos = pos;
        }
        
        
    }
    
    return true;
#undef MAX_NESTING_DEPTH
}
