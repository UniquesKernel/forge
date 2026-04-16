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
        for (const forge::parser::Binary& entry : settings.binaries) {
                const std::filesystem::path bin_path = "bin/" + entry.bin_name + "/src";

                std::vector<CompileTask>    tasks    = build_compile_commands_json(&settings, bin_path);
                compile(tasks);

                // Argument 2: entry.bin_name (e.g., "forge") -> The Output Filename
                // Argument 3: bin_path       (e.g., "bin/...") -> The Input Object Dir
                link(settings, entry.bin_name, bin_path);
        }

        for (const forge::parser::Library& entry : settings.libraries) {
                const std::filesystem::path lib_path = "lib/" + entry.lib_name + "/src";

                std::vector<CompileTask>    tasks    = build_compile_commands_json(&settings, lib_path);
                compile(tasks);

                // Same here for libraries
                link(settings, entry.lib_name, lib_path);
        }
        return EXIT_SUCCESS;
}
