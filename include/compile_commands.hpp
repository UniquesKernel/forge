#ifndef FORGE_COMPILE_COMMANDS_HPP
#define FORGE_COMPILE_COMMANDS_HPP

#include <string>
#include <vector>
#include "toml.hpp"

struct CommandJson {
        std::string directory;
        std::string command;
        std::string file;
};

std::vector<CommandJson> build_compile_commands_json(Settings settings);

#endif