#include "compile.hpp"
#include "toml.hpp"
#include <filesystem>

void compile(std::vector<CommandJson> commandsJson) {
        for (const auto& entry : commandsJson) {
                system(entry.command.c_str());
        }
}

int link(Settings settings) {
        std::string build_cmd = "mkdir -p .build/bin";

        std::string cmd       = settings.compiler + " -g -O0 ";
        for (const auto& entry : std::filesystem::directory_iterator(".build/obj/")) {
                cmd += entry.path().string() + " ";
        }
        cmd += " -o .build/bin/" + settings.binary_name;

        printf("Executing: %s\n", build_cmd.c_str());
        int build_result = system(build_cmd.c_str());
        printf("Executing: %s\n", cmd.c_str());
        int result = system(cmd.c_str());

        if (build_result >= 0) {
                printf("Succesfully Created .build/bin\n");
        } else {
                printf("Failed to construct .build/bin\n");
                return build_result;
        }

        if (result >= 0) {
                printf("Succesfully build, %s\n", settings.binary_name.c_str());
        } else {
                printf("Failed to build: %s\n", settings.binary_name.c_str());
                return result;
        }

        return 0;
}