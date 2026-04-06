#ifndef FORGE_TOML_HPP
#define FORGE_TOML_HPP

#include "token.hpp"
#include <filesystem>
#include <string>
#include <vector>

struct Library {
        std::string              lib_name;
        std::filesystem::path    src_path;
        std::vector<std::string> dependencies;
};

struct Binary {
        std::string              bin_name;
        std::filesystem::path    src_path;
        std::vector<std::string> dependencies;
};

struct Config {
        std::string binary;
        std::string compiler;
        std::string compiler_standard;
};

struct Settings {
        Config               config;
        std::vector<Library> libraries;
        std::vector<Binary>  binaries;
};

const char* read_project_toml();
Settings    parse_project_file(TokenStream tokens);

#endif
