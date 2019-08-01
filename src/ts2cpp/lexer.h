#pragma once

#include <fstream>

#include "ast.h"

enum class token
{
    // State values
    invalid,
    eof,

    // Characters
    open_curly, // {
    close_curly, // }
    open_bracket, // [
    close_bracket, // ]
    semicolon, // ;
    colon, // :
    question, // ?
    pipe, // |

    // Keywords
    keyword_export,
    keyword_module,
    keyword_interface,
    keyword_extends,

    // Types
    type_any,
    type_boolean,
    type_string,
    type_number,

    // Arbitrary string values
    string,
    identifier,
};

struct lexer
{
    lexer(std::ifstream& input, ast::file* file) : input(input), file(file) { advance(); }

    explicit operator bool() const noexcept
    {
        return (current_token != token::eof) && (current_token != token::invalid);
    }

    void advance();

    std::ifstream& input;
    ast::file* file;
    token current_token = token::invalid;
    std::string string_value;
};
