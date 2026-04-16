#include "assert.hpp"
#include "compile.hpp"
#include "compile_commands.hpp"
#include "parser.hpp"
#include "token.hpp"
#include "toml.hpp"
#include <cstdlib>
#include <sys/mman.h>

namespace {
void safe_token_stream_free(TokenStream* tokens) {
        REQUIRE(tokens != nullptr, );
        REQUIRE(tokens->stream != nullptr, );

        for (int i = 0; i < tokens->count; i++) {
                free(tokens->stream[i].lexeme);
        }
        free(tokens->stream);
}

void safe_toml_source_free(forge::toml::TomlSource* tokens) {
        REQUIRE(tokens != nullptr, );
        REQUIRE(tokens->source != nullptr, );

        munmap((void*)tokens->source, tokens->size);
}

} // namespace

int main(void) {
        printf("This is running forge v0.1\n");

        DEFER(safe_toml_source_free) forge::toml::TomlSource toml_source;
        if (forge::toml::map_config(&toml_source) != Error::OK) {
                return EXIT_FAILURE;
        }

        DEFER(safe_token_stream_free) TokenStream tstream;
        if (forge::token::lexing(toml_source, &tstream) != Error::OK) {
                return EXIT_FAILURE;
        }

        forge::parser::Settings settings = forge::parser::parse_token_stream(tstream);

        printf("%s", settings.binaries[0].bin_name.c_str());
        std::vector<CompileTask> all_tasks;

        // Collect Binary Tasks
        for (const auto& bin : settings.binaries) {
                const std::filesystem::path bin_path = "bin/" + bin.bin_name + "/src";
                std::vector<CompileTask>    tasks    = build_compile_commands_json(&settings, bin_path);
                all_tasks.insert(all_tasks.end(), tasks.begin(), tasks.end());
        }

        // Collect Library Tasks
        for (const auto& lib : settings.libraries) {
                const std::filesystem::path lib_path = "lib/" + lib.lib_name + "/src";
                std::vector<CompileTask>    tasks    = build_compile_commands_json(&settings, lib_path);
                all_tasks.insert(all_tasks.end(), tasks.begin(), tasks.end());
        }

        if (all_tasks.empty()) {
                printf("Nothing to compile.\n");
                return EXIT_SUCCESS;
        }

        // 3. Write Metadata (One file to rule them all)
        // This fixed the bug where the second target wiped the first target's data.
        write_compile_commands_json(all_tasks);

        // 4. Parallel Compilation (Saturation Phase)
        // This runs all tasks across your 20 available slots simultaneously.
        compile(all_tasks);

        // 5. Link Phase (Output Generation)
        // We link separately because each produces a unique artifact (.a, .so, or binary).
        printf("\nEntering Link Phase...\n");

        for (const auto& bin : settings.binaries) {
                const std::filesystem::path bin_path = "bin/" + bin.bin_name + "/src";
                if (link(settings, bin.bin_name, bin_path) != 0)
                        return EXIT_FAILURE;
        }

        for (const auto& lib : settings.libraries) {
                const std::filesystem::path lib_path = "lib/" + lib.lib_name + "/src";
                // Note: Your link() function will eventually need to check lib.static/lib.shared
                if (link(settings, lib.lib_name, lib_path) != 0)
                        return EXIT_FAILURE;
        }

        printf("\n[SUCCESS] Project built.\n");
        return EXIT_SUCCESS;
}
