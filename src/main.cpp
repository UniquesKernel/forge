#include "assert.hpp"
#include "compile.hpp"
#include "compile_commands.hpp"
#include "token.hpp"
#include "toml.hpp"
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>

namespace {
void safe_free(void* ptr) {
        void* internal_ptr = *(void**)ptr;
        REQUIRE(internal_ptr != nullptr,);

        free(internal_ptr);
}

void safe_token_stream_free(TokenStream* tokens) {
        REQUIRE(tokens != nullptr,);
        REQUIRE(tokens->stream != nullptr,);

        for (int i = 0; i < tokens->count; i++) {
                free(tokens->stream[i].lexeme);
        }
        free(tokens->stream);
}

} // namespace

int main(void) {
        printf("This is running forge v0.1\n");

        DEFER(safe_free) char* source = read_project_toml();
        DEFER(safe_token_stream_free) TokenStream tokens_stream = forge::token::lexing(source);
        

        DEFER(safe_free) char*                    src          = read_project_toml();
        DEFER(safe_token_stream_free) TokenStream tokens       = forge::token::lexing(src);
        Settings                                  settings     = parse_project_file(tokens);
        std::vector<CommandJson>                  json_entries = build_compile_commands_json(settings);
        compile(json_entries);
        link(settings);
        return EXIT_SUCCESS;
}
