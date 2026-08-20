#include <bulbtils/string.h>

#include <sstream>
#include <algorithm> // std::ranges::transform
#include <iterator>
#include <format>

namespace bulbtils::string
{
    void replace_all(std::string& str, const std::string& from, const std::string& to)
    {
        if (from.empty())
        {
            return;
        }

        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    std::vector<std::string> split_by_whitespace(const std::string& str)
    {
        std::istringstream iss(str);
        return { (std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>() };
    }

    std::vector<std::string> split_parameters(const std::string& str, const std::initializer_list<char>& grouping_chars)
    {
        for (auto c : grouping_chars)
        {
            if (std::isspace(c))
            {
                throw std::logic_error(std::format("Whitespace `{}` ({:d}, 0x{:x}) cannot be a grouping character", c, c, c));
            }
            else if (c == 0)
            {
                throw std::logic_error(std::format("Null cannot be a grouping character"));
            }
        }

        // Currently active grouping character (or 0 if none is active)
        char grouping_char = 0;

        std::vector<std::string> parameters {};
        std::string current_parameter = "";
        for (auto i = 0; i < str.size(); i++)
        {
            auto c = str[i];

            // Is our character whitespace?
            if (std::isspace(c))
            {
                // We're currently in a group, whitespace is a valid character for this parameter
                if (grouping_char)
                {
                    current_parameter += c;
                    continue;
                }

                // We're not currently in a group, and this parameter is empty - nothing to do, skip...
                if (current_parameter.empty())
                {
                    continue;
                }

                // We're not currently in a group, but this parameter is NOT empty
                // This means our parameter has concluded, and we can add it to our list
                parameters.push_back(current_parameter);
                current_parameter = "";
                continue;
            }

            // Our character isn't whitespace, so is it our currently active grouping character?
            if (c == grouping_char)
            {
                // Is it NOT the last character of our input string
                if (i + 1 < str.size())
                {
                    auto next_c = str[i + 1];

                    // If it isn't, and there isn't a space after it either, that's not good
                    if (!std::isspace(next_c))
                    {
                        throw std::invalid_argument(std::format("Stray closing grouping character `{}` ({:d}, 0x{:x}) at position {}", c, c, c, i));
                    }
                }

                // At this point, either one of these two things are true:
                // 1. This is the very last character of our input string, or
                // 2. It is NOT, but there is a space right after it
                // This is valid, and we can safely end the grouping and add the parameter
                // In case the parameter is empty, that's also valid (double grouping chars)
                grouping_char = 0;
                parameters.push_back(current_parameter);
                current_parameter = "";
                continue;
            }

            // Our character isn't whitespace nor is it our currently active grouping character
            // Could it be any of the other grouping characters that were provided?
            // (only bother checking if we have multiple grouping chars at all)
            if (grouping_chars.size() > 1 && std::find(grouping_chars.begin(), grouping_chars.end(), c) != grouping_chars.end())
            {
                // If we already have a grouping character currently active, add the character
                // Because we know for a fact this character isn't the one used for the grouping
                if (grouping_char)
                {
                    current_parameter += c;
                    continue;
                }

                // If we don't have a grouping character currently active, this one is eligible
                // Make sure it's only at the beginning of a parameter (after whitespace)
                if (!current_parameter.empty())
                {
                    throw std::invalid_argument(std::format("Stray opening grouping character `{}` ({:d}, 0x{:x}) at position {}", c, c, c, i));
                }

                // No active grouping character and it's at a good position, so this will be it
                grouping_char = c;
                continue;
            }

            // Our character isn't whitespace or any other grouping character
            // In that case, we don't need to do anything special
            current_parameter += c;

            // We've just parsed the very last character of our input string
            if (i == str.size() - 1)
            {
                // If we're still grouping a parameter at this point, this is very bad
                if (grouping_char)
                {
                    throw std::invalid_argument(std::format("Stray grouping character `{}` ({:d}, 0x{:x}) was left open", grouping_char, grouping_char, grouping_char));
                }

                // We're not grouping parameters, it is safe to end this one and add it to the list
                parameters.push_back(current_parameter);
            }
        }
        return parameters;
    }

    namespace inplace
    {
        void to_lowercase(std::string& source_dest)
        {
            auto to_lowercase_fn = [](unsigned char c) { return std::tolower(c); };
            std::ranges::transform(source_dest, source_dest.begin(), to_lowercase_fn);
        }

        void remove_extra_spaces(std::string& source_dest)
        {
            auto both_are_spaces = [](char lhs, char rhs) { return (lhs == rhs) && (lhs == ' '); };
            auto new_end = std::unique(source_dest.begin(), source_dest.end(), both_are_spaces);
            source_dest.erase(new_end, source_dest.end());
        }
    }
}