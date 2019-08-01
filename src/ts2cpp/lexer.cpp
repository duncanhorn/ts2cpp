
#include "lexer.h"

using namespace std::literals;

static constexpr bool is_whitespace(char ch) noexcept
{
    return (ch == ' ') || (ch == '\f') || (ch == '\n') || (ch == '\r') ||
        (ch == '\t') || (ch == '\v');
}

static constexpr bool in_range(char ch, char begin, char end) noexcept
{
    return (ch >= begin) && (ch <= end);
}

static constexpr bool is_valid_identifier_start(char ch) noexcept
{
    return in_range(ch, 'A', 'Z') || in_range(ch, 'a', 'z');
}

static constexpr bool is_valid_identifier_character(char ch) noexcept
{
    return is_valid_identifier_start(ch) || in_range(ch, '0', '9') || (ch == '_');
}

template <typename Func>
static void skip_while(std::ifstream& input, Func&& func)
{
    while (!input.fail() && func(static_cast<char>(input.peek()))) { input.get(); }
}

static void skip_whitespace(std::ifstream& input)
{
    skip_while(input, is_whitespace);
}

void lexer::advance()
{
    current_token = token::invalid;
    do
    {
        skip_whitespace(input);
        if (input.eof())
        {
            current_token = token::eof;
            return;
        }
        else if (input.fail())
        {
            std::printf("ERROR: Failed to read data from file\n");
            return;
        }

        auto ch = input.get();
        switch (ch)
        {
        case ';':
            string_value = ";";
            current_token = token::semicolon;
            break;

        case ':':
            string_value = ":";
            current_token = token::colon;
            break;

        case '{':
            string_value = "{";
            current_token = token::open_curly;
            break;

        case '}':
            string_value = "}";
            current_token = token::close_curly;
            break;

        case '?':
            string_value = "?";
            current_token = token::question;
            break;

        case '|':
            string_value = "|";
            current_token = token::pipe;
            break;

        case '[':
            string_value = "[";
            current_token = token::open_bracket;
            break;

        case ']':
            string_value = "]";
            current_token = token::close_bracket;
            break;

        case '/':
            if (input.peek() == '/')
            {
                // Read until the end of the line
                skip_while(input, [](char ch) { return ch != '\n'; });
                input.get(); // Consume the '\n'
            }
            else if (input.peek() == '*')
            {
                // Read until we get an ending '*/'
                input.get(); // Consume the initial '*'
                while (true)
                {
                    skip_while(input, [](char ch) { return ch != '*'; });
                    input.get(); // Consume the '*'
                    if (input.peek() == '/')
                    {
                        input.get(); // Consume the '/'
                        break;
                    }

                    if (input.eof())
                    {
                        std::printf("ERROR: End of file reached while parsing comment\n");
                        return;
                    }
                    else if (input.fail())
                    {
                        std::printf("ERROR: Failed to read data from file\n");
                        return;
                    }
                }
            }
            else
            {
                std::printf("ERROR: Unexpected character '%c' after '/'\n", input.peek());
                return;
            }
            break;

        case '\'':
        case '\"':
            // NOTE: All strings we will be processing will be quite simple as they are almost exclusively used as
            // identifiers, so keep it simple for now
            string_value.clear();
            while (true)
            {
                auto next = input.get();
                if (next == ch)
                {
                    current_token = token::string;
                    break;
                }
                else if (input.eof())
                {
                    std::printf("ERROR: End of file encountered while parsing string\n");
                    return;
                }
                else if (input.fail())
                {
                    std::printf("ERROR: Failed to read data from file\n");
                    return;
                }
                else
                {
                    string_value.push_back(static_cast<char>(next));
                }
            }
            break;

        default:
            if (is_valid_identifier_start(static_cast<char>(ch)))
            {
                string_value.clear();
                while (true)
                {
                    string_value.push_back(static_cast<char>(ch));
                    auto next = input.peek();
                    if (is_valid_identifier_character(static_cast<char>(next)))
                    {
                        ch = input.get(); // Consume and keep going
                    }
                    else if (!input.fail() || input.eof())
                    {
                        break;
                    }
                    else
                    {
                        std::printf("ERROR: Failed to read data from file\n");
                        return;
                    }
                }

                if (string_value == "string"sv)
                {
                    current_token = token::type_string;
                }
                else if (string_value == "boolean"sv)
                {
                    current_token = token::type_boolean;
                }
                else if (string_value == "number"sv)
                {
                    current_token = token::type_number;
                }
                else if (string_value == "any"sv)
                {
                    current_token = token::type_any;
                }
                else if (string_value == "export"sv)
                {
                    current_token = token::keyword_export;
                }
                else if (string_value == "interface"sv)
                {
                    current_token = token::keyword_interface;
                }
                else if (string_value == "extends"sv)
                {
                    current_token = token::keyword_extends;
                }
                else if (string_value == "module"sv)
                {
                    current_token = token::keyword_module;
                }
                else
                {
                    current_token = token::identifier;
                }
            }
            else
            {
                std::printf("ERROR: Invalid character '%c'\n", ch);
                return;
            }
        }
    } while (current_token == token::invalid);
}
