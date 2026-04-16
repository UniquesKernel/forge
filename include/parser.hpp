#ifndef PARSER_HPP
#define PARSER_HPP

#include "token.hpp"
#include <cstring>
#include <filesystem>
#include <vector>

namespace forge::parser {

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

/*!
  @brief Parses a token stream in to a `Settings` struct
 */
Settings parse_token_stream(TokenStream tokens);
} // namespace forge::parser

#endif // PARSER_HPP
