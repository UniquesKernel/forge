#ifndef FORGE_TOKEN_HPP
#define FORGE_TOKEN_HPP

enum class TokenType : int {
  TOK_KEY = 1,
  TOK_EQUALS = 2,
  TOK_STRING = 3,
  TOK_INTEGER = 4,
  TOK_EOF = 5,
  TOK_UNKNOWN = 6
};

struct Token {
  TokenType type;
  char lexeme[64];
};

struct TokenStream {
    Token* token_steam;
    int token_count;
};

void skip_whitespace(char** src);
Token tokenize(char** src);
TokenStream lexing(char* src);

#endif