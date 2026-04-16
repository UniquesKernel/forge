#include "compile.hpp"
#include "constants.hpp"
#include "parser.hpp"
#include <filesystem>
#include <vector>

void compile(std::vector<CompileTask> commandsJson) {
        for (const auto& entry : commandsJson) {
                system(entry.cmd_str);
        }
}

int link(const forge::parser::Settings& settings, const std::string& target_name,
         const std::filesystem::path& target_src_path) {
        using namespace std::filesystem;

        if (!exists(BINRARY_FOLDER)) {
                create_directories(BINRARY_FOLDER);
        }

        // 1. Where the objects are (mirrored convention)
        path        specific_obj_dir = path(OBJECT_FOLDER) / target_src_path;

        // 2. Where the binary goes (flat output)
        // Use target_name ("forge") so it sits directly in .build/bin/
        path        output_file      = path(BINRARY_FOLDER) / target_name;

        std::string cmd              = settings.config.compiler + " " + settings.config.compiler_standard + " -g -O0 ";

        // Collect objects from the mirrored directory
        if (exists(specific_obj_dir)) {
                for (const directory_entry& entry : recursive_directory_iterator(specific_obj_dir)) {
                        if (!entry.is_directory() && entry.path().extension() == OBJECT_FILE_EXTENSION) {
                                cmd += entry.path().string() + " ";
                        }
                }
        }

        // Use the flat path for the -o flag
        cmd += " -o " + output_file.string();

        printf("Executing Linker: %s\n", cmd.c_str());
        int result = system(cmd.c_str());

        if (result == 0) {
                printf("Successfully built binary: %s\n", output_file.c_str());
                return 0;
        }
        return 1;
}
