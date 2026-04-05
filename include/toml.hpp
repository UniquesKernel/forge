#ifndef FORGE_TOML_HPP
#define FORGE_TOML_HPP

#include "token.hpp"
#include <filesystem>
#include <string>
#include <vector>

struct Library {
        std::string           lib_name;
        std::filesystem::path src_path;
        std::vector<int>      dependencies;
};

struct Config {
        std::string binary;
        std::string compiler;
        std::string compiler_standard;
};

struct Settings {
        Config               config;
        std::vector<Library> libraries;
};

char*    read_project_toml();
Settings parse_project_file(TokenStream tokens);

#endif