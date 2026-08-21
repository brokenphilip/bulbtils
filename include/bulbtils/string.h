#pragma once

#include <string>
#include <vector>

namespace bulbtils::string
{
    // Wrapper for string literals, primarily used for template arguments
    template <size_t N>
    struct literal
    {
        char str[N]{ 0 };
        constexpr literal(const char(&str)[N])
        {
            for (int i = 0; i < N; i++)
            {
                this->str[i] = str[i];
            }
        }
    };

    // Replace all instances of "from" with "to" in "str"
    void replace_all(std::string& str, const std::string& from, const std::string& to);

    // Split "str" into individual tokens, by any amount of whitespace
    std::vector<std::string> split_by_whitespace(const std::string& str);

    // Split "str" into parameters, following these rules:
    // 1. Each parameter is separated by whitespace, unless it is being grouped (read below)
    // 2. Parameters are grouped using a matching identical pair of grouping characters
    //    a) By default, these are single (') and double (") quotations
    //    b) An opening grouping character must only be found at the beginning of a parameter
    //       ...which is either at the beginning of the input "str", or after whitespace
    //    c) A closing grouping character must only be found at the end of a parameter
    //       ...which is either at the end of the input "str", or before whitespace
    //    d) Whitespace characters and the null terminator (\0) cannot be used as grouping characters
    //    e) Grouping characters cannot be found "standalone" (ie. without a matching grouping character)
    //       ...unless they're being grouped using a matching pair of different grouping characters
    //    f) Parameter groups cannot be found "left open" - each must have a matching closing grouping character
    //
    // Examples of how various {input}s convert to various [output]s and their (parameter)s:
    // 
    // {foo bar} -> [ (foo)  (bar) ]
    // {foo    bar} -> [ (foo)  (bar) ]
    // {foo          bar} -> [ (foo)  (bar) ]
    // {foo bar          } -> [ (foo)  (bar) ]
    // {          foo bar} -> [ (foo)  (bar) ]
    // {"foo bar"} -> [ (foo bar) ]
    // {'foo bar'} -> [ (foo bar) ]
    // {""} -> [ () ]
    // {''} -> [ () ]
    // {foo '' "" bar} -> [ (foo)  ()  ()  (bar) ]
    // {foo "bar baz"} -> [ (foo)  (bar baz) ]
    // {'foo bar' baz} -> [ (foo bar)  (baz) ]
    // {"foo bar" 'baz qux'} -> [ (foo bar)  (baz qux) ]
    // {"foo'bar" 'baz"qux'} -> [ (foo'bar)  (baz"qux) ]
    // {"foo 'bar baz' qux"} -> [ (foo 'bar baz' qux) ]
    // {foo     "bar     baz     "     qux} -> [ (foo)  (bar     baz     )  (qux) ]
    // {foo   "   bar baz   "   qux} -> [ (foo)  (   bar baz   )  (qux) ]
    // {foo"bar} -> [ ERROR: Stray opening grouping character `"` (34, 0x22) at position 3 ]
    // {"foo"bar} -> [ ERROR: Stray closing grouping character `"` (34, 0x22) at position 4 ]
    // {foo'bar'} -> [ ERROR: Stray opening grouping character `'` (39, 0x27) at position 3 ]
    // {"foo bar} -> [ ERROR: Stray grouping character `"` (34, 0x22) was left open ]
    // {'foo bar} -> [ ERROR: Stray grouping character `'` (39, 0x27) was left open ]
    std::vector<std::string> split_parameters(const std::string& str, const std::initializer_list<char>& grouping_chars = {'"', '\''});

    namespace inplace
    {
        // Replaces all characters within a string to their lowercase equivalents
        void to_lowercase(std::string& source_dest);

        // Removes all extra spaces (and spaces only, NOT other whitespace) from a string
        void remove_extra_spaces(std::string& source_dest);
    }
}