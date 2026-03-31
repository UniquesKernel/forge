#ifndef FORGE_TOML_HPP
#define FORGE_TOML_HPP

#include "token.hpp"
#include <string>
struct Settings {
    std::string binary_name;
    std::string compiler;
};

char* read_project_file();
Settings parse_project_file(TokenStream tokens);

#endif