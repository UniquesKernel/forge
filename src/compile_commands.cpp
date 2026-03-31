#include "compile_commands.hpp"
#include "toml.hpp"
#include <filesystem>
#include <fstream>
#include <vector>

std::vector<CommandJson> build_compile_commands_json(Settings settings) {
        std::string              src_path     = "src";
        std::vector<CommandJson> json_entries = std::vector<CommandJson>();

        std::string              directory    = std::filesystem::absolute(std::filesystem::current_path());
        for (const auto& entry : std::filesystem::directory_iterator(src_path)) {
                std::string filename   = entry.path().filename();
                std::string command    = settings.compiler + " " + entry.path().string() + " -g -Iinclude -c -o " +
                                         ".build/obj/" + filename.substr(0, filename.find_last_of('.')) + ".o";
                CommandJson json_entry = {.directory = directory, .command = command, .file = filename};
                json_entries.push_back(json_entry);
                printf("%s\n", json_entry.command.c_str());
                printf("%s\n", json_entry.file.c_str());

                printf("%s\n", json_entry.directory.c_str());
        }

        std::ofstream outfile("compile_commands.json");
        system("mkdir -p .build/obj/");

        if (outfile.is_open()) {
                outfile << "[\n";
                bool is_first = true;
                for (const auto& json_entry : json_entries) {
                        if (!is_first) {
                                outfile << "\t,{\n";
                        } else {
                                outfile << "\t{\n";
                                is_first = false;
                        }
                        outfile << "\t\t\"directory\":" << "\"" << json_entry.directory << "\",\n";
                        outfile << "\t\t\"command\":" << "\"" << json_entry.command << "\",\n";
                        outfile << "\t\t\"file\":" << "\"" << json_entry.file << "\"\n";
                        outfile << "\t}\n";
                        printf("%s\n", std::filesystem::current_path().c_str());
                        printf("Executing the build command: %s\n", json_entry.command.c_str());
                }
                outfile << "]\n";
                outfile.close();
        }


        return json_entries;
}