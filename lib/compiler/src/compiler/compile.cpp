#include "compile.hpp"
#include "toml.hpp"
#include <filesystem>
#include <queue>
#include <vector>

void compile2(std::vector<CommandJson> commandsJson) {
        for (const auto& entry : commandsJson) {
                system(entry.command.c_str());
        }
}

int link2(Settings settings) {
        std::string                       build_cmd  = "mkdir -p .build/bin";
        std::queue<std::filesystem::path> to_explore = std::queue<std::filesystem::path>();
        to_explore.push(".build/obj/");
        std::string cmd = settings.config.compiler + settings.config.compiler_standard + " -g -O0 ";
        while (!to_explore.empty()) {
                for (const auto& entry : std::filesystem::directory_iterator(to_explore.front())) {
                        if (entry.is_directory()) {
                                to_explore.push(entry.path());
                                continue;
                        }
                        cmd += entry.path().string() + " ";
                }
                to_explore.pop();
        }
        cmd += " -o .build/bin/" + settings.config.binary;

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
                printf("Succesfully build, %s\n", settings.config.binary.c_str());
        } else {
                printf("Failed to build: %s\n", settings.config.binary.c_str());
                return result;
        }

        return 0;
}
