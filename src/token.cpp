#include "token.hpp"
#include "types.hpp"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>

static void skip_whitespace(char** source);
static void process_key(Token* tok, char** source);
static void process_value(Token* tok, char** source);
static void process_digit(Token* tok, char** source);
static void process_label(Token* tok, char** source);

static const forge::u8 MAX_LEXEME_LENGTH = 100;

namespace forge::token {

[[nodiscard("Tokens from the forge.toml file should not be ignored")]]
Token tokenize(char** source) {
        Token tok = {};
        tok.lexeme = (char*)malloc(sizeof(char) * MAX_LEXEME_LENGTH);

        char* src = *source;

        if (*src == '\0') {
                tok.lexeme[0] = 'E';
                tok.type = TokenType::TOK_EOF;
                goto return_label;
        }

        skip_whitespace(&src);

        if (isalnum(*src) || *src == '_' || *src == '-') {
                process_key(&tok, &src);
                goto return_label;
        }

        if (*src == '[') {
                process_label(&tok, &src);
                goto return_label;
        }

        if (*src == '=') {
                tok.type      = TokenType::TOK_EQUALS;
                tok.lexeme[0] = *(src++);
                tok.lexeme[1] = '\0';
                goto return_label;
        }

        if (*src == '"') {
                process_value(&tok, &src);
                goto return_label;
        }

        if (isdigit(*src)) {
                process_digit(&tok, &src);
                goto return_label;
        }

        tok.lexeme[0] = 'U';
        tok.type = TokenType::TOK_UNKNOWN;
        src++;

    return_label:
        *source = src;
        return tok;
}

[[nodiscard("A Token Stream should not be ignored, "
            "as this will lead to a memory leak" )]]
TokenStream lexing(char* src) {
        Token* token_steam = (Token*)malloc(sizeof(Token) * 100);

        int    token_count = 0;
        Token  tok         = tokenize(&src);
        while (tok.type != TokenType::TOK_EOF && token_count < 100) {
                printf("%s\n", tok.lexeme);
                token_steam[token_count++] = tok;
                tok                        = tokenize(&src);
        }
        token_steam[token_count++] = tok;

        return {.stream = token_steam, .count = token_count};
}
} // namespace forge::token


static void skip_whitespace(char** source) {
        char* src = *source;

        while ((*src) == ' ' || (*src) == '\t' || (*src) == '\n' || (*src) == '\r') {
                src++;
        }

        *source = src;
}

static void process_key(Token* tok, char** source) {
        char* src = *source;

        int   i   = 0;
        while (isalnum(*src) || (*src) == '_' || (*src) == '-') {
                tok->lexeme[i++] = *(src++);
        }
        tok->lexeme[i++] = '\0';
        tok->type = TokenType::TOK_KEY;

        *source  = src;
}

static void process_value(Token* tok, char** source) {
        char* src = *source;

        // NOTE: Skip starting '\"' character
        src++;

        int       i         = 0;
        const int token_max = 50;
        while (*src && *src != '"' && i < token_max) {
                tok->lexeme[i++] = *(src++);
        }
        tok->lexeme[i++] = '\0';
        tok->type = TokenType::TOK_VALUE;

        // NOTE: Skip the closing '\"' character
        src++;

        *source = src;
}

static void process_digit(Token* tok, char** source) {
        char* src = *source;
        int   i   = 0;
        
        while (isdigit(*src)) {
                tok->lexeme[i++] = *(src++);
        }
        tok->lexeme[i++] = '\0';
        tok->type = TokenType::TOK_INTEGER;
        
        *source  = src;
}

static void process_label(Token* tok, char** source) {
        char* src = *source;

        int i = 0;
        src++; // skip first '[' character
        src++; // skip second '[' character
        while(*src != ']') {
                tok->lexeme[i++] = *(src++);
        }
        tok->lexeme[i++] = '\0';
        tok->type = TokenType::TOK_LABEL;
        src++; // skip first ']' character in label
        src++; // skip second ']' character in label

        *source = src;
}