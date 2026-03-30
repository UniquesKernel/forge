#include "token.hpp"
#include <cctype>
#include <cstdlib>

void skip_whitespace(char **source) {
  char* src = *source;
  
  while ((*src) == ' ' || (*src) == '\t') {
    src++;
  }

  *source = src;
}

Token tokenize(char **source) {
  Token tok = {};

  char *src = *source;

  if (*src == '\0') {
    tok.type = TokenType::TOK_EOF;
    *source = src;
    return tok;
  }

  skip_whitespace(&src);

  if (isalnum(*src) || *src == '_' || *src == '-') {
    int i = 0;
    while (isalnum(*src) || *src == '_' || *src == '-') {
      tok.lexeme[i++] = *src++;
    }
    tok.type = TokenType::TOK_KEY;
    *source = src;
    return tok;
  }

  if (*src == '=') {
    tok.type = TokenType::TOK_EQUALS;
    tok.lexeme[0] = *src++;
    *source = src;
    return tok;
  }

  if (*src == '"') {
    src++; // skip "
    int i = 0;
    while (*src && *src != '"') {
      tok.lexeme[i++] = *src++;
    }
    src++; 
    tok.type = TokenType::TOK_STRING;
    *source = src;
    return tok;
  }

  if (isdigit(*src)) {
    int i = 0;
    while (isdigit(*src)) {
      tok.lexeme[i++] = *src++;
    }
    tok.type = TokenType::TOK_INTEGER;
    *source = src;
    return tok;
  }

  tok.type = TokenType::TOK_UNKNOWN;
  src++;
  *source = src;
  return tok;
}

TokenStream lexing(char *src) {
  Token *token_steam = (Token *)malloc(sizeof(Token) * 100);

  int token_count = 0;
  Token tok = tokenize(&src);
  while (tok.type != TokenType::TOK_EOF && token_count < 100) {
    token_steam[token_count++] = tok;
    tok = tokenize(&src);
  }

  return {.token_steam = token_steam, .token_count = token_count };
}
