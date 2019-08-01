#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ast
{
    struct node
    {
        virtual ~node() {}

        node* parent = nullptr;
    };

    struct file : node
    {
        bool strict = false;
        std::vector<node*> children;

        // For cleanup
        std::vector<std::unique_ptr<node>> nodes;
    };

    struct module : node
    {
        bool is_export = false;
        std::string name;
        std::vector<node*> children;
    };

    struct member : node
    {
        bool is_optional = false;
        std::string name;
        node* type;
    };

    enum class fundamental_type
    {
        any,
        boolean,
        number,
        string,
    };

    struct fundamental_type_reference : node
    {
        fundamental_type type;
        fundamental_type_reference(fundamental_type type) : type(type) {}
    };

    struct interface_reference : node
    {
        std::string name;
    };

    struct object : node
    {
        std::vector<member*> named_members;
        // TODO: unnamed members (i.e. arbitrary key:value pairs)
    };

    struct interface : node
    {
        bool is_export = false;
        node* base = nullptr;
        std::string name;
        object* definition = nullptr;
    };
}
