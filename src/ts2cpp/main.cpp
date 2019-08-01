
#include <fstream>

#include "parser.h"

int main(int argc, char** argv)
{
    const char* filename = "proto.ts";
    if (argc > 1) filename = argv[1];

    std::ifstream input(filename);
    if (input.fail())
    {
        std::printf("ERROR: Failed to open file '%s'\n", filename);
        return 1;
    }

    auto file = parse_file(input);
    if (!file)
    {
        std::printf("Error encountered while parsing file; aborting\n");
        return 1;
    }

    return 0;
}
