#ifndef FORGE_COMPILE_COMMANDS_HPP
#define FORGE_COMPILE_COMMANDS_HPP

#include "parser.hpp"
#include <filesystem>

struct CompileTask {
        char**      argv;
        char*       cmd_str;
        const char* file;
};

void                     write_compile_commands_json(const std::vector<CompileTask>& tasks);
std::vector<CompileTask> build_compile_commands_json(forge::parser::Settings*    settings,
                                                     const std::filesystem::path path);

#endif
