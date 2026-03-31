#include "compile.hpp"
#include "compile_commands.hpp"
#include "token.hpp"
#include "toml.hpp"
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

void safe_free(void* ptr) {
        if ((void**)ptr == NULL || *((void**)(ptr)) == NULL) {
                return;
        }
        free(*((void**)ptr));
}

int main(void) {
        printf("This is running forge v0.1\n");

        __attribute__((cleanup(safe_free))) char* src          = read_project_file();
        TokenStream                               tokens       = forge::token::lexing(src);
        Settings                                  settings     = parse_project_file(tokens);
        std::vector<CommandJson>                  json_entries = build_compile_commands_json(settings);
        compile(json_entries);
        link(settings);

        free(tokens.stream);

        return EXIT_SUCCESS;
}
