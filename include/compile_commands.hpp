#ifndef FORGE_COMPILE_COMMANDS_HPP
#define FORGE_COMPILE_COMMANDS_HPP

#include "toml.hpp"
#include <filesystem>
#include <string>
#include <vector>

struct CommandJson {
        std::string directory;
        std::string command;
        std::string file;
};

std::vector<CommandJson> build_compile_commands_json(Settings* settings, const std::filesystem::path path);

#endif
