#pragma once

#include "ast.h"

std::unique_ptr<ast::file> parse_file(std::ifstream& input);
