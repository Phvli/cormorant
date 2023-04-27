#ifndef _CORE_UTIL_STRING_H
#define _CORE_UTIL_STRING_H

#include <cstdarg>
#include "config.h"

namespace core
{
    class Config;
    
    namespace str
    {
        char *
        empty(void);
        // Returns an empty string with enough space reserved for the terminating NULL character only

        char *
        empty(char *&s);
        // Empties given string

        char *
        set(char *&s, char *value);
        // Frees s, then sets it to point to value

        char *
        dup(const char *s);

        char *
        dupn(const char *s, int n);

        char *
        format(const char *format, ...);
        // Modifiers not supported

        char *
        vformat(const char *format, va_list args);
        // Modifiers not supported

        char *
        cat(char c1, char c2);

        char *
        cat(char c1, const char *s2);

        char *
        cat(const char *s1, char c2);

        char *
        cat(const char *s1, const char *s2);

        char *
        cat(const char *s1, const char *s2);

        char *
        insert(const char *base, const char *s, unsigned int position, unsigned int length = 0);

        char *
        cut(const char *s, unsigned int position, unsigned int length);

        char *
        substring(const char *start, const char *end);

        char *
        replace(char *s, char match, char replacement);

        char *
        replace(char *s, char match, char replacement, int max_replacements);

        char *
        replace(const char *s, const char *match, const char *replacement);

        char *
        replace(const char *s, const char *match, const char *replacement, int max_replacements);

        char *
        trim(char *s);
        
        char *
        ltrim(char *s);
        
        char *
        rtrim(char *s);
        
        char *
        ltrim(char *s, const char *match);
        
        char *
        rtrim(char *s, const char *match);
        
        char *
        strip(char *s, char c);
        // Strips all occurrences of c
        
        char *
        strip(char *s, const char *c_list);
        // Strips all occurrences of any character in c
        
        char *
        strip_non_visible(char *s);
        // Strips all ASCII codes between 1 and 31
        
        char *
        strip_duplicate(char *s, char c);
        // Strips all repeated occurrences of c from s, returns s (c must not be '\0')
        
        char *
        ucase(char *s);
        
        char *
        lcase(char *s);

        char
        ucase(char c);
        
        char
        lcase(char c);

        bool
        is_numeric(char c);
        // Returns true for '0' ... '9', '+', '-' and '.'
        
        bool
        is_numeric(const char *s);
        // Returns true if begins with is_numeric(char) and is a number

        char *
        unescape(char *s);
        // Removes quotation marks, unescapes \", \' and \t and strips anything not quoted
        
        char *
        dup_escaped(const char *s);

        char *
        decode_html(const char *s);
        // Replaces HTML entities with printable characters

        char
        first(const char *s);
        // Returns the first character or '\0' if the string is NULL or empty

        char
        last(const char *s);
        // Returns the last character or '\0' if the string is NULL or empty

        const char *
        last(const char *s, int n);
        // Returns the last n characters from s
        
        int
        len(const char *s);
        // Returns string length

        int
        cmp(const char *s1, const char *s2);
        // String comparison (0 if equal, otherwise diff of the first difference)

        int
        icmp(const char *s1, const char *s2);
        // Case-insensitive compare

        int
        ncmp(const char *s1, const char *s2, int n);
        // Limited-length compare
        
        int
        nicmp(const char *s1, const char *s2, int n);
        // Limited-length case-insensitive compare

        int
        count(const char *s, const char *match);
        // Counts substring occurrences in s

        int
        icount(const char *s, const char *match);
        // Counts substring occurrences in s (case-insensitive)

        char *
        find(const char *haystack, const char *needle);
        // Returns pointer to haystack at the first occurrence of needle (NULL if not found)

        char *
        find(const char *haystack, char needle);
        // Returns pointer to haystack at the first occurrence of needle (NULL if not found)

        char *
        ifind(const char *haystack, const char *needle);
        // Case-insensitive match

        char *
        ifind(const char *haystack, char needle);
        // Case-insensitive match
        
        bool
        contains(const char *haystack, const char *needle);

        bool
        begins(const char *haystack, const char *needle);

        bool
        ends(const char *haystack, const char *needle);

        bool
        icontains(const char *haystack, const char *needle);

        bool
        ibegins(const char *haystack, const char *needle);

        bool
        iends(const char *haystack, const char *needle);

        Config *
        split(const char *s, char delimiter = 0, bool include_empty = false);
        // Splits into a numerically indexed Config (use delimiter = 0 to split at any space)
        
        Config *
        split_unescaped(const char *s, char delimiter = ',');
        // Splits while retaining (and unescaping) quoted values

        char *
        join(const Config *list, char delimiter, bool include_empty = false);
        // Concats all Config string values optionally separated with a delimiter character

        char *
        join(const Config *list, const char *delimiter = NULL, bool include_empty = false);
        // Concats all Config string values optionally separated with a delimiter string
    }
}

#endif