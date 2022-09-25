#ifndef TOKEN_H
#define TOKEN_H

// This header file defines the tags used for tokens (i.e., terminal
// symbols in the grammar.)

enum TokenKind {
  TOK_IDENTIFIER,
  TOK_INTEGER_LITERAL,
  TOK_PLUS,
  TOK_MINUS,
  TOK_TIMES,
  TOK_DIVIDE,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_SEMICOLON,
  TOK_GREATER,
  TOK_LESS,
  TOK_GREATER_EQUAL,
  TOK_LESS_EQUAL,
  TOK_EQUAL,
  TOK_NOT_EQUAL,
  TOK_LOGICAL_AND,
  TOK_LOGICAL_OR,
  TOK_DEFINITION,
  TOK_ASSIGNMENT,
  TOK_IF,
  TOK_ELSE,
  TOK_LBRACK,
  TOK_RBRACK,
  TOK_WHILE
  // TODO: add members for additional kinds of tokens
};

#endif // TOKEN_H
