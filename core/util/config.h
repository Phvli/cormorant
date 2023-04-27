#ifndef _CORE_UTIL_CONFIG_H
#define _CORE_UTIL_CONFIG_H

#include <map>
#include <iterator>
#include <cstring>

namespace core
{
    class Config;
    
    class Config
    {
        public:
            typedef
                enum {
                    FIRST,    // first existing element of the array
                    MIDDLE,   // middle element (or next to midpoint on even number of elements)
                    LAST,     // last existing element
                    RANDOM,   // random existing element
                    WEIGHTED, // random existing element (element values being numeric occurrence weights between 0 and an arbitrary maximum)
                    NEXT,     // new element inserted with the next available numeric key
                    APPEND,   // new element inserted as the absolute last one (random key)
                    PREPEND   // new element inserted before any existing ones (random key, wraps around at zero)
                }             // if the element is empty, an empty child will be created
                Accessor;
            
            typedef
                unsigned int
                Hash; // 32-bit
                
            typedef
                std::map<Hash, Config *>
                Value;
            
            Config();
            Config(const char *filename);
            Config(const Config &config);
            ~Config();
            
            static Hash
            get_hash(const char *s);

            /* Save and load */
            void load(const char *filename);
            void save(const char *filename);
            
            /*
                Methods for binding variables for direct access.
                
                (!) NOTE: binding a string (char *) frees no resources,
                          but assigns the default value anyway.
            */
            void bind(bool         &variable, bool  def = false);
            void bind(char         &variable, char  def = 0x00);
            void bind(int          &variable, int   def = 0);
            void bind(float        &variable, float def = 0.0f);
            void bind(unsigned int &variable, float def = 0x00000000);
            void bind(char *       &variable, const char *def = NULL);
            void bind(Config       &nested_config);
            void unbind();
            
            Config &operator[](const char *key);
            Config &operator[](int numeric_key);
            Config &operator[](Accessor token);
            Config &find(const char *path); // Returns a child object, keys in path separated by '/'
            
            const char *key(void);  // returns string key or NULL if has no parents
            int numeric_key(void);  // returns numeric key or NULL if has no parents
            
            bool exists()   const;  // Returns true if a value has been assigned or bound
            bool is_bound() const;  // Returns true if a variable has been bound to this value
            bool is_empty() const;  // Returns true if contains any child variables
            int  count()    const;  // Returns the number of contained child variables

            /* Value getters */
            bool          boolean(bool         def = false) const;
            int           integer(int          def = 0)     const;
            float         real   (float        def = 0.0f)  const;
            unsigned int  uint32 (unsigned int def = 0)     const;
            const char   *string (const char  *def = "(undefined)");
            float         percentage(int       def = 0)     const;
            bool          chance (int          def = 50)    const;  // Probability of returning true is the assigned numeric value (0 - 100).
            
            /* Value setters */
            Config &operator=(const char *v);
            Config &operator=(bool v);
            Config &operator=(char v);
            Config &operator=(int v);
            Config &operator=(float v);
            Config &operator=(unsigned int v);
            Config &operator=(const Config &v);

            void  set(const char *v); // sets value and auto-assigns a type
            void *get(void);          // returns pointer to the bound value (NULL if nothing bound)
            
            Config *
            pop(bool free_memory = false); // Returns this, detached from parent's bookkeeping (must be deleted by the caller unless free_memory is true, in which case returns NULL)
            
            bool operator==(const char *key) const;
            bool operator==(const Config &c) const;
            bool operator!=(const char *key) const;
            bool operator!=(const Config &c) const;
            
            // Search for a value; returns pointer or NULL if not found
            Config *search(int         match) const;
            Config *search(float       match, float threshold = 0.0f) const;
            Config *search(const char *match);

            void
            print(void *stream = NULL, int indent = 0); // print to a FILE stream (NULL prints to stdout)
            
            Config *
            merge(const Config *config);
            
            Config *
            diff(const Config *config);
            
            Config *
            intersect(const Config *config);
            
            void clear();
            inline void empty() { this->clear(); }
            
            Config::Value::iterator begin() { return this->children.begin(); }
            Config::Value::iterator end()   { return this->children.end();   }
            Config::Value::const_iterator begin() const { return this->children.begin(); };
            Config::Value::const_iterator end()   const { return this->children.end();   };
            
            
            
        protected:
            void *value; // pointer to bound variable
            int   type;  // type of bound variable
            
            char  *s;   // container for return value of string() method
            unsigned char data[sizeof(void *)]; // data storage for unbound values
            
            Config *parent;
            Config::Value children;
            
            void _bind(void *p, int type);
            bool parse_file_contents(const char *s);
            
            void read_binary(void *stream);
            void write_binary(void *stream);
    };
}


#endif
