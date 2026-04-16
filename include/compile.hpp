#ifndef FORGE_COMPILE_HPP
#define FORGE_COMPILE_HPP

#include "compile_commands.hpp"
#include "parser.hpp"

void compile(std::vector<CompileTask> commandsJson);
void compile2(std::vector<CompileTask> commandsJson);
int  link(const forge::parser::Settings& settings, const std::string& target_name,
          const std::filesystem::path& target_src_path);
int  link2(forge::parser::Settings settings);

#endif
