/*
    Generic key-object cache.
    Upon destruction deletes all objects the pointers to which it contains.
    
        ------------------------------------------------------------------------
        core::Cache<std::string> cache;
        cache->store("one", new std::string("Hello, "));
        cache->store("two", new std::string("World!"));
        cout << cache["one"] << cache["two"];
        return 0;
        ------------------------------------------------------------------------
        
    Phvli 2017-08-06
*/

#ifndef _CORE_UTIL_CACHE_H
#define _CORE_UTIL_CACHE_H

#include <cstddef> // void
#include <map>

namespace core
{
    template <class T>
    class Cache
    {
        public:
            typedef
                unsigned int
                Hash; // 32-bit
            
            Cache() {}
            ~Cache() { this->flush(); }
            
            T *
            store(const char *key, T *data_point);
            // Stores a new object and assigns given key to it.
            // The object should be deleted after it's been stored.
            // Cache destroys stored objects in its destructor.
            // Returns data_point.
            // If a conflicting old value exists, data_point will
            // be destroyed immediately and the old value returned
            // instead.
            
            T *
            get(const char *key);
            // Returns pointer to the cached object.
            
            T *
            operator[](const char *key) { return this->get(key); }
            // Same as get().
            
            bool
            contains(const char *key);
            // Returns true if given key is cached.
            
            bool
            contains(const T *data_point);
            // Returns true if given object is cached.
            
            void
            drop(const char *key, bool delete_target = true);
            // Deletes given index from the cache.
            // Actual object will only be deleted if delete_target is set to true.
            
            void
            drop(T *data_point, bool delete_target = true);
            // Deletes given object's index from the cache.
            // Actual object will only be deleted if delete_target is set to true.
            
            void
            flush(bool delete_targets = true);
            // Empties cache.
            // Actual objects will only be deleted if delete_targets is set to true.

        protected:
            typedef
                std::map<Hash, T *>
                Storage;
            
            static Hash
            get_hash(const char *s);
            
            Storage data;
    };

    // Classic Jenkins
    template <class T>
    typename Cache<T>::Hash
    Cache<T>::get_hash(const char *key)
    {
        typename Cache<T>::Hash hash;
        
        for (hash = 0x00000000; *key != '\0'; ++key)
        {
            hash += *key;
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }
        
        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);
        
        return hash;
    }

    template <class T>
    bool
    Cache<T>::contains(const char *key)
    {
        return (this->data.find(Cache::get_hash(key)) != this->data.end());
    }

    template <class T>
    bool
    Cache<T>::contains(const T *data_point)
    {
        for (typename Cache::Storage::iterator i = this->data.begin();
            i != this->data.end(); ++i)
        {
            if (i->second == data_point)
            {
                return true;
            }
        }

        return false;
    }

    template <class T>
    T *
    Cache<T>::get(const char *key)
    {
        typename Cache::Storage::iterator i
            = this->data.find(Cache::get_hash(key));
        
        return (i != this->data.end())
            ? i->second
            : NULL;
    }

    template <class T>
    T *
    Cache<T>::store(const char *key, T *data_point)
    {
        typename Cache::Hash hash = Cache::get_hash(key);

        typename Cache::Storage::iterator i = this->data.find(hash);
        if (i != this->data.end())
        {
            delete data_point;
            return i->second;
        }
        
        this->data.insert(std::make_pair(hash, data_point));
        return data_point;
    }

    template <class T>
    void
    Cache<T>::flush(bool delete_targets)
    {
        if (delete_targets)
        {
            Cache::Storage backup = this->data;
            this->data.clear();

            for (typename Cache::Storage::iterator i = backup.begin();
                i != backup.end(); ++i)
            {
                delete i->second;
            }
        }
        else
        {
            this->data.clear();
        }
    }

    template <class T>
    void
    Cache<T>::drop(const char *key, bool delete_target)
    {
        typename Cache::Storage::iterator i
            = this->data.find(Cache::get_hash(key));
        
        if (i != this->data.end())
        {
            if (delete_target)
            {
                delete i->second;
            }
            this->data.erase(i);
        }
    }

    template <class T>
    void
    Cache<T>::drop(T *data_point, bool delete_target)
    {
        for (typename Cache::Storage::iterator i = this->data.begin();
            i != this->data.end(); ++i)
        {
            if (i->second == data_point)
            {
                if (delete_target)
                {
                    delete i->second;
                }
                this->data.erase(i);
                return;
            }
        }
    }
}

#endif
