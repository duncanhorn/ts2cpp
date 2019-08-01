
#include <cassert>

#include "lexer.h"
#include "parser.h"

using namespace std::literals;

static ast::node* parse_export(lexer& lex);
static ast::object* parse_object(lexer& lex);

static ast::module* parse_module(lexer& lex)
{
    assert(lex.current_token == token::keyword_module);
    lex.advance();

    if (lex.current_token != token::identifier)
    {
        std::printf("ERROR: Unexpected token '%s' for name of module; expected an identifier\n", lex.string_value.c_str());
        return nullptr;
    }

    auto result = std::make_unique<ast::module>();
    result->name.swap(lex.string_value);

    lex.advance();
    if (lex.current_token != token::open_curly)
    {
        std::printf("ERROR: Unexpected token '%s' after declaration of module '%s'; expected an '{'\n", lex.string_value.c_str(), result->name.c_str());
        return nullptr;
    }

    lex.advance();
    while (lex.current_token != token::close_curly)
    {
        switch (lex.current_token)
        {
        case token::keyword_export:
        {
            auto ptr = parse_export(lex);
            if (!ptr)
            {
                std::printf("NOTE: While processing module '%s'\n", result->name.c_str());
                return nullptr;
            }
            ptr->parent = result.get();
            result->children.push_back(std::move(ptr));
        }   break;

        default:
            std::printf("ERROR: Unexpected token '%s' while parsing module '%s' body\n", lex.string_value.c_str(), result->name.c_str());
            return nullptr;
        }
    }

    lex.advance(); // Consume the '}'

    auto resultPtr = result.get();
    lex.file->nodes.push_back(std::move(result));
    return resultPtr;
}

static ast::node* parse_type_reference(lexer& lex)
{
    ast::node* result = nullptr;
    switch (lex.current_token)
    {
    case token::type_string:
        result = new ast::fundamental_type_reference(ast::fundamental_type::string);
        lex.file->nodes.emplace_back(result);
        lex.advance();
        break;

    case token::type_boolean:
        result = new ast::fundamental_type_reference(ast::fundamental_type::boolean);
        lex.file->nodes.emplace_back(result);
        lex.advance();
        break;

    case token::type_number:
        result = new ast::fundamental_type_reference(ast::fundamental_type::number);
        lex.file->nodes.emplace_back(result);
        lex.advance();
        break;

    case token::type_any:
        result = new ast::fundamental_type_reference(ast::fundamental_type::any);
        lex.file->nodes.emplace_back(result);
        lex.advance();
        break;

    case token::open_curly:
        result = parse_object(lex);
        break;

    case token::identifier:
    {
        auto ref = std::make_unique<ast::interface_reference>();
        ref->name.swap(lex.string_value);
        result = ref.get();
        lex.file->nodes.push_back(std::move(ref));
        lex.advance();
    }   break;

    case token::string:
    {
        auto defn = std::make_unique<ast::enumeration>();
        while (true)
        {
            defn->values.push_back(std::move(lex.string_value));
            lex.advance();

            if (lex.current_token != token::pipe)
            {
                break;
            }
            lex.advance();
        }

        result = defn.get();
        lex.file->nodes.push_back(std::move(defn));
    }   break;

    default:
        std::printf("ERROR: Unexpected identifier '%s'; expected a type or identifier\n", lex.string_value.c_str());
        return nullptr;
    }

    return result;
}

static ast::object* parse_object(lexer& lex)
{
    assert(lex.current_token == token::open_curly);
    lex.advance(); // Consume the '{'

    auto result = std::make_unique<ast::object>();
    while (lex.current_token != token::close_curly)
    {
        switch (lex.current_token)
        {
        case token::keyword_module: // Allowed as an identifier in certain contexts
        case token::identifier:
        {
            auto member = std::make_unique<ast::member>();
            member->name.swap(lex.string_value);
            lex.advance();

            if (lex.current_token == token::question)
            {
                member->is_optional = true;
                lex.advance();
            }

            if (lex.current_token != token::colon)
            {
                std::printf("ERROR: Unexpected token '%s' while parsing object member '%s'; expected ':'\n", lex.string_value.c_str(), member->name.c_str());
                return nullptr;
            }
            lex.advance();

            ast::node* type = parse_type_reference(lex);
            if (!type)
            {
                std::printf("NOTE: While processing object member '%s'\n", member->name.c_str());
                return nullptr;
            }

            if (lex.current_token == token::open_bracket)
            {
                auto arr = std::make_unique<ast::array>();
                arr->type = type;
                type->parent = arr.get();
                type = arr.get();
                lex.file->nodes.push_back(std::move(arr));

                lex.advance();
                if (lex.current_token != token::close_bracket)
                {
                    std::printf("ERROR: Unexpected token '%s' while parsing object member '%s'; expected ']'\n", lex.string_value.c_str(), member->name.c_str());
                    return nullptr;
                }
                lex.advance();
            }

            if (lex.current_token != token::semicolon)
            {
                std::printf("ERROR: Unexpected token '%s' while parsing object member '%s'; expected ';'\n", lex.string_value.c_str(), member->name.c_str());
                return nullptr;
            }
            lex.advance();

            auto memberPtr = member.get();
            lex.file->nodes.push_back(std::move(member));
            result->named_members.push_back(memberPtr);
        }   break;

        default:
            std::printf("ERROR: Unexpected token '%s' while parsing object body\n", lex.string_value.c_str());
            return nullptr;
        }
    }

    lex.advance(); // Consume the '}'

    auto resultPtr = result.get();
    lex.file->nodes.push_back(std::move(result));
    return resultPtr;
}

static ast::interface* parse_interface(lexer& lex)
{
    assert(lex.current_token == token::keyword_interface);
    lex.advance();

    if (lex.current_token != token::identifier)
    {
        std::printf("ERROR: Unexpected token '%s' for name of interface; expected an identifier\n", lex.string_value.c_str());
        return nullptr;
    }

    auto result = std::make_unique<ast::interface>();
    result->name.swap(lex.string_value);

    lex.advance();
    if (lex.current_token == token::keyword_extends)
    {
        lex.advance();
        if (lex.current_token != token::identifier)
        {
            std::printf("ERROR: Unexpected token '%s' while parsing 'extends' type for interface '%s'; expected an identifier\n", lex.string_value.c_str(), result->name.c_str());
            return nullptr;
        }

        auto baseRef = std::make_unique<ast::interface_reference>();
        baseRef->name.swap(lex.string_value);
        result->base = baseRef.get();
        lex.file->nodes.push_back(std::move(baseRef));
        lex.advance();
    }

    if (lex.current_token != token::open_curly)
    {
        std::printf("ERROR: Unexpected token '%s' after declaration of interface '%s'; expected an '{'\n", lex.string_value.c_str(), result->name.c_str());
        return nullptr;
    }

    result->definition = parse_object(lex);
    if (!result->definition)
    {
        std::printf("NOTE: While processing interface '%s'\n", result->name.c_str());
        return nullptr;
    }
    result->definition->parent = result.get();

    auto resultPtr = result.get();
    lex.file->nodes.push_back(std::move(result));
    return resultPtr;
}

static ast::node* parse_export(lexer& lex)
{
    assert(lex.current_token == token::keyword_export);
    lex.advance();
    switch (lex.current_token)
    {
    case token::keyword_module:
    {
        auto result = parse_module(lex);
        if (result) result->is_export = true;
        return result;
    }

    case token::keyword_interface:
    {
        auto result = parse_interface(lex);
        if (result) result->is_export = true;
        return result;
    }

    default:
        std::printf("ERROR: Unexpected token '%s' while parsing export\n", lex.string_value.c_str());
        return nullptr;
    }
}

std::unique_ptr<ast::file> parse_file(std::ifstream& input)
{
    auto result = std::make_unique<ast::file>();

    lexer lex(input, result.get());
    while (lex)
    {
        bool firstToken = true;
        switch (lex.current_token)
        {
        case token::string:
            if (lex.string_value != "use strict"sv)
            {
                std::printf("ERROR: String '%s' unexpected at file scope\n", lex.string_value.c_str());
                return nullptr;
            }
            else if (lex.advance(); lex.current_token != token::semicolon)
            {
                std::printf("ERROR: Missing ';' after 'use strict'\n");
                return nullptr;
            }
            else if (!firstToken)
            {
                std::printf("ERROR: 'use strict' must be the first statement\n");
                return nullptr;
            }
            result->strict = true;
            break;

        case token::keyword_export:
        {
            auto ptr = parse_export(lex);
            if (!ptr) return nullptr;
            ptr->parent = result.get();
            result->children.push_back(ptr);
        }   break;

        default:
            std::printf("ERROR: Token '%s' unexpected at file scope\n", lex.string_value.c_str());
            return nullptr;
        }

        lex.advance();
        firstToken = false;
    }

    return result;
}
