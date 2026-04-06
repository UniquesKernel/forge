#ifndef FORGE_CONSTANTS_HPP
#define FORGE_CONSTANTS_HPP

#include <filesystem>
#include <string>

static const std::string DIRECTORY             = std::filesystem::absolute(std::filesystem::current_path());
static const std::string OBJECT_FOLDER         = ".build/obj/";
static const std::string BINRARY_FOLDER        = ".build/bin/";
static const std::string COMPILE_COMMANDS_FILE = "compile_commands.json";
static const std::string SOURCE_FILE_EXTENSION = ".cpp";
static const std::string OBJECT_FILE_EXTENSION = ".o";

#endif // FORGE_CONSTANTS_HPP
