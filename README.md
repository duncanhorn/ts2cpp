# TypeScript to C++ (ts2cpp)
The primary purpose of this project is to convert interfaces defined in TypeScript into C++ type definitions. For example, when interfacing with a protocol that returns JSON whose documentation/specification is written in TypeScript. In addition to converting types, it will also generate "reflection" structures/functions that are used when serializing/parsing JSON text.

## Examples
Here are a few examples that describe how the conversion process works

### Simple Example
Take the following TypeScript:
```ts
export module Foo {
    export interface Bar {
        x: number;
    }
}
```
This will generate the following type definition in C++:
```c++
namespace Foo
{
    struct Bar
    {
        json::number_t x;
    };
}
```

### Optional Example
Optional members get converted as the `json::optional_t<>` type (which is just a typedef for `std::optional<>`). E.g.:
```ts
export interface LoginInfo {
    username: string;
    password?: string;
}
```
Will become:
```c++
struct LoginInfo
{
    json::string_t username;
    json::optional_t<json::string_t> password;
};
```

### Unnamed Structure Example
If the type of a member is an object declared inline (i.e. using the `foo: { ... }` syntax), then the generated structure name will attempt to combine the member and interface name. For example:
```ts
export interface Computer {
    screen: {
        width: number;
        height: number;
    }
}
```
Will become:
```c++
struct ComputerScreen
{
    json::number_t width;
    json::number_t height;
};

struct Computer
{
    ComputerScreen screen;
};
```

### Enumeration Example
Enumerations are extracted out and named in an identical manner to unnamed structures. For example, take the following:
```ts
export interface Car {
    make: 'Ford' | 'Honda' | 'Nissan';
}
```
Will become:
```c++
enum class CarMake
{
    Ford,
    Honda,
    Nissan,
};

struct Car
{
    CarMake make;
};
```
If the enumeration is a member of an unnamed structure, the interface name used to generate the enum name will be the first user-supplied interface name. E.g. if we modify the above:
```ts
export interface Car {
    info: {
        make: 'Ford' | 'Honda' | 'Nissan';
    };
}
```
The generated enum will still be named `CarMake`.
