/*
    FILE SYSTEM WRAPPER
    
    core::Dir
        Reads directory contents
        Test if a directory exists
    
        ------------------------------------------------------------------------
        core::Dir dir("data/files/misc");
        for (int i = 0; i < dir.count(); ++i)
            printf("%s\n", dir[i]);
        ------------------------------------------------------------------------
    

    core::File
        Read/write text and binary files in a portable format
        Test if a file exist
        Create, delete, copy and move files

        ------------------------------------------------------------------------
        core::File text("test.txt");
        core::File data("test.dat");
        text->write("Hello, World");
        data->write_uint16(42);
        data->rewind();
        int i = data->read_uint16();
        ------------------------------------------------------------------------

*/

#ifndef _CORE_UTIL_FILE_H
#define _CORE_UTIL_FILE_H

#include <cstddef>
#include "config.h"

#define DATA_DIRECTORY "../data/"

#ifdef _WIN32
#define LINE_BREAK "\r\n"
#else
#define LINE_BREAK "\n"
#endif

namespace core
{
    namespace str
    {
        char *
        normalize_path(const char *path, bool trailing_slash = false);
        // Remove all redundant ./ and ../ directives

        char *
        get_directory(const char *path);
        // Return directory path with a trailing slash with filename (or last directory) stripped

        char *
        get_filename(const char *path);
        // Return filename without path or extension

        char *
        get_extension(const char *path);
        // Return file extension or NULL if not present

        void
        get_path_components(const char *path, char **dir, char **filename, char **ext);
        // Assign normalized directory, file name and extension.
        // Any value can be left NULL. Old strings are not deallocated.
    }
    
    class Dir
    {
    public:
        char * const &path;
        char * const &ext;

        const unsigned int &flags;
            static const int FILES          = 0x01;
            static const int DIRECTORIES    = 0x02;
            static const int ALL            = (FILES | DIRECTORIES);

            static const int EXCLUDE_HIDDEN = 0x04;
            static const int OMIT_EXTENSION = 0x08;
            static const int FULL_PATH      = 0x10;
        
        Dir();
        Dir(const char *path, unsigned int flags = ALL, const char *extension = NULL);
        ~Dir();
        
        bool
        exists(void);
        
        void
        filter(unsigned int flags);
        
        void
        filter(const char *path, const char *extension = NULL);
        
        const char *
        operator[](int index);
        
        int
        count();
        
        int
        count(unsigned int flags);
        
        void
        refresh(void);
        
    protected:
        bool changed, is_valid;
        unsigned int mutable_flags;
        char *mutable_path, *mutable_ext;
        void *data;
        int files, hidden_files, directories;
    };

    class File
    {
    public:
        char * const &filename;
        char * const &ext;
        const int    &line_number; // reliable only when reading/writing line by line
        
        File(const char *filename);
        ~File();
        
        const char *
        get_contents(bool force_update = false);
        // Stays valid until object desctruction. Reads and buffers the file only upon first call, or if force_update is true.
        
        bool   eof(void);
        bool   exists(void);
        size_t get_size(void);
        
        void   copy(const char *filename);
        void   rename(const char *filename);
        void   remove(void);
        
        void   append(void); // Opens the file in append mode (call before any writing operations) (invalidates line_number)
        void   empty(void);  // Empties the file (does this by default on first write operation)
        void   close(void);  // Closes the file immediately instead of waiting for object destruction. Opens again on the next read/write.
        
        /* READ/WRITE POSITION */
        size_t get_pos(void);
        void   set_pos(long int byte);     // invalidates line_number
        void   seek(long int byte_offset); // invalidates line_number
        void   rewind(void);               // invalidates line_number
        size_t read(void *ptr, size_t size, size_t count); // performs fread()
        size_t write(const void *ptr, size_t size, size_t count); // performs fwrite()
        void * get_FILE(void); // returns FILE pointer
        
        /* FILE IO: READING */
        char *            read(void); // reads a single line (terminated at EOF, "\n" or "\r\n")
        int               read_int     (void);
        float             read_float   (void);
        unsigned int      read_uint8   (void);
        unsigned int      read_uint16  (void);
        unsigned long int read_uint32  (void);
        char *            read_string  (void);

        /* FILE IO: WRITING */
        void              write(const char *s); // writes a single line plus an "\r\n" line break
        void              write_int    (int n);   // 32-bit
        void              write_float  (float f); // 32-bit
        void              write_uint8  (unsigned int n);
        void              write_uint16 (unsigned int n);
        void              write_uint32 (unsigned long int n);
        void              write_string (const char *s); // 0...65534 characters or NULL.

        /* MAGIC WORDS:   Write/test for a matching string in the file. */
        void              write_magic  (const char *magic_word);
        void              test_magic   (const char *magic_word); // Throws an exception on mismatch.
        
    protected:
        char  mode;
        int   mutable_line_number;
        char *contents, *mutable_filename, *mutable_ext;
        void *data;

        void
        set_filename(const char *filename);

        void
        set_mode(int mode); // opens or closes the file in appropriate mode if not done already
    };
}

#endif
