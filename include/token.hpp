#ifndef FORGE_TOKEN_HPP
#define FORGE_TOKEN_HPP

enum class TokenType : int {
        TOK_KEY     = 1,
        TOK_EQUALS  = 2,
        TOK_VALUE   = 3,
        TOK_INTEGER = 4,
        TOK_LABEL   = 5,
        TOK_EOF     = 6,
        TOK_UNKNOWN = 7
};

struct Token {
        TokenType type;
        char*     lexeme;
};

typedef struct TokenStream {
        Token* stream;
        int    count;
}TokenStream;

namespace forge::token {
Token       tokenize(char** src);
TokenStream lexing(char* src);
} // namespace forge::token

#endif