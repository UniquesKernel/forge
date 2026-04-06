#ifndef FORGE_COMPILE_HPP
#define FORGE_COMPILE_HPP

#include "compile_commands.hpp"
#include "toml.hpp"
void compile(std::vector<CommandJson> commandsJson);
void compile2(std::vector<CommandJson> commandsJson);
int  link(Settings settings);
int  link2(Settings settings);

#endif
